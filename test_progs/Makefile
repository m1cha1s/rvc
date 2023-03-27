CC := clang
CFLAGS := -Wl,-Ttext=0x0 -nostdlib --target=riscv64 -march=rv64g -mno-relax

OBJCOPY := llvm-objcopy

add-addi.bin: add-addi.s
	$(CC) $(CFLAGS) -o add-addi $<
	$(OBJCOPY) -O binary add-addi $@
	
.PHONY: clean all

all: add-addi.bin

clean:
	rm -f ./*.bin
	rm -f add-addi
