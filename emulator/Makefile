include ../Makefile.common

SRC := cpu_decode.c \
       cpu_execute.c \
       cpu_csr.c \
       cpu_int.c \
       device_clint.c \
       device_plic.c \
       device_uart8250.c \
       device_virtio.c \
       emulator.c \
       isa.c \
       main.c \
       mmu_paging_guest_to_guest.c \
       mmu_paging_guest_to_host.c \

SRC_A :=

ifdef SDL
	SRC += emulator_sdl.c
	CFLAGS += -DRISCV_EMULATOR_SDL_SUPPORT
	LDFLAGS += -lSDL2
endif

DYNAREC_CODEGEN_DIR  := dynarec_x86_64_codegen
DYNAREC_CODEGEN_FILE := dynarec_x86_64_codegen.c
DYNAREC_CODEGEN_SRC  := $(DYNAREC_CODEGEN_DIR)/main.c \
                        $(DYNAREC_CODEGEN_DIR)/assembler.c \
                        $(DYNAREC_CODEGEN_DIR)/codegen.c \
                        $(DYNAREC_CODEGEN_DIR)/x86_isa.c
DYNAREC_CODEGEN_OBJ  := $(DYNAREC_CODEGEN_SRC:%.c=%.o)
DYNAREC_CODEGEN_OUT  := $(DYNAREC_CODEGEN_DIR)/codegen

ifdef DYNAREC_X86_64
	SRC += dynarec_x86_64.c \
	       $(DYNAREC_CODEGEN_FILE)
	SRC_A += dynarec_x86_64_entry_exit.s
	CFLAGS += -DRISCV_EMULATOR_DYNAREC_X86_64_SUPPORT
endif

OBJ := $(SRC:%.c=%.o)
OBJ_A := $(SRC_A:%.s=%.o)
OUT := ../riscv-emulator

.PHONY: all
all: $(OUT)

$(OUT): $(OBJ) $(OBJ_A)
	$(CC) $^ -o $@ $(LDFLAGS)

$(OBJ_A): %.o: %.s
	$(CC) $(AFLAGS) -c $< -o $@

$(OBJ): %.o: %.c
	$(CC) -MMD -MT $@ $(CFLAGS) -c $< -o $@

$(DYNAREC_CODEGEN_FILE): $(DYNAREC_CODEGEN_OUT)
	(./$< > $@) || (rm $@ && exit 1)

$(DYNAREC_CODEGEN_OUT): $(DYNAREC_CODEGEN_OBJ)
	$(HOST_CC) $^ -o $@ $(HOST_LDFLAGS)

$(DYNAREC_CODEGEN_OBJ): %.o: %.c
	$(HOST_CC) -MMD -MT $@ $(HOST_CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	-rm -v *.o *.d
	-rm -v $(DYNAREC_CODEGEN_DIR)/*.o $(DYNAREC_CODEGEN_DIR)/*.d
	-rm -v $(DYNAREC_CODEGEN_FILE) $(DYNAREC_CODEGEN_OUT)

-include $(OBJ:%.o=%.d)
-include $(DYNAREC_CODEGEN_OBJ:%.o=%.d)
