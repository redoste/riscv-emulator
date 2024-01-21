li t0, -1

lui t1, 0x4141

li t2, 0x24 # lui t1, 0x4241 : 04241337

sb t2, 6(zero)

addi t0, t0, 1
beq t0, zero, -16

# EXPECTED
# t0: 1
# t1: 0x4241000
# t2: 0x24
# sp: 16384
