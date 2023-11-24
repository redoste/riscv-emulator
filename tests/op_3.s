li a0, -1
li a1, 0xaa
li a2, 0x55
li a3, 0xff

# XOR
xor t0, a0, a1
xor t1, a0, a2
xor t2, a0, a3
xor t3, a1, a2
xor t4, a1, a3

# OR
or s0, a0, a1
or s1, a0, a2
or s2, a0, a3
or s3, a1, a2
or s4, a1, a3

# AND
and s5, a0, a1
and s6, a0, a2
and s7, a0, a3
and s8, a1, a2
and s9, a1, a3

# EXPECTED
# a0: 0xffffffffffffffff
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
