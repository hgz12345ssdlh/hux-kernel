/**
 * The Hux kernel entry point.
 */


/** Check correct cross compiling. */
#if !defined(__i386__)
#error "The Hux kernel needs to be compiled with an 'ix86-elf' compiler"
#endif


#include "boot/multiboot.h"
#include "boot/elf.h"

#include "common/printf.h"
#include "common/string.h"
#include "common/debug.h"

#include "display/terminal.h"
#include "display/vga.h"

#include "interrupt/idt.h"

#include "memory/gdt.h"
#include "memory/paging.h"
#include "memory/kheap.h"

#include "device/timer.h"
#include "device/keyboard.h"


/**
 * Enabling interrupts by executing the `sti` instruction.
 * This should be called after all devices have been initialized, so that
 * the CPU starts taking in interrupts.
 */
inline void
_enable_interrupts()
{
    asm volatile ( "sti" );
}

/** Disables interrupts by executing the `cli` instruction. */
inline void
_disable_interrupts()
{
    asm volatile ( "cli" );
}


/** Displaying initialization progress message. */
static inline void
_init_message(char *msg)
{
    printf("[");
    cprintf(VGA_COLOR_BLUE, "INIT");
    printf("] %s...", msg);
}

static inline void
_init_message_ok(void)
{
    cprintf(VGA_COLOR_GREEN, " OK\n");
}


/** The main function that `boot.s` jumps to. */
void
kernel_main(unsigned long magic, unsigned long addr)
{
    /** Initialize VGA text-mode terminal support. */
    terminal_init();

    /** Double check the multiboot magic number. */
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        error("invalid bootloader magic: %#x", magic);
        return;
    }

    /** Get pointer to multiboot info. */
    multiboot_info_t *mbi = (multiboot_info_t *) addr;

    /** Initialize debugging utilities. */
    _init_message("initializing debugging utilities");
    debug_init(mbi);
    _init_message_ok();

    /** Initialize global descriptor table (GDT). */
    _init_message("setting up global descriptor table (GDT)");
    gdt_init();
    _init_message_ok();

    /** Initialize interrupt descriptor table (IDT). */
    _init_message("setting up interrupt descriptor table (IDT)");
    idt_init();
    _init_message_ok();

    /** Initialize PIT timer at 100 Hz frequency. */
    _init_message("kicking off the PIT timer & interrupts");
    uint16_t timer_freq_hz = 100;
    timer_init(timer_freq_hz);
    _init_message_ok();
    info("timer frequency set to %dHz", timer_freq_hz);

    /** Initialize PS/2 keyboard support. */
    _init_message("initializing PS/2 keybaord support");
    keyboard_init();
    _init_message_ok();

    /** Initialize paging and switch to use paging. */
    _init_message("setting up virtual memory using paging");
    paging_init();
    _init_message_ok();
    info("supporting physical memory size: %3dMiB", NUM_FRAMES * 4 / 1024);
    info("reserving memory for the kernel: %3dMiB", KMEM_MAX / 1024 / 1024);

    /** Initialize the kernel heap allocator. */
    _init_message("initializing kernel heap memory allocator");
    kheap_init();
    _init_message_ok();
    info("kernel free heap starts at %#X", kheap_curr);

    /** Executes `sti`, CPU starts taking in interrupts. */
    _enable_interrupts();

    printf("\nKallocing arr1 - 128 bytes...\n");
    char *arr1 = (char *) kalloc(128 * sizeof(char));
    strncpy(arr1, "hello\n", 127);

    printf("\nKallocing arr2 - 23 bytes...\n");
    char *arr2 = (char *) kalloc(23 * sizeof(char));
    strncpy(arr2, "hello\n", 22);

    printf("\nKallocing arr3 - 437 bytes...\n");
    char *arr3 = (char *) kalloc(437 * sizeof(char));
    strncpy(arr3, "hello\n", 436);

    printf("\nKfreeing arr3, should coalesce with the big chunk...\n");
    kfree(arr3);

    printf("\nKfreeing arr1, should have no coalescing...\n");
    kfree(arr1);

    printf("\nKallocing arr4 - 54 bytes, should reuse the first chunk...\n");
    char *arr4 = (char *) kalloc(54 * sizeof(char));
    strncpy(arr4, "hello\n", 53);

    printf("\nKfreeing arr2, should coalesce with both neighbors...\n");
    kfree(arr2);

    printf("\nKallocing arr5 - 3971 bytes...\n");
    char *arr5 = (char *) kalloc(3971 * sizeof(char));
    strncpy(arr5, "hello\n", 3970);

    while (1)   // CPU idles with a `hlt` loop.
        asm volatile ( "hlt" );
}
