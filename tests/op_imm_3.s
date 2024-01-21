li a0, -1
li a1, 0xaa

# SRLI
srli s0, a1, 4
srli s1, a1, 31
srli s2, a1, 63
srli s3, a0, 4
srli s4, a0, 31
srli s5, a0, 63

# SRAI
srai s6, a1, 4
srai s7, a1, 31
srai s8, a1, 63
srai s9, a0, 4
srai s10, a0, 31
srai s11, a0, 63

# EXPECTED
# a0: -1
# a1: 0xaa
# s0: 0xa
# s1: 0
# s2: 0
# s3: 0x0fffffffffffffff
# s4: 0x1ffffffff
# s5: 0x1
# s6: 0xa
# s7: 0
# s8: 0
# s9: 0xffffffffffffffff
# s10: 0xffffffffffffffff
# s11: 0xffffffffffffffff
# sp: 16384
