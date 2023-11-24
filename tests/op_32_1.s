lui a0, 0x55555
addi a0, a0, 0x555
slli a0, a0, 32

li a1, -1
srli a1, a1, 32
or a1, a1, a0

li a2, 100
li a3, -200

li a4, 1
or a4, a4, a0

# ADDW
addw t0, a1, a4
addw t1, a2, a4
addw t2, a3, a1
addw t3, a3, a4

# SUBW
subw t4, a1, a4
subw t5, a2, a4
subw t6, a3, a1
subw s0, a3, a4

# EXPECTED
# a0: 0x5555555500000000
# a1: 0x55555555ffffffff
# a2: 100
# a3: -200
# a4: 0x5555555500000001
# t0: 0
# t1: 101
# t2: -201
# t3: -199
# t4: -2
# t5: 99
# t6: -199
# s0: -201
