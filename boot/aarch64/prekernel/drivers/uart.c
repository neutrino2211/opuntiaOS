/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "uart.h"

volatile uint32_t* output = NULL;

void uart_init()
{
    // devtree_entry_t* dev = devtree_find_device("uart");
    // if (!dev) {
    //     while (1) { };
    // }
    output = (uint32_t*)(0x09000000);
}

int uart_write(uint8_t data)
{
    if (!output) {
        return 1;
    }

    *output = data;
    return 0;
}