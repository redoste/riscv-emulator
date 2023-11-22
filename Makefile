EXE_ASM := riscv-assembler
EXE_EMU := riscv-emulator

.PHONY: all cleanall clean
all: $(EXE_ASM) $(EXE_EMU)

.PHONY: $(EXE_ASM)
$(EXE_ASM):
	$(MAKE) -C assembler/

.PHONY: $(EXE_EMU)
$(EXE_EMU):
	$(MAKE) -C emulator/

cleanall: clean
	@ rm -f $(EXE_EMU) $(EXE_ASM)

clean:
	@ find tests \( -name '*.o' -o -name '*.bin' -o -name '*.hex' -o -name '*.state' \) -delete
	$(MAKE) -C assembler/ clean
	$(MAKE) -C emulator/ clean

.PHONY: format
format:
	git ls-files '*.c' '*.h' | xargs clang-format -i
