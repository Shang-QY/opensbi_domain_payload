CC          = riscv64-unknown-elf-gcc
OBJCOPY     = riscv64-unknown-elf-objcopy
CC_FLAGS    = -Os -march=rv64imac -mabi=lp64 -mcmodel=medany -ffunction-sections -fdata-sections
ASM_FLAGS   = $(CC_FLAGS)
LD_FLAGS    = -nostartfiles -nostdlib -nostdinc -static -lgcc -Wl,--nmagic -Wl,--gc-sections

BUILD_DIR   = build

all: s_hello ns_linux_app_hello

clean:
	rm -fr $(BUILD_DIR)

s_hello: $(BUILD_DIR)/s_hello.bin

$(BUILD_DIR)/s_hello.bin: $(BUILD_DIR)/s_hello.elf
	$(OBJCOPY) -O binary $< $@

$(BUILD_DIR)/s_hello.elf: $(BUILD_DIR)/crt.o $(BUILD_DIR)/hello.o
	$(CC) $(CC_FLAGS) $(LD_FLAGS) -T s_hello/default.lds $^ -o $@

$(BUILD_DIR)/crt.o: s_hello/crt.S
	mkdir -p $(BUILD_DIR)
	$(CC) $(ASM_FLAGS) -c $< -o $@ -DSYS_INIT_SP_ADDR=0xF0C10000

$(BUILD_DIR)/hello.o: s_hello/s_hello.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CC_FLAGS) -c $< -o $@

ns_linux_app_hello: $(BUILD_DIR)/ns_linux_app_hello

$(BUILD_DIR)/ns_linux_app_hello:
	mkdir -p $(BUILD_DIR)
	riscv64-unknown-linux-gnu-gcc -o $@ -static ns_linux_app_hello/ns_linux_app_hello.c -lpthread
