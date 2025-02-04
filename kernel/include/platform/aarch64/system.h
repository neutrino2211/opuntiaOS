/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_AARCH64_SYSTEM_H
#define _KERNEL_PLATFORM_AARCH64_SYSTEM_H

#include <libkern/c_attrs.h>
#include <libkern/types.h>
#include <platform/generic/registers.h>

/**
 * INTS
 */

void system_disable_interrupts();
void system_enable_interrupts();
void system_enable_interrupts_only_counter();

inline static void system_instruction_barrier()
{
    asm volatile("isb");
}

inline static void system_data_synchronise_barrier()
{
    asm volatile("dsb sy");
}

inline static void system_data_memory_barrier()
{
    asm volatile("dmb sy");
}

inline static void system_disable_interrupts_no_counter()
{
    asm volatile("msr daifset, #0xf");
    system_instruction_barrier();
}

inline static void system_enable_interrupts_no_counter()
{
    asm volatile("msr daifclr, #0xf");
    system_instruction_barrier();
}

/**
 * PAGING
 */

extern void system_set_pdir(uintptr_t pdir0, uintptr_t pdir1);

inline static void system_flush_local_tlb_entry(uintptr_t vaddr)
{
    asm volatile("isb");
    asm volatile("tlbi vmalle1\n");
    asm volatile("dsb sy");
}

inline static void system_flush_all_cpus_tlb_entry(uintptr_t vaddr)
{
    asm volatile("isb");
    asm volatile("tlbi vmalle1\n");
    asm volatile("dsb sy");
}

inline static void system_flush_whole_tlb()
{
    asm volatile("isb");
    asm volatile("tlbi vmalle1\n");
    asm volatile("dsb sy");
}

inline static void system_enable_write_protect()
{
}

inline static void system_disable_write_protect()
{
}

inline static void system_enable_paging()
{
}

inline static void system_disable_paging()
{
}

inline static void system_stop_until_interrupt()
{
    asm volatile("wfi");
}

NORETURN inline static void system_stop()
{
    system_disable_interrupts();
    system_stop_until_interrupt();
    while (1) { }
}

void system_cache_invalidate(void* addr, size_t size);
void system_cache_clean_and_invalidate(void* addr, size_t size);
void system_cache_clean(void* addr, size_t size);

/**
 * CPU
 */

inline static int system_cpu_id()
{
    return 0;
}

#endif /* _KERNEL_PLATFORM_AARCH64_SYSTEM_H */