# FENCE
fence i, iorw
fence iorw, i
fence io, rw
fence ow, ir
fence iorw, 0
fence 0, ow
fence 0, 0

# EBREAK
ebreak

# ECALL
lui a0, 0x50495
addi a0, a0, -0x1b9
mv a1, a0
ecall
# The 0x50494e47 "PING" emucall will write 0x504f4e47 "PONG" in a0

# EXPECTED
# a0: 0x504f4e47
# a1: 0x50494e47
