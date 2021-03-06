/**
 * User program system calls library.
 */


#ifndef SYSCALL_H
#define SYSCALL_H


#include <stdint.h>

#include "printf.h"


/** Externed from ASM `syscall.s`. */
extern int8_t getpid();
extern int8_t fork();
extern void exit(void);
extern int8_t sleep(int32_t millisecs);
extern int8_t wait(void);
extern int8_t kill(int8_t pid);
extern void tprint(vga_color_t color, char *str);


#endif
