li a0, 10
li a1, 100
li a2, -10
mv s1, a0
mv t0, zero # correct branches count
mv t1, zero # incorrect branches count

# BLTU
bltu a2, a0, 12 # PC
addi t0, t0, 1  # PC+4
j 8             # PC+8   PC
addi t1, t1, 1  # PC+12  PC+4
                #        PC+8


bltu a0, a2, 8 # PC
addi t1, t1, 1 # PC+4
addi t0, t0, 1 # PC+8

bltu a0, s1, 12 # PC
addi t0, t0, 1  # PC+4
j 8             # PC+8   PC
addi t1, t1, 1  # PC+12  PC+4
                #        PC+8

# BGEU
bgeu a2, a0, 8 # PC
addi t1, t1, 1 # PC+4
addi t0, t0, 1 # PC+8

bgeu a0, a2, 12 # PC
addi t0, t0, 1  # PC+4
j 8             # PC+8   PC
addi t1, t1, 1  # PC+12  PC+4
                #        PC+8

bgeu a0, s1, 8 # PC
addi t1, t1, 1 # PC+4
addi t0, t0, 1 # PC+8

# EXPECTED
# a0: 10
# a1: 100
# a2: -10
# s1: 10
# t0: 6
# t1: 0
# sp: 16384
