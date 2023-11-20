EXE_ASM := riscv-assembler
SRC_ASM := $(wildcard assembler/*.c)
HDR_ASM := $(wildcard assembler/*.h)

EXE_EMU := riscv-emulator
SRC_EMU := $(wildcard emulator/*.c)
HDR_EMU := $(wildcard emulator/*.h)

TEST_DIR := tests/

all: $(EXE_ASM) $(EXE_EMU)

# TODO : add build option to choose between debug (-O0 -g) and release (-O2 or -O3)
CFLAGS := -Wall -Wextra -Wpedantic -Wstrict-prototypes -Wvla -O3 -g
# TODO : add build option to disable SDL support
LFLAGS_EMU := -lSDL2

$(EXE_ASM): $(SRC_ASM) $(HDR_ASM)
	gcc $(SRC_ASM) -o $@ $(CFLAGS)

$(EXE_EMU): $(SRC_EMU) $(HDR_EMU)
	gcc $(SRC_EMU) -o $@ $(CFLAGS) $(LFLAGS_EMU)

.PHONY: all cleanall clean

cleanall: clean
	@ rm -f $(EXE_EMU) $(EXE_ASM)

clean:
	@ find tests \( -name '*.o' -o -name '*.bin' -o -name '*.hex' -o -name '*.state' \) -delete

.PHONY: format
format:
	git ls-files '*.c' '*.h' | xargs clang-format -i
