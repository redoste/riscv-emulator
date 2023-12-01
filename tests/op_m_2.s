lui a0, 0x12345
addi a0, a0, 0x123
slli a0, a0, 32
addi a0, a0, 0x456

lui a1, 0x98765
addi a1, a1, 0x123
slli a1, a1, 32
addi a1, a1, 0x456

mv a2, zero

li a3, 1
slli a3, a3, 63

li a4, -1

# DIV
div t0, a0, a1
div t1, a1, a0
div t2, a0, a2
div t3, a2, a0
div t4, a3, a4
div t5, a4, a3

# DIVU
divu s0, a0, a1
divu s1, a1, a0
divu s2, a0, a2
divu s3, a2, a0
divu s4, a3, a4
divu s5, a4, a3

# REM
rem s6, a0, a1
rem s7, a1, a0
rem s8, a0, a2
rem s9, a2, a0
rem s10, a3, a4
rem s11, a4, a3

# REMU
remu a5, a0, a1
remu a6, a1, a0
remu a7, a0, a2
remu x1, a2, a0
remu x2, a3, a4
remu x3, a4, a3

# EXPECTED
# a0: 0x1234512300000456
# a1: 0x9876512300000456
# a2: 0
# a3: 0x8000000000000000
# a4: -1
# t0: 0
# t1: -5
# t2: -1
# t3: 0
# t4: 0x8000000000000000
# t5: 0
# s0: 0
# s1: 8
# s2: -1
# s3: 0
# s4: 0
# s5: 1
# s6: 0x1234512300000456
# s7: 0xf37be6d200001a04
# s8: 0x1234512300000456
# s9: 0
# s10: 0
# s11: -1
# a5: 0x1234512300000456
# a6: 0x6d3c80affffe1a6
# a7: 0x1234512300000456
# x1: 0
# x2: 0x8000000000000000
# x3: 0x7fffffffffffffff
