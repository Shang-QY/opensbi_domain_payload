##
## Author: Qingyu
## Description:
## 	The file now assumes the kernel located in ../../linux,
##	if you would like to use your own one, please change the path

obj-m += ns_linux_driver.o

all:
	make -C ../../linux/ CROSS_COMPILE=riscv64-linux-gnu- ARCH=riscv M=$(PWD) modules

clean:
	make -C ../../linux/ ARCH=riscv M=$(PWD) clean
