li a0, 0xc
slli a0, a0, 28

li a1, -1
li a2, 1
li a3, 100
li a4, -100

sd a1, 0(a0)

# AMOMIN.D
amomin.d a5, a2, (a0)
ld a6, 0(a0)

# AMOMAX.D
amomax.d a7, a3, (a0)
ld s0, 0(a0)

# AMOMINU.D
amominu.d s1, a4, (a0)
ld s2, 0(a0)

amominu.d s3, a2, (a0)
ld s4, 0(a0)

# AMOMAXU.D
amomaxu.d s5, a1, (a0)
ld s6, 0(a0)

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
