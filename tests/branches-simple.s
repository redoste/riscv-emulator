beq x31, x31, 0xaa
bne x31, x31, 0xaa
blt x31, x31, 0xaa
bge x31, x31, 0xaa

beq x16, x16, 0xfaa
bne x16, x16, 0xfaa
blt x16, x16, 0xfaa
bge x16, x16, 0xfaa

beq x16, x16, 0xffe
bne x16, x16, 0xffe
blt x16, x16, 0xffe
bge x16, x16, 0xffe

beq x0, x0, 0
bne x0, x0, 0
blt x0, x0, 0
bge x0, x0, 0

jal ra, 0x80
jal x31, 0xaaa

jal ra, -1048576
jal ra, 1048574

beq x0, x0, -4096
beq x0, x0, 4094
