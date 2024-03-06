# test_context_switch -- Bare metal multicore app

This repo includes a simple test application to test the new opensbi domain
context switch feature. It can be compiled and generate two domain payload:
s-hello.bin and ns-hello.bin, corresponding to secure domain and non-secure
domain.

When events happen (secure service requirements, in this test), harts can
switch between non-secure and secure domain. Each domain has hart contexts
as many as platform hart count (2 harts in this test), which can be switched
in (running) or out (suspended) simultaneously.

## Test Explanation

When ns-hello start up, it continuously requests s-hello's service on each
hart. That means s-hello need to be started up on each hart and actively
suspend itself at the entry point of service request hander earlier, then
start up ns-hello on each hart. This requires the cooperation of device tree
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
				next-addr = <0x0 0x80C00000>;
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

s-hello and ns-hello both use sbi_hsm mechanism to boot secondary harts.
When ns-hello running, there is no lock to synchronize the two harts when outputting and requesting services, so there will be overlap in the log printing.

### Expected test bahavior

The application behaves as follows: Upon platform boot-up, s-hello is
initially executed. It then yields control to start up the next domain
ns-hello. When ns-hello runs, it continuously requests the context manager
through an ecall interface to synchronize entry into the secure domain's
s-hello logic. In the secure domain, s-hello simply places a message and
returns control back to ns-hello.

## Test Output

![image](https://github.com/Shang-QY/test_context_switch/assets/55442231/bf18d960-b4c4-490a-8643-7a8dea3ca354)

## Test Setup

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
git clone https://github.com/Penglai-Enclave/opensbi.git -b dev-context-management-v5.3
cd opensbi
CROSS_COMPILE=riscv64-linux-gnu- make PLATFORM=generic
```

### Build test domain payload
```
cd $WORKDIR
cd test_context_switch/bare_metal_test
make clean
make
```

### Build device tree
```
cd $WORKDIR/test_context_switch/bare_metal_test
sudo apt-get install device-tree-compiler
dtc -I dts -O dtb -o ../qemu-virt-new.dtb qemu-virt.dts

[Or if you want to customize the qemu-virt-new.dtb by yourself]
cd $WORKDIR
./qemu/build/qemu-system-riscv64 -nographic -machine virt,dumpdtb=qemu-virt.dtb -smp 2 -bios ./opensbi/build/platform/generic/firmware/fw_jump.bin
sudo apt-get install device-tree-compiler
dtc -I dtb -O dts -o qemu-virt.dts qemu-virt.dtb
vim qemu-virt.dts  <== Modify this file 
dtc -I dts -O dtb -o qemu-virt-new.dtb qemu-virt.dts
```

### Run test application
```
cd $WORKDIR
./qemu/build/qemu-system-riscv64 -nographic -machine virt -smp 2 -bios ./opensbi/build/platform/generic/firmware/fw_jump.bin -dtb ./qemu-virt-new.dtb -kernel test_context_switch/bare_metal_test/build/ns-hello/ns-hello.bin -device loader,file=test_context_switch/bare_metal_test/build/s-hello/s-hello.bin,addr=0x80C00000
```
