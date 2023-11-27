li t0, -1

lui t1, 0x4141

lui t2, 0x4242      # lui t1, 0x4242 : 04242337
addi t2, t2, 0x337

lui a0, 0x40000
slli a0, a0, 1
sw t2, 4(a0)

addi t0, t0, 1
beq t0, zero, -28

# EXPECTED
# t0: 1
# t1: 0x4242000
# t2: 0x04242337
# a0: 0x80000000
