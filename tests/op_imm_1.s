li a0, 100
li a1, 2047
addi a1, a1, 1
li a2, -2047
li a3, 0xaa

# ADDI
addi t0, a0, 200
addi t1, a0, -200
addi t2, a0, -2048
addi t3, a0, 2047
addi t4, a1, -2048
addi t5, a2, 2047

# SLLI
slli s0, a3, 1
slli s1, a3, 4
slli s2, a3, 5
slli s3, a3, 56
slli s4, a3, 57
slli s5, a3, 63

# SLTI
slti s6, a0, 200
slti s7, a2, 0
slti s8, a0, 5
slti s9, a0, 100

# SLTIU
sltiu s10, a0, 200
sltiu s11, a2, 0
sltiu t6, a0, -5
sltiu a7, a2, -2047

# EXPECTED
# a0: 100
# a1: 2048
# a2: -2047
# a3: 0xaa
# t0: 300
# t1: -100
# t2: -1948
# t3: 2147
# t4: 0
# t5: 0
# s0: 0x154
# s1: 0xaa0
# s2: 0x1540
# s3: 0xaa00000000000000
# s4: 0x5400000000000000
# s5: 0
# s6: 1
# s7: 1
# s8: 0
# s9: 0
# s10: 1
# s11: 0
# t6: 1
# a7: 0
# sp: 16384
