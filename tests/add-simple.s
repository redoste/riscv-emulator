addi a1, zero, 20
addi a2, zero, 22
add a0, a1, a2

# TODO : what's the initial value for sp ?
#        in my opinion it's up to the guest to setup all of its registers
#        except for pc (assuming it knows the memory map)
li sp, 0x40
slli sp, sp, 8

# EXPECTED
# sp: 16384
# a0: 42
# a1: 20
# a2: 22
