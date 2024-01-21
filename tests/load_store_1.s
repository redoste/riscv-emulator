# We assume the RAM is at 0xc0000000
li a0, 0xc0
slli a0, a0, 24
addi a1, a0, 0x100

li a2, 0xaa
li a3, 0x55
slli a3, a3, 8
addi a3, a3, 0x55

li a4, -1
addi a5, a4, -4

# SD
sd a2, 0(a0)
sd a2, 8(a0)
sd a3, 0(a1)
sd zero, -8(a1)

# SW
sw zero, 12(a1)
sw a4, 8(a1)
sw a5, 16(a1)

# SH
sh zero, -10(a1)
sh zero, -14(a1)
sh a3, -16(a1)
sh a5, -12(a1)

# SB
sb zero, 17(a0)
sb zero, 19(a0)
sb a3, 16(a0)
sb a2, 18(a0)
sb a3, 20(a0)
sb a3, 21(a0)
sb a3, 22(a0)
sb a4, 23(a0)

# Known memory state :
# c0000000  aa 00 00 00 00 00 00 00  aa 00 00 00 00 00 00 00
# c0000010  55 00 aa 00 55 55 55 ff
# [...]
# c00000f0  55 55 00 00 fb ff 00 00  00 00 00 00 00 00 00 00
# c0000100  55 55 00 00 00 00 00 00  ff ff ff ff 00 00 00 00
# c0000110  fb ff ff ff

# LD
ld t0, 0(a0)
ld t1, 8(a0)
ld t2, 0(a1)
ld t3, 1(a0)
ld t4, 4(a0)
ld t5, -1(a1)
ld t6, -6(a1)
ld s0, 8(a1)
ld s9, -16(a1)
ld s10, 16(a0)

# LW
lw s1, 8(a1)
lw s2, 16(a1)

# LWU
lwu s3, 8(a1)
lwu s4, 16(a1)

# LH
lh s5, -16(a1)
lh s6, -12(a1)

# LHU
lhu s7, -16(a1)
lhu s8, -12(a1)

# LB
lb s11, 18(a0)
lb a6, 23(a0)

# LBU
lbu a7, 18(a0)
lbu x4, 23(a0)

# EXPECTED
# a0: 0xc0000000
# a1: 0xc0000100
# a2: 0xaa
# a3: 0x5555
# a4: -1
# a5: -5
# t0: 0xaa
# t1: 0xaa
# t2: 0x5555
# t3: 0xaa00000000000000
# t4: 0xaa00000000
# t5: 0x555500
# t6: 0x5555000000000000
# s0: 0xffffffff
# s1: -1
# s2: -5
# s3: 0xffffffff
# s4: 0xfffffffb
# s5: 0x5555
# s6: -5
# s7: 0x5555
# s8: 0xfffb
# s9: 0x0000fffb00005555
# s10: 0xff55555500aa0055
# s11: 0xffffffffffffffaa
# a6: -1
# a7: 0xaa
# x4: 0xff
# sp: 16384
