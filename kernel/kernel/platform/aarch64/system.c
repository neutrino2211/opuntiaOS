/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libkern/libkern.h>
#include <platform/aarch64/system.h>
#include <tasking/cpu.h>

bool system_can_preempt_kernel()
{
    return 0;
}

void system_disable_interrupts()
{
    THIS_CPU->int_depth_counter++;
    system_disable_interrupts_no_counter();
}

void system_enable_interrupts()
{
    THIS_CPU->int_depth_counter--;
    ASSERT(THIS_CPU->int_depth_counter >= 0);
    if (THIS_CPU->int_depth_counter == 0) {
        system_enable_interrupts_no_counter();
    }
}

void system_enable_interrupts_only_counter()
{
    THIS_CPU->int_depth_counter--;
    ASSERT(THIS_CPU->int_depth_counter >= 0);
}

void system_cache_invalidate(void* addr, size_t size)
{
    const size_t cache_line_size = 64;
    size_t start = ROUND_FLOOR((size_t)addr, cache_line_size);
    size_t end = ROUND_CEIL((size_t)addr + size, cache_line_size);

    asm volatile("isb");
    asm volatile("dsb sy");
    for (size_t curaddr = start; curaddr < end; curaddr += cache_line_size) {
        asm volatile("dc ivac, %0"
                     :
                     : "r"(curaddr));
    }
    asm volatile("dsb sy");
    asm volatile("isb");
}

void system_cache_clean_and_invalidate(void* addr, size_t size)
{
    const size_t cache_line_size = 64;
    size_t start = ROUND_FLOOR((size_t)addr, cache_line_size);
    size_t end = ROUND_CEIL((size_t)addr + size, cache_line_size);

    asm volatile("isb");
    asm volatile("dsb sy");
    for (size_t curaddr = start; curaddr < end; curaddr += cache_line_size) {
        asm volatile("dc civac, %0"
                     :
                     : "r"(curaddr));
    }
    asm volatile("dsb sy");
    asm volatile("isb");
}

void system_cache_clean(void* addr, size_t size)
{
    // Cache clean also invalidates cache.
    return system_cache_clean_and_invalidate(addr, size);
}