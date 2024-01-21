auipc a1, 0

li t0, 0x5
sb t0, 18(a1)
# Workaround used to force QEMU to invalidate the next instruction
beq x0, x0, 4

lui a0, 0x41

# EXPECTED
# a1: 0
# t0: 5
# a0: 0x51000
# sp: 16384
