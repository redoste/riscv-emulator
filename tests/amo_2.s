li a0, 0xc
slli a0, a0, 28

li a1, -1
li a2, 1
li a3, 100
li a4, -100

sw a1, 0(a0)

# AMOMIN.W
amomin.w a5, a2, (a0)
lw a6, 0(a0)

# Unicorn bug : the operands of amomin.w aren't properly sign-extended, the expected result
#               was manually checked with qemu-system-riscv64 v8.1.3
# SKIP UC

# AMOMAX.W
amomax.w a7, a3, (a0)
lw s0, 0(a0)

# AMOMINU.W
amominu.w s1, a4, (a0)
lw s2, 0(a0)

amominu.w s3, a2, (a0)
lw s4, 0(a0)

# AMOMAXU.W
amomaxu.w s5, a1, (a0)
lw s6, 0(a0)

# EXPECTED
# a0: 0xc0000000
# a1: -1
# a2: 1
# a3: 100
# a4: -100
# a5: -1
# a6: -1
# a7: -1
# s0: 100
# s1: 100
# s2: 100
# s3: 100
# s4: 1
# s5: 1
# s6: -1
