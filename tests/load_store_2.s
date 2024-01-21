# We assume the RAM is at 0xc0000000
li a0, 0xc0
slli a0, a0, 24
lui a1, 0x1
add a1, a0, a1

li a2, -1

# Try to store and load across a page boundary
sd a2, -4(a1)
ld a3, -4(a1)

sh a2, -1(a1)
lhu a4, -1(a1)

# EXPECTED
# a0: 0xc0000000
# a1: 0xc0001000
# a2: -1
# a3: -1
# a4: 0xffff
# sp: 16384
