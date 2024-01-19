#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "device_plic.h"
#include "device_virtio.h"
#include "devices.h"
#include "isa.h"

static void virtio_free(emulator_t* emu, void* device_data) {
	(void)emu;

	virtio_t* virtio = (virtio_t*)device_data;
	virtio->handlers->free_handler(emu, virtio);
	free(virtio->queues);
	free(virtio);
}

static bool virtio_add_used_desc(emulator_t* emu, virtio_queue_t* queue, size_t desc_index, size_t desc_len) {
	/* struct virtq_used {
	 *   le16 flags;
	 *   le16 idx;
	 *   struct virtq_used_elem ring[queue_num];
	 * };
	 * struct virtq_used_elem {
	 *   le32 id;
	 *   le32 len;
	 * };
	 */

	if (!emu_physical_w32(emu, queue->queue_device + 4 + ((queue->queue_device_index % queue->queue_num) * 8), desc_index) ||
	    !emu_physical_w32(emu, queue->queue_device + 4 + ((queue->queue_device_index % queue->queue_num) * 8) + 4, desc_len) ||
	    !emu_physical_w16(emu, queue->queue_device + 2, queue->queue_device_index + 1)) {
		return false;
	}
	queue->queue_device_index++;
	return true;
}

static bool virtio_read_desc(emulator_t* emu, virtio_queue_t* queue, size_t desc_index, virtio_desc_t* desc) {
	desc->desc_index = desc_index;

	for (size_t i = 0; i < sizeof(desc->bufs) / sizeof(desc->bufs[0]); i++) {
		if (desc_index >= queue->queue_num) {
			return false;
		}
		uint64_t addr;
		uint32_t len;
		uint16_t flags, next;

		/* struct virtq_desc {
		 *   le64 addr;
		 *   le32 len;
		 *   le16 flags;
		 *   le16 next;
		 * }
		 */
		if (!emu_physical_r64(emu, queue->queue_desc + (desc_index * 16) + 0, &addr) ||
		    !emu_physical_r32(emu, queue->queue_desc + (desc_index * 16) + 8, &len) ||
		    !emu_physical_r16(emu, queue->queue_desc + (desc_index * 16) + 12, &flags) ||
		    !emu_physical_r16(emu, queue->queue_desc + (desc_index * 16) + 14, &next)) {
			return false;
		}

		desc->bufs[i].addr = addr;
		desc->bufs[i].len = len;
		desc->bufs[i].flags = flags;

		if (!(flags & VIRTQ_DESC_F_NEXT)) {
			desc->bufs_len = i + 1;
			return true;
		} else {
			desc_index = next;
		}
	}
	return false;
}

static void virtio_process_queue(emulator_t* emu, virtio_t* virtio, size_t queue_index) {
	assert(queue_index < virtio->queue_size);
	virtio_queue_t* queue = &virtio->queues[queue_index];

	uint16_t flags, idx;

	/* struct virtq_avail {
	 *   le16 flags;
	 *   le16 idx;
	 *   le16 ring[queue_size];
	 * };
	 */
	if (!emu_physical_r16(emu, queue->queue_driver + 0, &flags) ||
	    !emu_physical_r16(emu, queue->queue_driver + 2, &idx)) {
		return;
	}

	bool used_desc = false;
	for (uint16_t i = queue->queue_driver_index; i != idx; i = i + 1) {
		uint16_t ring_idx;
		virtio_desc_t desc = {0};
		if (!emu_physical_r16(emu, queue->queue_driver + 4 + (2 * (i % queue->queue_num)), &ring_idx) ||
		    !virtio_read_desc(emu, queue, ring_idx, &desc)) {
			return;
		}

		if (virtio->handlers->req_handler(emu, virtio, queue_index, &desc)) {
			queue->queue_driver_index = i + 1;
			used_desc |= true;
			if (!virtio_add_used_desc(emu, queue, desc.desc_index, desc.written_len)) {
				return;
			}
		} else {
			break;
		}
	}

	if (used_desc && !(flags & VIRTQ_AVAIL_F_NO_INTERRUPT) && virtio->interrupt_status == 0 &&
	    virtio->int_number != 0 && emu->plic != NULL) {
		virtio->interrupt_status = (1 << 0);  // Used buffer notification
		plic_throw_interrupt(emu, virtio->int_number);
	}
}

bool virtio_read_and_use_desc(emulator_t* emu, virtio_t* virtio, size_t queue_index, virtio_desc_t* desc, size_t desc_written_len) {
	assert(queue_index < virtio->queue_size);
	virtio_queue_t* queue = &virtio->queues[queue_index];

	uint16_t flags, idx;

	/* struct virtq_avail {
	 *   le16 flags;
	 *   le16 idx;
	 *   le16 ring[queue_size];
	 * };
	 */
	if (!emu_physical_r16(emu, queue->queue_driver + 0, &flags) ||
	    !emu_physical_r16(emu, queue->queue_driver + 2, &idx)) {
		return false;
	}

	if (queue->queue_driver_index == idx) {
		// No new desc available
		return false;
	}

	uint16_t ring_idx;
	if (!emu_physical_r16(emu, queue->queue_driver + 4 + (2 * (queue->queue_driver_index % queue->queue_num)), &ring_idx) ||
	    !virtio_read_desc(emu, queue, ring_idx, desc) ||
	    !virtio_add_used_desc(emu, queue, desc->desc_index, desc_written_len)) {
		return false;
	}
	queue->queue_driver_index += 1;

	if (!(flags & VIRTQ_AVAIL_F_NO_INTERRUPT) && virtio->interrupt_status == 0 &&
	    virtio->int_number != 0 && emu->plic != NULL) {
		virtio->interrupt_status = (1 << 0);  // Used buffer notification
		plic_throw_interrupt(emu, virtio->int_number);
	}
	return true;
}

static uint8_t virtio_r8(emulator_t* emu, void* device_data, guest_paddr addr) {
	virtio_t* virtio = (virtio_t*)device_data;
	if (addr >= VIRTIO_CONFIG_BASE && virtio->handlers->config_r8_handler != NULL) {
		return virtio->handlers->config_r8_handler(emu, virtio, addr - VIRTIO_CONFIG_BASE);
	} else {
		cpu_throw_exception(emu, EXC_LOAD_ACCESS_FAULT, virtio->base + addr);
		return 0;
	}
}

static uint16_t virtio_r16(emulator_t* emu, void* device_data, guest_paddr addr) {
	virtio_t* virtio = (virtio_t*)device_data;
	cpu_throw_exception(emu, EXC_LOAD_ACCESS_FAULT, virtio->base + addr);
	return 0;
}

static uint32_t virtio_r32(emulator_t* emu, void* device_data, guest_paddr addr) {
	virtio_t* virtio = (virtio_t*)device_data;
	switch (addr) {
		case VIRTIO_MAGIC_BASE:
			return VIRTIO_MAGIC;
		case VIRTIO_VERSION_BASE:
			return VIRTIO_VERSION;
		case VIRTIO_DEVICEID_BASE:
			return virtio->device_id;
		case VIRTIO_VENDORID_BASE:
			return 0;
		case VIRTIO_DEVICEFEATURES_BASE:
			return virtio->device_features_sel == 0   ? virtio->device_features & 0xffffffff
			       : virtio->device_features_sel == 1 ? virtio->device_features >> 32
								  : 0;
		case VIRTIO_QUEUENUMMAX_BASE:
			return virtio->queues[virtio->queue_sel].queue_num_max;
		case VIRTIO_QUEUEREADY_BASE:
			return virtio->queues[virtio->queue_sel].queue_ready;
		case VIRTIO_INTERRUPTSTATUS_BASE:
			return virtio->interrupt_status;
		case VIRTIO_STATUS_BASE:
			return virtio->status;
		case VIRTIO_CONFIGGENERATION_BASE:
			return virtio->config_generation;
		default:
			if (addr >= VIRTIO_CONFIG_BASE && virtio->handlers->config_r32_handler != NULL) {
				return virtio->handlers->config_r32_handler(emu, virtio, addr - VIRTIO_CONFIG_BASE);
			} else {
				cpu_throw_exception(emu, EXC_LOAD_ACCESS_FAULT, virtio->base + addr);
				return 0;
			}
	}
}

static uint64_t virtio_r64(emulator_t* emu, void* device_data, guest_paddr addr) {
	virtio_t* virtio = (virtio_t*)device_data;
	cpu_throw_exception(emu, EXC_LOAD_ACCESS_FAULT, virtio->base + addr);
	return 0;
}

static void virtio_w8(emulator_t* emu, void* device_data, guest_paddr addr, uint8_t value) {
	(void)value;

	virtio_t* virtio = (virtio_t*)device_data;
	if (addr >= VIRTIO_CONFIG_BASE && virtio->handlers->config_w8_handler != NULL) {
		virtio->handlers->config_w8_handler(emu, virtio, addr - VIRTIO_CONFIG_BASE, value);
	} else {
		cpu_throw_exception(emu, EXC_STORE_ACCESS_FAULT, virtio->base + addr);
	}
}

static void virtio_w16(emulator_t* emu, void* device_data, guest_paddr addr, uint16_t value) {
	(void)value;

	virtio_t* virtio = (virtio_t*)device_data;
	cpu_throw_exception(emu, EXC_STORE_ACCESS_FAULT, virtio->base + addr);
}

static void virtio_w32(emulator_t* emu, void* device_data, guest_paddr addr, uint32_t value) {
	virtio_t* virtio = (virtio_t*)device_data;
	switch (addr) {
		case VIRTIO_DEVICEFEATURESSEL_BASE:
			virtio->device_features_sel = value;
			break;
		case VIRTIO_DRIVERFEATURES_BASE:
			virtio->driver_features = (virtio->driver_features & ~(0xffffffff << (virtio->driver_features_sel * 32))) |
						  (value << (virtio->driver_features_sel * 32));
			break;
		case VIRTIO_DRIVERFEATURESSEL_BASE:
			virtio->driver_features_sel = value;
			break;
		case VIRTIO_QUEUESEL_BASE:
			virtio->queue_sel = value < virtio->queue_size ? value : 0;
			break;
		case VIRTIO_QUEUENUM_BASE:
			if (value <= virtio->queues[virtio->queue_sel].queue_num_max) {
				virtio->queues[virtio->queue_sel].queue_num = value;
			}
			break;
		case VIRTIO_QUEUEREADY_BASE:
			virtio->queues[virtio->queue_sel].queue_ready = value;
			break;
		case VIRTIO_QUEUENOTIFY_BASE:
			if (value < virtio->queue_size) {
				virtio_process_queue(emu, virtio, value);
			}
			break;
		case VIRTIO_INTERRUPTACK_BASE:
			virtio->interrupt_status = 0;
			break;
		case VIRTIO_STATUS_BASE:
			virtio->status = value;
			break;
		case VIRTIO_QUEUEDESCLOW_BASE:
			virtio->queues[virtio->queue_sel].queue_desc = (virtio->queues[virtio->queue_sel].queue_desc & (0xffffffffll << 32)) |
								       value;
			break;
		case VIRTIO_QUEUEDESCHIGH_BASE:
			virtio->queues[virtio->queue_sel].queue_desc = (virtio->queues[virtio->queue_sel].queue_desc & 0xffffffffll) |
								       ((uint64_t)value << 32);
			break;
		case VIRTIO_QUEUEDRIVERLOW_BASE:
			virtio->queues[virtio->queue_sel].queue_driver = (virtio->queues[virtio->queue_sel].queue_driver & (0xffffffffll << 32)) |
									 value;
			break;
		case VIRTIO_QUEUEDRIVERHIGH_BASE:
			virtio->queues[virtio->queue_sel].queue_driver = (virtio->queues[virtio->queue_sel].queue_driver & 0xffffffffll) |
									 ((uint64_t)value << 32);
			break;
		case VIRTIO_QUEUEDEVICELOW_BASE:
			virtio->queues[virtio->queue_sel].queue_device = (virtio->queues[virtio->queue_sel].queue_device & (0xffffffffll << 32)) |
									 value;
			break;
		case VIRTIO_QUEUEDEVICEHIGH_BASE:
			virtio->queues[virtio->queue_sel].queue_device = (virtio->queues[virtio->queue_sel].queue_device & 0xffffffffll) |
									 ((uint64_t)value << 32);
			break;
		default:
			if (addr >= VIRTIO_CONFIG_BASE && virtio->handlers->config_w32_handler != NULL) {
				virtio->handlers->config_w32_handler(emu, virtio, addr - VIRTIO_CONFIG_BASE, value);
			} else {
				cpu_throw_exception(emu, EXC_STORE_ACCESS_FAULT, virtio->base + addr);
			}
			break;
	}
}

static void virtio_w64(emulator_t* emu, void* device_data, guest_paddr addr, uint64_t value) {
	(void)value;

	virtio_t* virtio = (virtio_t*)device_data;
	cpu_throw_exception(emu, EXC_STORE_ACCESS_FAULT, virtio->base + addr);
}

static void virtio_update(emulator_t* emu, void* device_data) {
	virtio_t* virtio = (virtio_t*)device_data;
	if (virtio->handlers->update_handler != NULL) {
		virtio->handlers->update_handler(emu, virtio);
	}

	if (virtio->interrupt_status != 0 &&
	    virtio->int_number != 0 && emu->plic != NULL) {
		plic_throw_interrupt(emu, virtio->int_number);
	}
}

bool virtio_create(emulator_t* emu, guest_paddr base, size_t int_number, uint32_t device_id, size_t queue_size, uint32_t queue_num_max, const virtio_handlers_t* handlers, void* device_data) {
	virtio_t* virtio = malloc(sizeof(virtio_t));
	assert(virtio != NULL);
	memset(virtio, 0, sizeof(virtio_t));
	virtio->base = base;
	virtio->int_number = int_number;
	virtio->device_id = device_id;
	virtio->device_features = (1ll << 32);  // VIRTIO_F_VERSION_1
	virtio->handlers = handlers;
	virtio->device_data = device_data;

	assert(queue_size > 0);
	virtio->queue_size = queue_size;
	virtio->queues = malloc(sizeof(virtio_queue_t) * queue_size);
	assert(virtio->queues != NULL);
	memset(virtio->queues, 0, sizeof(virtio_queue_t) * queue_size);
	for (size_t i = 0; i < queue_size; i++) {
		virtio->queues[i].queue_num_max = queue_num_max;
	}

	device_mmio_t device = {
		virtio,
		virtio_free,
		virtio_update,
		virtio_r8,
		virtio_r16,
		virtio_r32,
		virtio_r64,
		virtio_w8,
		virtio_w16,
		virtio_w32,
		virtio_w64,
	};

	if (!emu_add_mmio_device(emu, base, 0x1000, &device)) {
		virtio->handlers->free_handler(emu, virtio);
		free(virtio->queues);
		free(virtio);
		return false;
	}
	return true;
}
