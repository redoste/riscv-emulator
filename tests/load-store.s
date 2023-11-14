ld t0, -16(a0)
sd t0, -32(a3)

ld ra, -16(sp)
sd ra, -16(sp)

ld x31, 0(x31)
sd x31, 0(x31)

ld a0, 2047(s0)
ld a0, -2048(s0)
sd a0, 2047(s0)
sd a0, -2048(s0)
