include ../Makefile.common

SRC := assembler.c \
       diag.c \
       isa.c \
       lexer.c \
       main.c \

OBJ := $(SRC:%.c=%.o)
OUT := ../riscv-assembler

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
