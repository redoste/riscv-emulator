mv a0, zero
li a1, -1
li a2, 100
li a3, -200
li a4, 0x80
slli a4, a4, 56
li a5, 1

# ADD
add t0, a2, a3
add t1, a2, a2
add t2, a0, a1
add t3, a4, a1
add t4, a1, a5

# SUB
sub s0, a2, a3
sub s1, a2, a2
sub s2, a0, a1
sub s3, a4, a1
sub s4, a0, a5

# SLT
slt s5, a1, a2
slt s6, a2, a1
slt s7, a5, s2

# SLTU
sltu s8, a1, a2
sltu s9, a2, a1
sltu s10, a5, s2

# EXPECTED
# a0: 0
# a1: -1
# a2: 100
# a3: -200
# a4: -0x8000000000000000
# a5: 1
# t0: -100
# t1: 200
# t2: -1
# t3: 0x7fffffffffffffff
# t4: 0
# s0: 300
# s1: 0
# s2: 1
# s3: -0x7fffffffffffffff
# s4: -1
# s5: 1
# s6: 0
# s7: 0
# s8: 0
# s9: 1
# s10: 0
