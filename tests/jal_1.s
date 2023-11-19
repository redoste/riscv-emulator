mv t0, zero
mv t1, zero

j 16           # PC
addi t1, t1, 1 # PC+4
addi t1, t1, 1 # PC+8
addi t1, t1, 1 # PC+12
               # PC+16

jal ra, 16     # PC
addi t1, t1, 1 # PC+4
addi t1, t1, 1 # PC+8
addi t1, t1, 1 # PC+12
               # PC+16

j 16                 # PC
addi t1, t1, 1       # PC+4
addi t0, t0, 1       # PC+8   PC-20
addi t0, t0, 1       # PC+12  PC-16
add zero, zero, zero # PC+16  PC-12
add zero, zero, zero #        PC-8
blt zero, t0, 8      #        PC-4   PC
jal s0, -20          #        PC     PC+4
                     #               PC+8

# EXPECTED
# t0: 2
# t1: 0
# ra: 0x8000001c
# s0: 0x80000048
