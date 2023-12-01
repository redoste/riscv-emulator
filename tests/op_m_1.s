lui a0, 0x12345
addi a0, a0, 0x123
slli a0, a0, 32
addi a0, a0, 0x456

lui a1, 0x98765
addi a1, a1, 0x123
slli a1, a1, 32
addi a1, a1, 0x456

# MUL
mul t0, a0, a0
mul t1, a0, a1

# MULH
mulh t2, a0, a0
mulh t3, a0, a1

# MULHSU
mulhsu t4, a0, a0
mulhsu t5, a0, a1
mulhsu t6, a1, a0

# MULHU
mulhu s0, a0, a0
mulhu s1, a0, a1

# EXPECTED
# a0: 0x1234512300000456
# a1: 0x9876512300000456
# t0: 0xddaf9b840012cce4
# t1: 0xffdb9b840012cce4
# t2: 0x014b6619fbef2b66
# t3: 0xf8a328ec38f52956
# t4: 0x014b6619fbef2b66
# t5: 0x0ad77a0f38f52dac
# t6: 0xf8a328ec38f52956
# s0: 0x014b6619fbef2b66
# s1: 0x0ad77a0f38f52dac
