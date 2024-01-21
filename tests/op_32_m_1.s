lui a0, 0x55555
addi a0, a0, 0x555
slli a0, a0, 32

lui a1, 0x12345
addi a1, a1, 0x123
or a1, a1, a0

lui a2, 0x98765
addi a2, a2, 0x123
xor a2, a2, a0

li a3, 1
slli a3, a3, 31
or a3, a3, a0

li a4, -1
xor a4, a4, a0

# MULW
mulw t0, a1, a1
mulw t1, a1, a2

# DIVW
divw t2, a1, a2
divw t3, a2, a1
divw t4, a1, a0
divw t5, a3, a4

# DIVUW
divuw t6, a1, a2
divuw s0, a2, a1
divuw s1, a1, a0
divuw s2, a3, a4

# REMW
remw s3, a1, a2
remw s4, a2, a1
remw s5, a1, a0
remw s6, a3, a4

# REMUW
remuw s7, a1, a2
remuw s8, a2, a1
remuw s9, a1, a0
remuw s10, a3, a4

# EXPECTED
# a0: 0x5555555500000000
# a1: 0x5555555512345123
# a2: 0xaaaaaaaa98765123
# a3: 0x5555555580000000
# a4: 0xaaaaaaaaffffffff
# t0: 0xfffffffffbef2ac9
# t1: 0x0000000038f52ac9
# t2: 0
# t3: -5
# t4: 0xffffffffffffffff
# t5: 0xffffffff80000000
# t6: 0
# s0: 8
# s1: -1
# s2: 0
# s3: 0x0000000012345123
# s4: 0xfffffffff37be6d2
# s5: 0x0000000012345123
# s6: 0
# s7: 0x0000000012345123
# s8: 0x0000000006d3c80b
# s9: 0x0000000012345123
# s10: 0xffffffff80000000
# sp: 16384
