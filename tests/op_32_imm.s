lui a0, 0x55555
addi a0, a0, 0x555
slli a0, a0, 32

li a1, -1
srli a1, a1, 32
or a1, a1, a0

li a2, 1
or a2, a2, a0

li a3, 0x700
or a3, a3, a0

# ADDIW
addiw t0, a0, 1
addiw t1, a0, -1
addiw t2, a1, 1
addiw t3, a1, -1
addiw t4, a2, 1
addiw t5, a2, -1
addiw t6, a2, 0
addiw s0, a1, -2048
addiw s1, a1, 2047

# SLLIW
slliw s2, a1, 4
slliw s3, a1, 7
slliw s4, a1, 31
slliw a5, a3, 1

# SRLIW
srliw s5, a1, 4
srliw s6, a1, 7
srliw s7, a1, 31
srliw a4, a3, 1

# SRAIW
sraiw s8, a1, 4
sraiw s9, a1, 7
sraiw s10, a1, 31
sraiw s11, a3, 1

# EXPECTED
# a0: 0x5555555500000000
# a1: 0x55555555ffffffff
# a2: 0x5555555500000001
# a3: 0x5555555500000700
# t0: 1
# t1: -1
# t2: 0
# t3: -2
# t4: 2
# t5: 0
# t6: 1
# s0: -2049
# s1: 2046
# s2: 0xfffffffffffffff0
# s3: 0xffffffffffffff80
# s4: 0xffffffff80000000
# a5: 0xe00
# s5: 0x0fffffff
# s6: 0x01ffffff
# s7: 1
# a4: 0x380
# s8: -1
# s9: -1
# s10: -1
# s11: 0x380
