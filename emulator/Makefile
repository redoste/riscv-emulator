include ../Makefile.common

SRC := cpu_decode.c \
       cpu_execute.c \
       emulator.c \
       isa.c \
       main.c \
       mmu_paging_guest_to_host.c \

ifdef SDL
	SRC += emulator_sdl.c
	CFLAGS += -DRISCV_EMULATOR_SDL_SUPPORT
	LDFLAGS += -lSDL2
endif

OBJ := $(SRC:%.c=%.o)
OUT := ../riscv-emulator

.PHONY: all
all: $(OUT)

$(OUT): $(OBJ)
	$(CC) $^ -o $@ $(LDFLAGS)

$(OBJ): %.o: %.c
	$(CC) -MMD -MT $@ $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	-rm -v *.o *.d

-include $(OBJ:%.o=%.d)
