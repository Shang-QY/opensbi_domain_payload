#include <linux/mm.h>
#include <linux/file.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/dma-mapping.h>
#include <linux/list.h>
#include <linux/file.h>
#include <asm/sbi.h>
#include <asm/csr.h>

#define SBI_EXT_TEST 0x54455354
#define SBI_EXT_SECURE_ENTER 0x0
#define SBI_EXT_SECURE_EXIT 0x1

#define PENGLAI_IOC_ENTER_SECURE 1

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("ioctl for linux app context switch to secure world.");
MODULE_AUTHOR("Shangqy");
MODULE_VERSION("penglai_linux_ioctl");

static int enclave_mmap(struct file *f, struct vm_area_struct *vma)
{
    return 0;
}

int penglai_enter_secure(struct file *filep, unsigned long args)
{
    struct sbiret ret = {0};
    int retval;

    ret = sbi_ecall(SBI_EXT_TEST, SBI_EXT_SECURE_ENTER, 0, 0, 0, 0, 0, 0);
    if (ret.error)
    {
        printk("KERNEL MODULE: sbi call attest secure linux is failed \n");
    }
    retval = ret.value;
    return retval;
}

long penglai_enclave_ioctl(struct file *filep, unsigned int cmd, unsigned long args)
{
    char ioctl_data[1024];
    int ret;

    switch (cmd)
    {
    case PENGLAI_IOC_ENTER_SECURE:
        ret = penglai_enter_secure(filep, (unsigned long)ioctl_data);
        break;
    default:
        return -EFAULT;
    }

    return ret;
}

static const struct file_operations enclave_ops = {
    .owner = THIS_MODULE,
    .mmap = enclave_mmap,
    .unlocked_ioctl = penglai_enclave_ioctl};

struct miscdevice enclave_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "penglai_linux_dev",
    .fops = &enclave_ops,
    .mode = 0666,
};

int enclave_ioctl_init(void)
{
    int ret;
    printk("penglai_linux_ioctl_init...\n");

    ret = misc_register(&enclave_dev);
    if (ret < 0)
    {
        printk("Enclave_driver: register enclave_dev failed!(ret:%d)\n",
               ret);
        goto deregister_device;
    }

    printk("[Penglai KModule] register penglai_sec_linux_dev succeeded!\n");
    return 0;

deregister_device:
    misc_deregister(&enclave_dev);
    return ret;
}

void enclave_ioctl_exit(void)
{
    printk("penglai_linux_ioctl_exit...\n");

    misc_deregister(&enclave_dev);
    return;
}

module_init(enclave_ioctl_init);
module_exit(enclave_ioctl_exit);
