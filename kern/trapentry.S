/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>

#include <kern/picirq.h>


###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps

.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
*/

TRAPHANDLER_NOEC(DIVIDE,0)	
TRAPHANDLER_NOEC(DEBUG,1)	
TRAPHANDLER_NOEC(NMI,2)	
TRAPHANDLER_NOEC(BRKPT,3)	
TRAPHANDLER_NOEC(OFLOW,4)	
TRAPHANDLER_NOEC(BOUND,5)	
TRAPHANDLER_NOEC(ILLOP,6)	
TRAPHANDLER_NOEC(DEVICE,7)	
TRAPHANDLER(DBLFLT,8)	
//TRAPHANDLER(,9)	
TRAPHANDLER(TSS,10)	
TRAPHANDLER(SEGNP,11)	
TRAPHANDLER(STACK,12)	
TRAPHANDLER(GPFLT,13)	
TRAPHANDLER(PGFLT,14)	
//TRAPHANDLER(,15)	
TRAPHANDLER_NOEC(FPERR,16)	
TRAPHANDLER_NOEC(ALIGN,17)	
TRAPHANDLER_NOEC(MCHK,18)	
TRAPHANDLER_NOEC(SIMDERR,19)	

TRAPHANDLER_NOEC(SYSCALL, 48)

TRAPHANDLER_NOEC(TIMER, 32)
TRAPHANDLER_NOEC(KBD, 33)
TRAPHANDLER_NOEC(IRQ2, 34)	
TRAPHANDLER_NOEC(IRQ3, 35)

TRAPHANDLER_NOEC(SERIAL, 36)
TRAPHANDLER_NOEC(IRQ5, 37)
TRAPHANDLER_NOEC(IRQ6, 38)

TRAPHANDLER_NOEC(SPURIOUS, 39)
TRAPHANDLER_NOEC(IRQ8, 40)
TRAPHANDLER_NOEC(IRQ9, 41)
TRAPHANDLER_NOEC(IRQ10, 42)
TRAPHANDLER_NOEC(IRQ11, 43)
TRAPHANDLER_NOEC(IRQ12, 44)
TRAPHANDLER_NOEC(IRQ13, 45)

TRAPHANDLER_NOEC(IDE, 46)
TRAPHANDLER_NOEC(IRQ15, 47)	
TRAPHANDLER_NOEC(ERROR, IRQ_OFFSET + IRQ_ERROR)

TRAPHANDLER_NOEC(DEFAULT, T_DEFAULT)
	/*
 * Lab 3: Your code here for _alltraps
	*/
_alltraps:
	pushl %ds
	pushl %es
	pushal
	pushl $GD_KD
	popl %ds
	pushl $GD_KD
	popl %es
	pushl %esp
	call trap
	addl $4, %esp


