# The LSBs should be cleared
li t0, 0x555
li t1, 0x555
li t2, 0x555
li t3, 0x555
li t4, 0x555
li t5, 0x555
li t6, 0x555
li s0, 0x555
li s1, 0x555

# AUIPC
auipc t0, 0
auipc t1, 0xfffff
auipc t2, 1
auipc t3, 0x1000

# LUI
lui t4, 1
lui t5, 0xfffff
lui t6, 0
lui s0, 0x1000
lui s1, 0x80000

# EXPECTED
# t0: 0x80000024
# t1: 0x7ffff028
# t2: 0x8000102c
# t3: 0x81000030
# t4: 0x1000
# t5: 0xfffffffffffff000
# t6: 0
# s0: 0x1000000
# s1: 0xffffffff80000000
