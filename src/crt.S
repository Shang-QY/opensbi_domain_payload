# See LICENSE for license details.

#define __ASM_STR(x)	x

#if __riscv_xlen == 64
#define __REG_SEL(a, b)		__ASM_STR(a)
#define RISCV_PTR		.dword
#elif __riscv_xlen == 32
#define __REG_SEL(a, b)		__ASM_STR(b)
#define RISCV_PTR		.word
#else
#error "Unexpected __riscv_xlen"
#endif

#define REG_L		__REG_SEL(ld, lw)
#define REG_S		__REG_SEL(sd, sw)

	.section .text.init, "ax", @progbits
	.align 3
	.globl _start
_start:
	/* Pick one hart to run the main boot sequence */
	lla	a3, _hart_lottery
	li	a2, 1
	amoadd.w a3, a2, (a3)
	bnez	a3, _start_hang

	/* Save a0 and a1 */
	lla	a3, _boot_a0
	REG_S	a0, 0(a3)

	.align 3
	.globl _start_warm
_start_warm:
	/* Setup stack */
	li	a3, SYS_INIT_SP_ADDR
	li	a4, 0x2000
    mul	a4, a0, a4
	add	sp, a3, a4

	/* Jump to C main */
	lla	a3, _boot_a0
	REG_L	a1, 0(a3)
	call	main

	/* We don't expect to reach here hence just hang */
	j	_start_hang

	.section .text, "ax", @progbits
	.align 3
	.globl _start_hang
_start_hang:
	wfi
	j	_start_hang

	.section .text, "ax", @progbits
	.align	3
_hart_lottery:
	RISCV_PTR	0
_boot_a0:
	RISCV_PTR	0
