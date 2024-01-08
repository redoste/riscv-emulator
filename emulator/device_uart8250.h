#ifndef DEVICE_UART8250_H
#define DEVICE_UART8250_H

#include <stdbool.h>

#include "emulator.h"
#include "isa.h"

/* This is an implementation of a 8250 UART
 * <https://www.sci.muni.cz/docs/pc/serport.txt>
 * <https://wiki.osdev.org/Serial_Ports>
 */

/* uart8250_create : create and attach a UART 8250 to the emulator
 *     emulator_t* emu  : pointer to the emulator
 *     guest_paddr base : base address of the UART
 *     int fd_tx        : TX file descriptor (the one being written to)
 *     int fd_rx        : RX file descriptor (the one being read from)
 */
bool uart8250_create(emulator_t* emu, guest_paddr base, int fd_tx, int fd_rx);

#endif
