li a0, 0xc
slli a0, a0, 28

lui a1, 0x55555
addi a1, a1, 0x555
slli t0, a1, 32
or a1, a1, t0
sd a1, 0(a0)

lui a2, 0x12345
addi a2, a2, 0x123
slli t0, a2, 32
or a2, a2, t0

# AMOSWAP.D
amoswap.d.aq a3, a2, (a0)
ld a4, 0(a0)

# AMOADD.D
amoadd.d.rl a5, a1, (a0)
ld a6, 0(a0)

# AMOXOR.D
amoxor.d.rl a7, a1, (a0)
ld s0, 0(a0)

# AMOAND.D
amoand.d.aqrl s1, a1, (a0)
ld s2, 0(a0)

# AMOOR.D
amoor.d.aqrl s3, a1, (a0)
ld s4, 0(a0)

# LR.D
lr.d s5, (a0)

# SC.D
addi t0, a0, 8
li t1, -1

lr.d.aq x0, (t0)
sc.d.rl s6, t1, (t0)
lr.d.aq s7, (t0)

# EXPECTED
# a0: 0xc0000000
# a1: 0x5555555555555555
# a2: 0x1234512312345123
# a3: 0x5555555555555555
# a4: 0x1234512312345123
# a5: 0x1234512312345123
# a6: 0x6789a6786789a678
# a7: 0x6789a6786789a678
# s0: 0x32dcf32d32dcf32d
# s1: 0x32dcf32d32dcf32d
# s2: 0x1054510510545105
# s3: 0x1054510510545105
# s4: 0x5555555555555555
# s5: 0x5555555555555555
# t0: 0xc0000008
# t1: -1
# s6: 0
# s7: -1
# sp: 16384
