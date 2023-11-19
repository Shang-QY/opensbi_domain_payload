# test_context_switch

This repo includes a simple test application to test the new opensbi domain context switch feature. It can be compiled and generate two domain payload: s-hello.bin and ns-hello.bin, corresponding to secure domain and non-secure domain.

The application behaves as follows: Upon platform boot-up, s-hello is initially executed by the context manager. It then yields control to proceed with the boot-up process. When ns-hello runs, it continuously requests the context manager through an ecall interface to synchronize entry into the secure domain's s-hello logic. In the secure domain, s-hello simply places a message and returns control back to ns-hello.

Here is the build and run commands for reproducing.

### Build QEMU
```
export WORKDIR=`pwd`
git clone https://github.com/qemu/qemu.git -b v8.1.2
cd qemu
./configure --target-list=riscv64-softmmu
make -j $(nproc)
```

### Build OpenSBI
```
cd $WORKDIR
git clone https://github.com/Penglai-Enclave/opensbi.git -b dev-context-management
cd opensbi
CROSS_COMPILE=riscv64-linux-gnu- make PLATFORM=generic
```

### Build test domain payload
```
cd $WORKDIR
git clone https://github.com/Shang-QY/test_context_switch.git
cd test_context_switch
make clean
make
```

### Build device tree
```
cd $WORKDIR/test_context_switch
sudo apt-get install device-tree-compiler
dtc -I dts -O dtb -o ../qemu-virt-new.dtb qemu-virt.dts

[Or if you want to customize the qemu-virt-new.dtb by yourself]
cd $WORKDIR
./qemu/build/qemu-system-riscv64 -nographic -machine virt,dumpdtb=qemu-virt.dtb -bios ./opensbi/build/platform/generic/firmware/fw_jump.bin
sudo apt-get install device-tree-compiler
dtc -I dtb -O dts -o qemu-virt.dts qemu-virt.dtb
vim qemu-virt.dts  <== Modify this file 
dtc -I dts -O dtb -o qemu-virt-new.dtb qemu-virt.dts
```

### Run test application
```
cd $WORKDIR
./qemu/build/qemu-system-riscv64 -nographic -machine virt -bios ./opensbi/build/platform/generic/firmware/fw_jump.bin -dtb ./qemu-virt-new.dtb -kernel test_context_switch/build/ns-hello/ns-hello.bin -device loader,file=test_context_switch/build/s-hello/s-hello.bin,addr=0x80C00000
```
