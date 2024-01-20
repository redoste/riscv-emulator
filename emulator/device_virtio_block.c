#include <stdbool.h>
#include <stdio.h>

#include "device_virtio.h"
#include "device_virtio_block.h"
#include "emulator.h"
#include "isa.h"

#define VIRTIO_BLK_DEVICE_ID 2

#define VIRTIO_BLK_CFG_CAPACITY_LOW_BASE  0
#define VIRTIO_BLK_CFG_CAPACITY_HIGH_BASE 4

#define VIRTIO_BLK_REQUESTQ 0
#define VIRTIO_BLK_N_QUEUES 1

#define VIRTIO_BLK_QUEUE_NUM_MAX 1024

#define VIRTIO_BLK_T_IN           0
#define VIRTIO_BLK_T_OUT          1
#define VIRTIO_BLK_T_FLUSH        4
#define VIRTIO_BLK_T_DISCARD      11
#define VIRTIO_BLK_T_WRITE_ZEROES 13

#define VIRTIO_BLK_S_OK     0
#define VIRTIO_BLK_S_IOERR  1
#define VIRTIO_BLK_S_UNSUPP 2

#define VIRTIO_BLK_SECTOR_SIZE 512

typedef struct virtio_block_t {
	size_t capacity;
	FILE* image;
} virtio_block_t;

static void virtio_block_free(emulator_t* emu, virtio_t* virtio) {
	(void)emu;

	virtio_block_t* virtio_block = (virtio_block_t*)virtio->device_data;
	fclose(virtio_block->image);
	free(virtio_block);
}

static bool virtio_block_req(emulator_t* emu, virtio_t* virtio, size_t queue_index, virtio_desc_t* desc) {
	assert(queue_index == VIRTIO_BLK_REQUESTQ);
	virtio_block_t* virtio_block = (virtio_block_t*)virtio->device_data;

	if (desc->bufs_len < 3 ||
	    desc->bufs[0].len < 0x10 ||
	    desc->bufs[1].len == 0 || (desc->bufs[1].len % VIRTIO_BLK_SECTOR_SIZE) != 0 ||
	    !(desc->bufs[2].flags & VIRTQ_DESC_F_WRITE) || desc->bufs[2].len != 1) {
		return false;
	}

	/* struct virtio_blk_req {
	 *   le32 type;
	 *   le32 reserved;
	 *   le64 sector;
	 *   u8 data[];
	 *   u8 status;
	 * };
	 */

	uint32_t type;
	uint64_t sector;
	if (!emu_physical_r32(emu, desc->bufs[0].addr + 0, &type) ||
	    !emu_physical_r64(emu, desc->bufs[0].addr + 8, &sector)) {
		return false;
	}

	bool ret = true;
	size_t data_len = desc->bufs[1].len;
	if (type == VIRTIO_BLK_T_IN && (desc->bufs[1].flags & VIRTQ_DESC_F_WRITE)) {
		ret &= fseek(virtio_block->image, sector * VIRTIO_BLK_SECTOR_SIZE, SEEK_SET) == 0;
		uint8_t* data_buf = malloc(data_len);
		assert(data_buf != NULL);
		size_t read_buf = fread(data_buf, 1, data_len, virtio_block->image);
		ret &= read_buf == data_len;
		for (size_t i = 0; i < read_buf; i++) {
			ret &= emu_physical_w8(emu, desc->bufs[1].addr + i, data_buf[i]);
		}
		free(data_buf);

		desc->written_len = read_buf + 1;
	} else if (type == VIRTIO_BLK_T_OUT) {
		ret &= fseek(virtio_block->image, sector * VIRTIO_BLK_SECTOR_SIZE, SEEK_SET) == 0;
		uint8_t* data_buf = malloc(data_len);
		assert(data_buf != NULL);
		for (size_t i = 0; i < data_len; i++) {
			ret &= emu_physical_r8(emu, desc->bufs[1].addr + i, &data_buf[i]);
		}
		ret &= fwrite(data_buf, 1, data_len, virtio_block->image) == data_len;
		free(data_buf);

		desc->written_len = 1;
	} else {
		return emu_physical_w8(emu, desc->bufs[2].addr, VIRTIO_BLK_S_UNSUPP);
	}

	return emu_physical_w8(emu, desc->bufs[2].addr, ret ? VIRTIO_BLK_S_OK : VIRTIO_BLK_S_IOERR);
}

static uint32_t virtio_block_r32_config(emulator_t* emu, virtio_t* virtio, guest_paddr addr) {
	virtio_block_t* virtio_block = (virtio_block_t*)virtio->device_data;
	if (addr == VIRTIO_BLK_CFG_CAPACITY_LOW_BASE) {
		return virtio_block->capacity & 0xffffffff;
	} else if (addr == VIRTIO_BLK_CFG_CAPACITY_HIGH_BASE) {
		return virtio_block->capacity >> 32;
	} else {
		cpu_throw_exception(emu, EXC_LOAD_ACCESS_FAULT, virtio->base + VIRTIO_CONFIG_BASE + addr);
		return 0;
	}
}

static void virtio_block_w32_config(emulator_t* emu, virtio_t* virtio, guest_paddr addr, uint32_t value) {
	(void)value;

	cpu_throw_exception(emu, EXC_STORE_ACCESS_FAULT, virtio->base + VIRTIO_CONFIG_BASE + addr);
}

static const virtio_handlers_t virtio_block_handlers = {
	.free_handler = virtio_block_free,
	.req_handler = virtio_block_req,
	.config_r32_handler = virtio_block_r32_config,
	.config_w32_handler = virtio_block_w32_config,
};

bool virtio_block_create(emulator_t* emu, guest_paddr base, size_t int_number, FILE* image) {
	if (fseek(image, 0, SEEK_END) != 0) {
		fclose(image);
		return false;
	}
	size_t capacity = ftell(image) / VIRTIO_BLK_SECTOR_SIZE;

	virtio_block_t* virtio_block = malloc(sizeof(virtio_block_t));
	assert(virtio_block != NULL);

	virtio_block->image = image;
	virtio_block->capacity = capacity;

	return virtio_create(emu, base, int_number, VIRTIO_BLK_DEVICE_ID, VIRTIO_BLK_N_QUEUES,
			     VIRTIO_BLK_QUEUE_NUM_MAX, &virtio_block_handlers, virtio_block);
}
