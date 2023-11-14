EXE_ASM := riscv-assembler
SRC_ASM := $(wildcard assembler/*.c)
HDR_ASM := $(wildcard assembler/*.h)

EXE_EMU := riscv-emulator
SRC_EMU := $(wildcard emulator/*.c)
HDR_EMU := $(wildcard emulator/*.h)

TEST_DIR := tests/

all: $(EXE_ASM) $(EXE_EMU)

CFLAGS := -Wall -Wextra -Wpedantic -Wstrict-prototypes -Wvla -O0 -g

$(EXE_ASM): $(SRC_ASM) $(HDR_ASM)
	gcc $(SRC_ASM) -o $@ $(CFLAGS)

$(EXE_EMU): $(SRC_EMU) $(HDR_EMU)
	gcc $(SRC_EMU) -o $@ $(CFLAGS)

.PHONY: all cleanall clean

cleanall: clean
	@ rm -f $(EXE_EMU) $(EXE_ASM)

clean:
	@ find tests \( -name '*.o' -o -name '*.bin' -o -name '*.hex' -o -name '*.state' \) -delete

.PHONY: format
format:
	git ls-files '*.c' '*.h' | xargs clang-format -i
