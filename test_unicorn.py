#!/usr/bin/env python3
import sys
import struct

try:
    from unicorn import *
    from unicorn.riscv_const import *
except ImportError:
    print("Unable to find unicorn with python bindings and RISC-V support in PYTHONPATH or LD_LIBRARY_PATH", file=sys.stderr)
    sys.exit(2)

DEFAULT_ROM_BASE = 0x80000000
DEFAULT_ROM_SIZE = 0x2000
DEFAULT_RAM_BASE = 0xc0000000
DEFAULT_RAM_SIZE = 0x2000

def hook_ecall(uc, intno, _):
    if intno != 8:
        raise Exception("unexpected interrupt {}".format(intno))

    a0 = uc.reg_read(UC_RISCV_REG_A0)
    if a0 == 0x50494e47: # "PING"
        uc.reg_write(UC_RISCV_REG_A0, 0x504f4e47) # "PONG"
    else:
        raise Exception("invalid ecall a0=0x{:x}".format(a0))

def hook_insn_invalid(uc):
    pc = uc.reg_read(UC_RISCV_REG_PC)
    insn = uc.mem_read(pc, 4)
    if bytes(insn) == struct.pack("<I", 0x00100073):
        # we ignore ebreaks
        uc.reg_write(UC_RISCV_REG_PC, pc + 4)
    else:
        raise UcError(UC_ERR_INSN_INVALID)

def main():
    if len(sys.argv) != 3:
        print("Usage : {} <HEX INPUT> <EMULATION OUTPUT>".format(sys.argv[0]), file=sys.stderr)
        sys.exit(1)

    uc = Uc(UC_ARCH_RISCV, UC_MODE_RISCV64)
    uc.mem_map(DEFAULT_ROM_BASE, DEFAULT_ROM_SIZE)
    uc.mem_map(DEFAULT_RAM_BASE, DEFAULT_RAM_SIZE)

    uc.hook_add(UC_HOOK_INTR, hook_ecall)

    max_rom_code_addr = DEFAULT_ROM_BASE
    with open(sys.argv[1], "r") as input_file:
        for line in input_file.readlines():
            instruction = int(line.strip(), 16)
            uc.mem_write(max_rom_code_addr, struct.pack("<I", instruction))
            max_rom_code_addr += 4

    uc.reg_write(UC_RISCV_REG_PC, DEFAULT_ROM_BASE)
    while True:
        try:
            uc.emu_start(uc.reg_read(UC_RISCV_REG_PC), max_rom_code_addr)
            break
        except UcError as e:
            if e.errno == UC_ERR_INSN_INVALID:
                hook_insn_invalid(uc)
            else:
                raise e

    with open(sys.argv[2], "w") as output_file:
        for i in range(32):
            output_file.write("x{}: 0x{:x}\n".format(i, uc.reg_read(UC_RISCV_REG_X0 + i)))

if __name__ == "__main__":
    main()
