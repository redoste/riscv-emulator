#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "device_uart8250.h"
#include "devices.h"
#include "emulator.h"
#include "isa.h"

#define COM_DATA_REG          0
#define COM_BAUD_LSB_REG      0
#define COM_INT_ENABLE_REG    1
#define COM_BAUD_MSB_REG      1
#define COM_INT_ID_REG        2
#define COM_FIFO_CONTROL_REG  2
#define COM_LINE_CONTROL_REG  3
#define COM_MODEM_CONTROL_REG 4
#define COM_LINE_STATUS_REG   5
#define COM_MODEM_STATUS_REG  6
#define COM_SCRATCH_REG       7

typedef struct uart8250_t {
	guest_paddr base;
	int fd_tx;
	int fd_rx;

	uint16_t baud_rate_divisor;
	uint8_t int_enable_reg;
	uint8_t int_id_reg;
	uint8_t fifo_control_reg;
	uint8_t line_control_reg;
	uint8_t modem_control_reg;
} uart8250_t;

static void uart_free(emulator_t* emu, void* device_data) {
	(void)emu;

	uart8250_t* uart = (uart8250_t*)device_data;
	free(uart);
}

static uint8_t uart8250_r8(emulator_t* emu, void* device_data, guest_paddr addr) {
	uart8250_t* uart = (uart8250_t*)device_data;
	switch (addr) {
		case COM_DATA_REG: {  // or COM_BAUD_LSB_REG
			if (uart->line_control_reg & (1 << 7) /* DLAB */) {
				return uart->baud_rate_divisor & 0xff;
			} else {
				uint8_t byte;
				ssize_t read_bytes = read(uart->fd_rx, &byte, 1);
				return read_bytes >= 1 ? byte : 0;
			}
		}
		case COM_INT_ENABLE_REG: {  // or COM_BAUD_MSB_REG
			if (uart->line_control_reg & (1 << 7) /* DLAB */) {
				return uart->baud_rate_divisor >> 8;
			} else {
				return uart->int_enable_reg;
			}
		}
		case COM_INT_ID_REG: {
			return uart->int_id_reg;
		}
		case COM_LINE_CONTROL_REG: {
			return uart->line_control_reg;
		}
		case COM_MODEM_CONTROL_REG: {
			return uart->modem_control_reg;
		}
		case COM_LINE_STATUS_REG: {
			bool dr = false;

			int bytes_fd;
			if (ioctl(uart->fd_rx, FIONREAD, &bytes_fd) == 0) {
				dr = bytes_fd > 0;
			}

			return (1 << 5) | /* Transmitter Holding Register Empty */
			       (dr << 0) /* Data Ready */;
		}
		case COM_MODEM_STATUS_REG: {
			return 0;
		}
		case COM_SCRATCH_REG: {
			return 0;
		}
		default: {
			cpu_throw_exception(emu, EXC_LOAD_ACCESS_FAULT, uart->base + addr);
			return 0;
		}
	}
}

static uint16_t uart8250_r16(emulator_t* emu, void* device_data, guest_paddr addr) {
	uart8250_t* uart = (uart8250_t*)device_data;
	cpu_throw_exception(emu, EXC_LOAD_ACCESS_FAULT, uart->base + addr);
	return 0;
}

static uint32_t uart8250_r32(emulator_t* emu, void* device_data, guest_paddr addr) {
	uart8250_t* uart = (uart8250_t*)device_data;
	cpu_throw_exception(emu, EXC_LOAD_ACCESS_FAULT, uart->base + addr);
	return 0;
}

static uint64_t uart8250_r64(emulator_t* emu, void* device_data, guest_paddr addr) {
	uart8250_t* uart = (uart8250_t*)device_data;
	cpu_throw_exception(emu, EXC_LOAD_ACCESS_FAULT, uart->base + addr);
	return 0;
}

static void uart8250_w8(emulator_t* emu, void* device_data, guest_paddr addr, uint8_t value) {
	uart8250_t* uart = (uart8250_t*)device_data;
	switch (addr) {
		case COM_DATA_REG: {  // or COM_BAUD_LSB_REG
			if (uart->line_control_reg & (1 << 7) /* DLAB */) {
				uart->baud_rate_divisor = (uart->baud_rate_divisor & 0xff00) | value;
			} else {
				write(uart->fd_tx, &value, 1);
			}
			break;
		}
		case COM_INT_ENABLE_REG: {  // or COM_BAUD_MSB_REG
			if (uart->line_control_reg & (1 << 7) /* DLAB */) {
				uart->baud_rate_divisor = (uart->baud_rate_divisor & 0xff) | (value << 8);
			} else {
				uart->int_enable_reg = value & 0x0f;
			}
			break;
		}
		case COM_FIFO_CONTROL_REG: {
			uart->fifo_control_reg = value & 0xcf;
			break;
		}
		case COM_LINE_CONTROL_REG: {
			uart->line_control_reg = value;
			break;
		}
		case COM_MODEM_CONTROL_REG: {
			uart->modem_control_reg = value & 0x1b;
			break;
		}
		case COM_LINE_STATUS_REG:
		case COM_MODEM_STATUS_REG:
		case COM_SCRATCH_REG: {
			break;
		}
		default: {
			cpu_throw_exception(emu, EXC_STORE_ACCESS_FAULT, uart->base + addr);
			break;
		}
	}
}

static void uart8250_w16(emulator_t* emu, void* device_data, guest_paddr addr, uint16_t value) {
	(void)value;

	uart8250_t* uart = (uart8250_t*)device_data;
	cpu_throw_exception(emu, EXC_LOAD_ACCESS_FAULT, uart->base + addr);
}

static void uart8250_w32(emulator_t* emu, void* device_data, guest_paddr addr, uint32_t value) {
	(void)value;

	uart8250_t* uart = (uart8250_t*)device_data;
	cpu_throw_exception(emu, EXC_LOAD_ACCESS_FAULT, uart->base + addr);
}

static void uart8250_w64(emulator_t* emu, void* device_data, guest_paddr addr, uint64_t value) {
	(void)value;

	uart8250_t* uart = (uart8250_t*)device_data;
	cpu_throw_exception(emu, EXC_LOAD_ACCESS_FAULT, uart->base + addr);
}

bool uart8250_create(emulator_t* emu, guest_paddr base, int fd_tx, int fd_rx) {
	uart8250_t* uart = malloc(sizeof(uart8250_t));
	assert(uart != NULL);
	memset(uart, 0, sizeof(*uart));
	uart->base = base;
	uart->fd_tx = fd_tx;
	uart->fd_rx = fd_rx;

	device_mmio_t device = {
		uart,
		uart_free,
		NULL,
		uart8250_r8,
		uart8250_r16,
		uart8250_r32,
		uart8250_r64,
		uart8250_w8,
		uart8250_w16,
		uart8250_w32,
		uart8250_w64,
	};

	int fd_rx_flags = fcntl(fd_rx, F_GETFL);
	if (fd_rx_flags < 0) {
		free(uart);
		return false;
	}
	if (fcntl(fd_rx, F_SETFL, fd_rx_flags | O_NONBLOCK) < 0) {
		free(uart);
		return false;
	}

	if (!emu_add_mmio_device(emu, base, 0x1000, &device)) {
		free(uart);
		return false;
	}
	return true;
}
