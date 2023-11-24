mv t0, zero
mv t1, zero

auipc ra, 0     # PC
jalr x0, 28(ra) # PC+4
addi t1, t1, 1  # PC+8
addi t1, t1, 1  # PC+12
addi t1, t1, 1  # PC+16
addi t0, t0, 1  # PC+20
addi t0, t0, 1  # PC+24
addi t0, t0, 1  # PC+28

# The LSB of the immediate should be ignored
auipc ra, 0     # PC
jalr x0, 29(ra) # PC+4
addi t1, t1, 1  # PC+8
addi t1, t1, 1  # PC+12
addi t1, t1, 1  # PC+16
addi t0, t0, 1  # PC+20
addi t0, t0, 1  # PC+24
addi t0, t0, 1  # PC+28

# We test overwriting ra while using it as a base
auipc ra, 0     # PC
jalr ra, 29(ra) # PC+4
addi t1, t1, 1  # PC+8
addi t1, t1, 1  # PC+12
addi t1, t1, 1  # PC+16
addi t0, t0, 1  # PC+20
addi t0, t0, 1  # PC+24
addi t0, t0, 1  # PC+28

j 32             # PC
addi t1, t1, 1   # PC+4
addi t1, t1, 1   # PC+8
addi t1, t1, 1   # PC+12
addi t0, t0, 1   # PC+16   PC-16
addi t0, t0, 1   # PC+20   PC-12
addi t0, t0, 1   # PC+24   PC-8
jalr x0, 0(a0)   # PC+28   PC-4
auipc a0, 0      # PC+32   PC
jalr a0, -12(a0)

# EXPECTED
# t0: 5
# t1: 0
# ra: 0x80000050
# a0: 0x80000090
