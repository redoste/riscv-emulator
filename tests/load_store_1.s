# We assume the RAM is at 0xc0000000
li a0, 0xc0
slli a0, a0, 24
addi a1, a0, 0x100

li a2, 0xaa
li a3, 0x55
slli a3, a3, 8
addi a3, a3, 0x55

# SD
sd a2, 0(a0)
sd a2, 8(a0)
sd a3, 0(a1)
sd zero, -8(a1)

# Known memory state :
# c0000000  aa 00 00 00 00 00 00 00  aa 00 00 00 00 00 00 00
# [...]
# c00000f0                           00 00 00 00 00 00 00 00
# c0000100  55 55 00 00 00 00 00 00

# LD
ld t0, 0(a0)
ld t1, 8(a0)
ld t2, 0(a1)
ld t3, 1(a0)
ld t4, 4(a0)
ld t5, -1(a1)
ld t6, -6(a1)

# EXPECTED
# a0: 0xc0000000
# a1: 0xc0000100
# a2: 0xaa
# a3: 0x5555
# t0: 0xaa
# t1: 0xaa
# t2: 0x5555
# t3: 0xaa00000000000000
# t4: 0xaa00000000
# t5: 0x555500
# t6: 0x5555000000000000
