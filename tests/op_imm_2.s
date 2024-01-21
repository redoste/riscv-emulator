li a1, 0xaa
li a2, 0x55
li a3, 0xff

# XORI
xori t0, a1, -1
xori t1, a2, -1
xori t2, a3, -1
xori t3, a1, 0x55
xori t4, a1, 0xff

# ORI
ori s0, a1, -1
ori s1, a2, -1
ori s2, a3, -1
ori s3, a1, 0x55
ori s4, a1, 0xff
ori s10, a1, -2048
ori s11, a3, -2048
ori t5, zero, -256

# ANDI
andi s5, a1, -1
andi s6, a2, -1
andi s7, a3, -1
andi s8, a1, 0x55
andi s9, a1, 0xff

# EXPECTED
# a1: 0xaa
# a2: 0x55
# a3: 0xff
# t0: 0xffffffffffffff55
# t1: 0xffffffffffffffaa
# t2: 0xffffffffffffff00
# t3: 0xff
# t4: 0x55
# s0: 0xffffffffffffffff
# s1: 0xffffffffffffffff
# s2: 0xffffffffffffffff
# s3: 0xff
# s4: 0xff
# s5: 0xaa
# s6: 0x55
# s7: 0xff
# s8: 0x00
# s9: 0xaa
# s10: 0xfffffffffffff8aa
# s11: 0xfffffffffffff8ff
# t5: 0xffffffffffffff00
# sp: 16384
