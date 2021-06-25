/**
 * The init program to be embedded.
 *
 * Should run in user mode; for now, trying it in kernel mode directly.
 */


.global start
.type start, @function
start:

    /** Trigger a page fault by accessing unmapped memory address. */
    movl $0x40000000, %eax
    movl (%eax), %ebx

    /** Infinite halt loop trick. */
    cli
halt:
    hlt
    jmp halt


/**
# Initial process execs /init.
# This code runs in user space.

#include "syscall.h"
#include "traps.h"


# exec(init, argv)
.globl start
start:
    pushl $argv
    pushl $init
    pushl $0  // where caller pc would be
    movl $SYS_exec, %eax
    int $T_SYSCALL

# for(;;) exit();
exit:
    movl $SYS_exit, %eax
    int $T_SYSCALL
    jmp exit

# char init[] = "/init\0";
init:
    .string "/init\0"

# char *argv[] = { init, 0 };
.p2align 2
argv:
    .long init
    .long 0
*/
