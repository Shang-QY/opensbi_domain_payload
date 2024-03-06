CC          = riscv64-unknown-elf-gcc
OBJCOPY     = riscv64-unknown-elf-objcopy
CC_FLAGS    = -Os -march=rv64imac -mabi=lp64 -mcmodel=medany -ffunction-sections -fdata-sections
ASM_FLAGS   = $(CC_FLAGS)
LD_FLAGS    = -nostartfiles -nostdlib -nostdinc -static -lgcc -Wl,--nmagic -Wl,--gc-sections

BUILD_DIR   = build
NS_HELLO_DIR= $(BUILD_DIR)/ns-hello
S_HELLO_DIR = $(BUILD_DIR)/s-hello

all: s-hello ns-hello

clean:
	rm -fr $(BUILD_DIR)

ns-hello: $(NS_HELLO_DIR)/ns-hello.bin

$(NS_HELLO_DIR)/ns-hello.bin: $(NS_HELLO_DIR)/ns-hello.elf
	$(OBJCOPY) -O binary $< $@

$(NS_HELLO_DIR)/ns-hello.elf: $(NS_HELLO_DIR)/crt.o $(NS_HELLO_DIR)/hello.o
	$(CC) $(CC_FLAGS) $(LD_FLAGS) -T src/default.lds $^ -o $@

$(NS_HELLO_DIR)/crt.o: src/crt.S
	mkdir -p $(NS_HELLO_DIR)
	$(CC) $(ASM_FLAGS) -c $< -o $@ -DSYS_INIT_SP_ADDR=0x80210000

$(NS_HELLO_DIR)/hello.o: src/ns-hello.c
	mkdir -p $(NS_HELLO_DIR)
	$(CC) $(CC_FLAGS) -c $< -o $@

s-hello: $(S_HELLO_DIR)/s-hello.bin

$(S_HELLO_DIR)/s-hello.bin: $(S_HELLO_DIR)/s-hello.elf
	$(OBJCOPY) -O binary $< $@

$(S_HELLO_DIR)/s-hello.elf: $(S_HELLO_DIR)/crt.o $(S_HELLO_DIR)/hello.o
	$(CC) $(CC_FLAGS) $(LD_FLAGS) -T src/default.lds $^ -o $@

$(S_HELLO_DIR)/crt.o: src/crt.S
	mkdir -p $(S_HELLO_DIR)
	$(CC) $(ASM_FLAGS) -c $< -o $@ -DSYS_INIT_SP_ADDR=0x80C10000

$(S_HELLO_DIR)/hello.o: src/s-hello.c
	mkdir -p $(S_HELLO_DIR)
	$(CC) $(CC_FLAGS) -c $< -o $@
