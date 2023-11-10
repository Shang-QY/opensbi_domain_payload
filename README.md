# test_context_switch

This is a simple test application to test the new opensbi domain context switch feature
```
git clone https://github.com/yli147/qemu.git -b dev-standalonemm-rpmi
export WORKDIR=`pwd`
cd qemu
./configure --target-list=riscv64-softmmu
make -j $(nproc)

cd $WORKDIR
git clone https://github.com/Penglai-Enclave/opensbi.git -b context_switch
cd opensbi
CROSS_COMPILE=riscv64-linux-gnu- make PLATFORM=generic

cd $WORKDIR
git clone https://github.com/yli147/test_context_switch.git
cd test_context_switch
make clean
make
sudo apt-get install device-tree-compiler
dtc -I dts -O dtb -o ../qemu-virt-new.dtb qemu-virt.dts

[Or if you want to customize the qemu-virt-new.dtb by yourself]
cd $WORKDIR
./qemu/build/qemu-system-riscv64 -nographic -machine virt,dumpdtb=qemu-virt.dtb -bios ./opensbi/build/platform/generic/firmware/fw_jump.bin
sudo apt-get install device-tree-compiler
dtc -I dtb -O dts -o qemu-virt.dts qemu-virt.dtb
vim qemu-virt.dts  <== Modify this file 
dtc -I dts -O dtb -o qemu-virt-new.dtb qemu-virt.dts

cd $WORKDIR
./qemu/build/qemu-system-riscv64 -nographic -machine virt -bios ./opensbi/build/platform/generic/firmware/fw_jump.bin -dtb ./qemu-virt-new.dtb -kernel test_context_switch/build/ns-hello/ns-hello.elf -device loader,file=test_context_switch/build/s-hello/s-hello.bin,addr=0x80C00000
```
