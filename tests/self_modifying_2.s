li t0, -1

lui t1, 0x4141

li t2, 0x24 # lui t1, 0x4241 : 04241337

lui a0, 0x40000
slli a0, a0, 1
sb t2, 6(a0)

addi t0, t0, 1
beq t0, zero, -24

# EXPECTED
# t0: 1
# t1: 0x4241000
# t2: 0x24
# a0: 0x80000000
