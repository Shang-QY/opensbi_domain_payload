#define SBI_EXT_0_1_CONSOLE_PUTCHAR		0x1
#define SBI_EXT_TEST   0x54455354
#define SBI_EXT_SECURE_ENTER 0x0
#define SBI_EXT_SECURE_EXIT  0x1

struct sbiret {
  unsigned long error;
  unsigned long value;
};
typedef unsigned long uintptr_t;

static struct sbiret sbi_ecall(int ext, int fid, unsigned long arg0,
                        unsigned long arg1, unsigned long arg2,
                        unsigned long arg3, unsigned long arg4,
                        unsigned long arg5)
{
        struct sbiret ret;

        register uintptr_t a0 asm ("a0") = (uintptr_t)(arg0);
        register uintptr_t a1 asm ("a1") = (uintptr_t)(arg1);
        register uintptr_t a2 asm ("a2") = (uintptr_t)(arg2);
        register uintptr_t a3 asm ("a3") = (uintptr_t)(arg3);
        register uintptr_t a4 asm ("a4") = (uintptr_t)(arg4);
        register uintptr_t a5 asm ("a5") = (uintptr_t)(arg5);
        register uintptr_t a6 asm ("a6") = (uintptr_t)(fid);
        register uintptr_t a7 asm ("a7") = (uintptr_t)(ext);
        asm volatile ("ecall"
                      : "+r" (a0), "+r" (a1)
                      : "r" (a2), "r" (a3), "r" (a4), "r" (a5), "r" (a6), "r" (a7)
                      : "memory");
        ret.error = a0;
        ret.value = a1;

        return ret;
}

void sbi_console_putchar(int ch)
{
        sbi_ecall(SBI_EXT_0_1_CONSOLE_PUTCHAR, 0, ch, 0, 0, 0, 0, 0);
}


void sbi_domain_secure_exit()
{
        sbi_ecall(SBI_EXT_TEST, SBI_EXT_SECURE_EXIT, 0, 0, 0, 0, 0, 0);
}

void main()
{
    const char *s = "Hello Secure World.\n";
    while (1) {
        const char *t = s;
        while (*t) sbi_console_putchar(*t++);
        sbi_domain_secure_exit();
    }
}
