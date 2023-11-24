li a0, -1
li a1, 0xaa
li a2, 4
li a3, 31
li a4, 65

# SLL
sll t0, a1, a2
sll t1, a1, a3
sll t2, a1, a4
sll t3, a0, a2
sll t4, a0, a3
sll t5, a0, a4

# SRL
srl s0, a1, a2
srl s1, a1, a3
srl s2, a1, a4
srl s3, a0, a2
srl s4, a0, a3
srl s5, a0, a4

# SRA
sra s6, a1, a2
sra s7, a1, a3
sra s8, a1, a4
sra s9, a0, a2
sra s10, a0, a3
sra s11, a0, a4

# EXPECTED
# a0: -1
# a1: 0xaa
# a2: 4
# a3: 31
# a4: 65
# t0: 0xaa0
# t1: 0x5500000000
# t2: 0x154
# t3: 0xfffffffffffffff0
# t4: 0xffffffff80000000
# t5: 0xfffffffffffffffe
# s0: 0xa
# s1: 0
# s2: 0x55
# s3: 0x0fffffffffffffff
# s4: 0x1ffffffff
# s5: 0x7fffffffffffffff
# s6: 0xa
# s7: 0
# s8: 0x55
# s9: 0xffffffffffffffff
# s10: 0xffffffffffffffff
# s11: 0xffffffffffffffff
