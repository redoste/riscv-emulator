li a0, 10
li a1, 100
li a2, -10
mv t0, zero # correct branches count
mv t1, zero # incorrect branches count

# BEQ
beq zero, zero, 8 # PC
addi t1, t1, 1    # PC+4
addi t0, t0, 1    # PC+8

beq a0, a2, 12 # PC
addi t0, t0, 1 # PC+4
j 8            # PC+8    PC
addi t1, t1, 1 # PC+12   PC+4
               #         PC+8

mv s0, a2
addi s0, s0, -1
addi s0, s0, 1 # PC-4
beq s0, a2, -4 # PC

# BNE
bne zero, zero, 12 # PC
addi t0, t0, 1     # PC+4
j 8                # PC+8   PC
addi t1, t1, 1     # PC+12  PC+4
                   #        PC+8

bne a0, a2, 8  # PC
addi t1, t1, 1 # PC+4
addi t0, t0, 1 # PC+8

mv s1, zero
addi s1, s1, 1 # PC-4
bne s1, a0, -4 # PC

# BLT
blt a2, a0, 8  # PC
addi t1, t1, 1 # PC+4
addi t0, t0, 1 # PC+8

blt a0, a2, 12 # PC
addi t0, t0, 1 # PC+4
j 8            # PC+8   PC
addi t1, t1, 1 # PC+12  PC+4
               #        PC+8

blt a0, s1, 12 # PC
addi t0, t0, 1 # PC+4
j 8            # PC+8   PC
addi t1, t1, 1 # PC+12  PC+4
               #        PC+8

# BGE
bge a2, a0, 12 # PC
addi t0, t0, 1 # PC+4
j 8            # PC+8   PC
addi t1, t1, 1 # PC+12  PC+4
               #        PC+8

bge a0, a2, 8  # PC
addi t1, t1, 1 # PC+4
addi t0, t0, 1 # PC+8

bge a0, s1, 8  # PC
addi t1, t1, 1 # PC+4
addi t0, t0, 1 # PC+8

# EXPECTED
# a0: 10
# a1: 100
# a2: -10
# t0: 10
# t1: 0
# s0: -9
# s1: 10
# sp: 16384
