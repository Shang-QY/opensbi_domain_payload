
CC		= riscv64-unknown-elf-gcc
OBJCOPY	= riscv64-unknown-elf-objcopy
CC_FLAGS    = -Os -march=rv64imac -mabi=lp64 -mcmodel=medany -ffunction-sections -fdata-sections
ASM_FLAGS  = $(CC_FLAGS)
LD_FLAGS   = -nostartfiles -nostdlib -nostdinc -static -lgcc -Wl,--nmagic -Wl,--gc-sections 

all: s-hello ns-hello

clean:
	rm -fr build

ns-hello:
	mkdir -p build/ns-hello
	$(CC) $(ASM_FLAGS) -c src/crt.S -o build/ns-hello/crt.o -DSYS_INIT_SP_ADDR=0x80210000
	$(CC) $(CC_FLAGS) -c src/ns-hello.c -o build/ns-hello/hello.o
	$(CC) $(CC_FLAGS) $(LD_FLAGS) -T src/default.lds build/ns-hello/crt.o build/ns-hello/hello.o -o build/ns-hello/ns-hello.elf 
	$(OBJCOPY) -O binary build/ns-hello/ns-hello.elf build/ns-hello/ns-hello.bin

s-hello:
	mkdir -p build/s-hello
	$(CC) $(ASM_FLAGS) -c src/crt.S -o build/s-hello/crt.o -DSYS_INIT_SP_ADDR=0x80C10000
	$(CC) $(CC_FLAGS) -c src/s-hello.c -o build/s-hello/hello.o
	$(CC) $(CC_FLAGS) $(LD_FLAGS) -T src/default.lds build/s-hello/crt.o build/s-hello/hello.o -o build/s-hello/s-hello.elf
	$(OBJCOPY) -O binary build/s-hello/s-hello.elf build/s-hello/s-hello.bin
