li a0, -1
li a1, 0xaa
li a2, 4
li a3, 15
li a4, 33

# SLLW
sllw t0, a1, a2
sllw t1, a1, a3
sllw t2, a1, a4
sllw t3, a0, a2
sllw t4, a0, a3
sllw t5, a0, a4

# SRLW
srlw s0, a1, a2
srlw s1, a1, a3
srlw s2, a1, a4
srlw s3, a0, a2
srlw s4, a0, a3
srlw s5, a0, a4

# SRAW
sraw s6, a1, a2
sraw s7, a1, a3
sraw s8, a1, a4
sraw s9, a0, a2
sraw s10, a0, a3
sraw s11, a0, a4

# EXPECTED
# a0: -1
# a1: 0xaa
# a2: 4
# a3: 15
# a4: 33
# t0: 0xaa0
# t1: 0x550000
# t2: 0x154
# t3: 0xfffffffffffffff0
# t4: 0xffffffffffff8000
# t5: 0xfffffffffffffffe
# s0: 0xa
# s1: 0
# s2: 0x55
# s3: 0x0fffffff
# s4: 0x1ffff
# s5: 0x7fffffff
# s6: 0xa
# s7: 0
# s8: 0x55
# s9: 0xffffffffffffffff
# s10: 0xffffffffffffffff
# s11: 0xffffffffffffffff
