li t0, -1

lui t1, 0x4141

lui t2, 0x4242      # lui t1, 0x4242 : 04242337
addi t2, t2, 0x337

sw t2, 4(zero)

addi t0, t0, 1
beq t0, zero, -20

# EXPECTED
# t0: 1
# t1: 0x4242000
# t2: 0x04242337
# sp: 16384
