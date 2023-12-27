li a0, 0xc
slli a0, a0, 28

lui a1, 0x55555
addi a1, a1, 0x555
sw a1, 0(a0)

lui a2, 0x12345
addi a2, a2, 0x123

# AMOSWAP.W
amoswap.w.aq a3, a2, (a0)
lwu a4, 0(a0)

# AMOADD.W
amoadd.w.rl a5, a1, (a0)
lwu a6, 0(a0)

# AMOXOR.W
amoxor.w.rl a7, a1, (a0)
lwu s0, 0(a0)

# AMOAND.W
amoand.w.aqrl s1, a1, (a0)
lwu s2, 0(a0)

# AMOOR.W
amoor.w.aqrl s3, a1, (a0)
lwu s4, 0(a0)

# LR.W
lr.w s5, (a0)

# SC.W
addi t0, a0, 8
li t1, -1

lr.w.aq x0, (t0)
sc.w.rl s6, t1, (t0)
lr.w.aq s7, (t0)

# EXPECTED
# a0: 0xc0000000
# a1: 0x55555555
# a2: 0x12345123
# a3: 0x55555555
# a4: 0x12345123
# a5: 0x12345123
# a6: 0x6789a678
# a7: 0x6789a678
# s0: 0x32dcf32d
# s1: 0x32dcf32d
# s2: 0x10545105
# s3: 0x10545105
# s4: 0x55555555
# s5: 0x55555555
# t0: 0xc0000008
# t1: -1
# s6: 0
# s7: -1
