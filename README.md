# test_context_switch -- Linux multicore app

This repo includes a simple test application to test the new opensbi domain
context switch feature. It can be compiled and generate two domain payload:
s_hello.bin and linux, corresponding to secure domain and non-secure
domain.

When events happen (secure service requirements, in this test), harts can
switch between non-secure and secure domain. Each domain has hart contexts
as many as platform hart count (2 harts in this test), which can be switched
in (running) or out (suspended) simultaneously.

## Test Explanation

When linux start up and run the multi-core app, it continuously requests s_hello's service on each
hart. That means s_hello need to be started up on each hart and actively
suspend itself at the entry point of service request hander earlier, then
start up linux on each hart. This requires the cooperation of device tree
configuration and application logic.

### Device tree configuration

We configure device tree as follows:

```
	chosen {
		...
		opensbi-domains {
			compatible = "opensbi,domain,config";

			tmem: tmem {
				compatible = "opensbi,domain,memregion";
				base = <0x0 0x80C00000>;
				order = <22>;   // 4M
			};

			allmem: allmem {
				compatible = "opensbi,domain,memregion";
				base = <0x0 0x0>;
				order = <64>;
			};

			tdomain: trusted-domain {
				compatible = "opensbi,domain,instance";
				regions = <&allmem 0x3f>;
				possible-harts = <&cpu0>, <&cpu1>;
				next-arg1 = <0x0 0x81F80000>;
				next-addr = <0x0 0xF0C00000>;
				next-mode = <0x1>;
			};

			udomain: untrusted-domain {
				compatible = "opensbi,domain,instance";
				regions = <&tmem 0x0>, <&allmem 0x3f>;
				possible-harts = <&cpu0>, <&cpu1>;
                boot-hart = <&cpu0>;
				next-arg1 = <0x0 0xbfe00000>;
				next-addr = <0x0 0x80200000>;
				next-mode = <0x1>;
			};
		};
	};

	cpus {
        ...
		cpu0: cpu@0 {
			phandle = <0x03>;
			device_type = "cpu";
			reg = <0x00>;
			status = "okay";
			compatible = "riscv";
			opensbi-domain = <&tdomain>;
            ...
		};

		cpu1: cpu@1 {
			phandle = <0x01>;
			device_type = "cpu";
			reg = <0x01>;
			status = "okay";
			compatible = "riscv";
			opensbi-domain = <&tdomain>;
            ...
		};
        ...
    };
```

There are two things need to be take care:
- Assign cpu0 and cpu1 to tdomain, which makes tdomain start up first.
- The declaration of tdomain comes before udomain, which causes hart to start udomain when exiting from tdomain for the first time.

### Application logic

s_hello use sbi_hsm mechanism to boot secondary harts.
When linux multi-core app running, there is no lock to synchronize the two harts when outputting and requesting services, so there will be overlap in the log printing.

### Expected test bahavior

The application behaves as follows: Upon platform boot-up, s_hello is
initially executed. It then yields control to start up the next domain
linux. When linux multi-core app runs, it continuously requests the context manager
through an ecall interface to synchronize entry into the secure domain's
s_hello logic. In the secure domain, s_hello simply places a message and
returns control back to linux multi-core app.

## Test Output

![image](https://github.com/Shang-QY/test_context_switch/assets/55442231/b91221f6-1c17-43bf-a173-30c6da3a4f80)

## Test Setup

Here is the build and run commands for reproducing.

### Download this project
```
export WORKDIR=`pwd`
git clone https://github.com/Shang-QY/test_context_switch.git -b linux_multicore_app
```

### Compile QEMU
```
cd $WORKDIR
git clone https://github.com/yli147/qemu.git -b dev-standalonemm-rpmi
cd qemu
./configure --target-list=riscv64-softmmu
make -j $(nproc)
```

### Compile OpenSBI
```
cd $WORKDIR
git clone https://github.com/Penglai-Enclave/opensbi.git -b dev-context-management-v5.2
cd opensbi
CROSS_COMPILE=riscv64-linux-gnu- make PLATFORM=generic
cp build/platform/generic/firmware/fw_dynamic.elf $WORKDIR
```

### Generate DTB
```
cd $WORKDIR
dtc -I dts -O dtb -o qemu-virt-new.dtb ./test_context_switch/qemu-virt.dts
```

### Compile U-Boot
```
cd $WORKDIR
git clone https://github.com/u-boot/u-boot.git
cd u-boot
git checkout v2023.10
make qemu-riscv64_smode_defconfig CROSS_COMPILE=riscv64-linux-gnu-
make -j$(nproc) CROSS_COMPILE=riscv64-linux-gnu-
cp u-boot.bin $WORKDIR
```

### Compile Linux
```
cd $WORKDIR
git clone https://github.com/yli147/linux.git -b dev-rpxy-optee-v3
cd linux
make ARCH=riscv CROSS_COMPILE=riscv64-linux-gnu- defconfig
make ARCH=riscv CROSS_COMPILE=riscv64-linux-gnu- -j $(nproc)
ls arch/riscv/boot -lSh
```

### Compile Linux driver
```
cd $WORKDIR/test_context_switch/ns_linux_driver
make
```

### Compile Linux test app and Secure domain payload
```
cd $WORKDIR/test_context_switch/
make
```

### Compile Rootfs
```
cd $WORKDIR
git clone https://github.com/buildroot/buildroot.git -b 2023.08.x
cd buildroot
make qemu_riscv64_virt_defconfig
make -j $(nproc)
ls ./output/images/rootfs.ext2
```

### Create Disk Image
```
cd $WORKDIR
dd if=/dev/zero of=disk.img bs=1M count=128
sudo sgdisk -g --clear --set-alignment=1 \
       --new=1:34:-0:    --change-name=1:'rootfs'    --attributes=3:set:2 \
	   disk.img
loopdevice=`sudo losetup --partscan --find --show disk.img`
echo $loopdevice
sudo mkfs.ext4 ${loopdevice}p1
sudo e2label ${loopdevice}p1 rootfs
mkdir -p mnt
sudo mount ${loopdevice}p1 ./mnt
sudo tar vxf buildroot/output/images/rootfs.tar -C ./mnt --strip-components=1
sudo mkdir ./mnt/boot
sudo cp -rf linux/arch/riscv/boot/Image ./mnt/boot
version=`cat linux/include/config/kernel.release`
echo $version

sudo mkdir -p .//mnt/boot/extlinux
cat << EOF | sudo tee .//mnt/boot/extlinux/extlinux.conf
menu title QEMU Boot Options
timeout 100
default kernel-$version

label kernel-$version
        menu label Linux kernel-$version
        kernel /boot/Image
        append root=/dev/vda1 ro earlycon console=ttyS0,115200n8

label recovery-kernel-$version
        menu label Linux kernel-$version (recovery mode)
        kernel /boot/Image
        append root=/dev/vda1 ro earlycon single
EOF

sudo cp ./test_context_switch/build/ns_linux_app_hello ./mnt/root/
sudo cp ./test_context_switch/ns_linux_driver/penglai_linux.ko ./mnt/root/

sudo umount ./mnt
sudo losetup -D ${loopdevice}
```

Run u-boot + linux (Need GUI):
```
cd $WORKDIR
./run-linux.sh
```

Login with `root`(no password). After Login, execute 
```
insmod ns_linux_driver.ko
./ns_linux_app_hello
```
