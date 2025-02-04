/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/driver_manager.h>
#include <libkern/log.h>
#include <libkern/types.h>
#include <platform/aarch64/interrupts.h>
#include <platform/aarch64/registers.h>
#include <tasking/sched.h>

static void aarch64_timer_write_reg(uint64_t val)
{
    asm volatile("msr cntp_ctl_el0, %x0"
                 :
                 : "r"(val)
                 : "memory");
    system_instruction_barrier();
}

static void aarch64_timer_write_ctrl(uint64_t val)
{
    asm volatile("msr cntp_tval_el0, %x0"
                 :
                 : "r"(val)
                 : "memory");
    system_instruction_barrier();
}

void aarch64_timer_enable()
{
    aarch64_timer_write_reg(1);
}

void aarch64_timer_disable()
{
    aarch64_timer_write_reg(0b10);
}

void aarch64_timer_rearm()
{
    uint64_t el;
    asm volatile("mrs %x0, CNTFRQ_EL0"
                 : "=r"(el)
                 :);

    aarch64_timer_write_ctrl(el / 125);
}

void tick()
{
    aarch64_timer_rearm();
    cpu_tick();
    timeman_timer_tick();
    sched_tick();
}

int aarch64_timer_init(device_t* dev)
{
    if (dev->device_desc.type != DEVICE_DESC_DEVTREE) {
        return -1;
    }

    devtree_entry_t* devtree_entry = dev->device_desc.devtree.entry;
    if (devtree_entry->irq_lane > 0) {
        irq_flags_t irqflags = irq_flags_from_devtree(devtree_entry->irq_flags);
        irq_register_handler(devtree_entry->irq_lane, devtree_entry->irq_priority, irqflags, tick, ALL_CPU_MASK);
    }

    aarch64_timer_disable();
    aarch64_timer_write_ctrl(0xfffffff);
    aarch64_timer_enable();
    aarch64_timer_rearm();
    return 0;
}

static driver_desc_t _aarch64_timer_keyboard_driver_info()
{
    driver_desc_t ms_desc = { 0 };
    ms_desc.type = DRIVER_TIMER;
    ms_desc.system_funcs.init_with_dev = aarch64_timer_init;
    ms_desc.system_funcs.recieve_notification = NULL;
    return ms_desc;
}

void aarch64_timer_install()
{
    devman_register_driver(_aarch64_timer_keyboard_driver_info(), "aa64timer");
}

devman_register_driver_installation(aarch64_timer_install);