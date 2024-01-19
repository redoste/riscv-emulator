#ifndef DEVICE_VIRTIO_H
#define DEVICE_VIRTIO_H

#include <stdbool.h>

#include "emulator.h"
#include "isa.h"

/* This is a base implementation for virtio devices
 * <https://docs.oasis-open.org/virtio/virtio/v1.1/cs01/virtio-v1.1-cs01.html>
 * <https://wiki.osdev.org/Virtio>
 */

/* VIRTIO_x_BASE : virtio MMIO registers base addresses
 */
#define VIRTIO_MAGIC_BASE             0x000
#define VIRTIO_VERSION_BASE           0x004
#define VIRTIO_DEVICEID_BASE          0x008
#define VIRTIO_VENDORID_BASE          0x00c
#define VIRTIO_DEVICEFEATURES_BASE    0x010
#define VIRTIO_DEVICEFEATURESSEL_BASE 0x014
#define VIRTIO_DRIVERFEATURES_BASE    0x020
#define VIRTIO_DRIVERFEATURESSEL_BASE 0x024
#define VIRTIO_QUEUESEL_BASE          0x030
#define VIRTIO_QUEUENUMMAX_BASE       0x034
#define VIRTIO_QUEUENUM_BASE          0x038
#define VIRTIO_QUEUEREADY_BASE        0x044
#define VIRTIO_QUEUENOTIFY_BASE       0x050
#define VIRTIO_INTERRUPTSTATUS_BASE   0x060
#define VIRTIO_INTERRUPTACK_BASE      0x064
#define VIRTIO_STATUS_BASE            0x070
#define VIRTIO_QUEUEDESCLOW_BASE      0x080
#define VIRTIO_QUEUEDESCHIGH_BASE     0x084
#define VIRTIO_QUEUEDRIVERLOW_BASE    0x090
#define VIRTIO_QUEUEDRIVERHIGH_BASE   0x094
#define VIRTIO_QUEUEDEVICELOW_BASE    0x0a0
#define VIRTIO_QUEUEDEVICEHIGH_BASE   0x0a4
#define VIRTIO_CONFIGGENERATION_BASE  0x0fc
#define VIRTIO_CONFIG_BASE            0x100

/* VIRTIO_MAGIC : virtio MMIO magic
 */
#define VIRTIO_MAGIC 0x74726976

/* VIRTIO_VERSION : virtio MMIO version
 */
#define VIRTIO_VERSION 2

/* VIRTQ_DESC_F_x : virtio queue descriptor flags
 */
#define VIRTQ_DESC_F_NEXT     1
#define VIRTQ_DESC_F_WRITE    2
#define VIRTQ_DESC_F_INDIRECT 4

/* VIRTQ_AVAIL_F_x : virtio queue driver area flags
 */
#define VIRTQ_AVAIL_F_NO_INTERRUPT 1

/* VIRTQ_USED_F_x : virtio queue device area flags
 */
#define VIRTQ_USED_F_NO_NOTIFY 1

/* virtio_queue_t : structure representing a single virtio queue
 */
typedef struct virtio_queue_t {
	uint32_t queue_num_max;
	uint32_t queue_num;
	uint32_t queue_ready;

	guest_paddr queue_desc;
	guest_paddr queue_driver;
	guest_paddr queue_device;

	uint16_t queue_device_index;
	uint16_t queue_driver_index;
} virtio_queue_t;

/* virtio_desc_t : structure representing a signle virtio descriptor
 */
typedef struct virtio_desc_t {
	size_t desc_index;
	size_t bufs_len;
	size_t written_len;
	struct {
		uint64_t addr;
		uint32_t len;
		uint16_t flags;
	} bufs[16];
} virtio_desc_t;

// NOTE : forward declaration for virtio_t
typedef struct virtio_t virtio_t;

/* virtio_free_handler_t : typedef for the virtio device free handler
 *                         it will be called when the device should be destroyed
 */
typedef void (*virtio_free_handler_t)(emulator_t*, virtio_t*);

/* virtio_req_handler_t : typedef for the virtio request handler
 *                        it will be called when new descriptors have been added to the
 *                        available ring buffer
 */
typedef bool (*virtio_req_handler_t)(emulator_t*, virtio_t*, size_t, virtio_desc_t*);

/* virtio_update_handler_t : typedef for the virtio device update handler
 *                           it will be called regularly to let the device update its state
 */
typedef void (*virtio_update_handler_t)(emulator_t*, virtio_t*);

/* virtio_config_{r,w}x_handler_t : typedef for the virtio device config space R/W handlers
 */
typedef uint8_t (*virtio_config_r8_handler_t)(emulator_t*, virtio_t*, guest_paddr);
typedef void (*virtio_config_w8_handler_t)(emulator_t*, virtio_t*, guest_paddr, uint8_t);
typedef uint32_t (*virtio_config_r32_handler_t)(emulator_t*, virtio_t*, guest_paddr);
typedef void (*virtio_config_w32_handler_t)(emulator_t*, virtio_t*, guest_paddr, uint32_t);

/* virtio_handlers_t : structure storing the handlers for a virtio device
 */
typedef struct virtio_handlers_t {
	virtio_free_handler_t free_handler;
	virtio_req_handler_t req_handler;
	virtio_update_handler_t update_handler;
	virtio_config_r8_handler_t config_r8_handler;
	virtio_config_w8_handler_t config_w8_handler;
	virtio_config_r32_handler_t config_r32_handler;
	virtio_config_w32_handler_t config_w32_handler;
} virtio_handlers_t;

/* virtio_t : structure representing a virtio device
 */
typedef struct virtio_t {
	guest_paddr base;
	size_t int_number;

	uint32_t device_id;
	uint32_t status;

	uint32_t device_features_sel;
	uint64_t device_features;
	uint32_t driver_features_sel;
	uint64_t driver_features;

	virtio_queue_t* queues;
	size_t queue_size;
	uint32_t queue_sel;

	uint32_t interrupt_status;

	uint32_t config_generation;

	const virtio_handlers_t* handlers;
	void* device_data;
} virtio_t;

/* virtio_read_and_use_desc : read and use a virtio descriptor from a queue if possible
 *                            returns true if a virtio descriptor was available and read
 *     emulator_t* emu         : pointer to the emulator
 *     virtio_t* virtio        : pointer to the virtio device
 *     size_t queue_index      : index of the queue to read from
 *     virtio_desc_t* desc     : pointer to the virtio_desc_t to fill
 *     size_t desc_written_len : number of bytes that will be written to the virtio descriptor
 */
bool virtio_read_and_use_desc(emulator_t* emu, virtio_t* virtio, size_t queue_index, virtio_desc_t* desc, size_t desc_written_len);

/* virtio_create : create and attach a virtio device to the emulator
 *                 returns true if the device was successfully created
 *     emulator_t* emu                   : pointer to the emulator
 *     guest_paddr base                  : base address of the virtio device
 *     size_t int_number                 : interrupt source number if the device is connected to the PLIC
 *     uint32_t device_id                : device id identifying the type of virtio device
 *     size_t queue_size                 : number of virtio queues
 *     uint32_t queue_num_max            : maximum number of virtio desc in each queues
 *     const virtio_handlers_t* handlers : pointer to the device handlers
 *     void* device_data                 : pointer to the private device data
 */
bool virtio_create(emulator_t* emu, guest_paddr base, size_t int_number, uint32_t device_id, size_t queue_size, uint32_t queue_num_max, const virtio_handlers_t* handlers, void* device_data);

#endif
