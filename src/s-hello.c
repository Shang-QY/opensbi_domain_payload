#define SBI_EXT_0_1_CONSOLE_PUTCHAR		0x1
#define SBI_EXT_TEST   0x54455354
#define SBI_EXT_SECURE_ENTER 0x0
#define SBI_EXT_SECURE_EXIT  0x1

#define SBI_EXT_HSM				0x48534D
#define SBI_EXT_HSM_HART_START			0x0

#define PRV_U				0
#define PRV_S				1
#define PRV_M				3

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

static int sbi_hsm_hart_start(unsigned long hartid, unsigned long saddr,
			      unsigned long priv)
{
	struct sbiret ret;

	ret = sbi_ecall(SBI_EXT_HSM, SBI_EXT_HSM_HART_START,
			hartid, saddr, priv, 0, 0, 0);
	if (ret.error)
		return -1;
	else
		return 0;
}

void sbi_domain_secure_exit()
{
        sbi_ecall(SBI_EXT_TEST, SBI_EXT_SECURE_EXIT, 0, 0, 0, 0, 0, 0);
}

int hart_table[] = {0, 1};
extern void _start_warm(void);

void main(int hartid, int cold_boot_hartid)
{
    char *s = "Hart_ : Hello Secure World.\n";

    if (hartid == cold_boot_hartid) {
        for (int i = 0; i < sizeof(hart_table) / sizeof(int); ++i) {
            if (i != hartid)
                sbi_hsm_hart_start(hart_table[i], (unsigned long)_start_warm, PRV_S);
        }
    }

    while (1) {
        const char *t = s;
        /* Need to have lock here... */
        s[5] = '0' + hartid;
        while (*t) sbi_console_putchar(*t++);
        sbi_domain_secure_exit();
    }
}
