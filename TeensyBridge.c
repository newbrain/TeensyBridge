/*
 * Copyright (c) 2022, Federico Zuccardi Merli.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include "bsp/board.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "tusb.h"
//#include <stdio.h>

#define UART_TX 4
#define UART_RX 5

#define PROGRAM 2
#define NMI     3

#define DELAY 50 /* ms */

int main(void)
{
    stdio_init_all();

    /* Initialize PROGRAM as Open Drain like */
    gpio_set_function(PROGRAM, GPIO_FUNC_SIO);        /* SIO for SW controlled GPIO */
    gpio_set_dir(PROGRAM, GPIO_IN);                   /* Disable output */
    gpio_set_pulls(PROGRAM, true, false);             /* Pull Up */
    gpio_put(PROGRAM, false);                         /* Low when active */
    gpio_set_slew_rate(PROGRAM, GPIO_SLEW_RATE_SLOW); /* No hurry */

    /* Initialize PROGRAM as Open Source like */
    gpio_set_function(NMI, GPIO_FUNC_SIO);        /* SIO for SW controlled GPIO */
    gpio_set_dir(NMI, GPIO_IN);                   /* Disable output */
    gpio_set_pulls(NMI, false, true);             /* Pull Down */
    gpio_put(NMI, true);                          /* High when active */
    gpio_set_slew_rate(NMI, GPIO_SLEW_RATE_SLOW); /* No hurry */

    /* Initialize USB library */
    board_init();
    tusb_init();

    uart_init(uart1, 115200);

    gpio_set_function(UART_RX, GPIO_FUNC_UART);
    gpio_set_function(UART_TX, GPIO_FUNC_UART);

    alarm_pool_init_default();

    for (;;)
    {
        tud_task();
        uint32_t count = 0;
        while (uart_is_writable(uart1) && tud_cdc_available())
        {
            char c = tud_cdc_read_char();
            uart_putc_raw(uart1, c);
        }
        while (uart_is_readable(uart1) && tud_cdc_write_available())
        {
            char c = uart_getc(uart1);
            tud_cdc_write_char(c);
        }
        tud_cdc_write_flush();
    }
    return 0;
}

int64_t pin_callback(alarm_id_t id, void *pin_vp)
{
    (void)id;
    uintptr_t pin = (uintptr_t)pin_vp;
    gpio_set_dir(pin, GPIO_IN);
    board_led_off();
    return 0;
}

void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const *line_options)
{
    (void)itf;
    uint32_t pin;
    if (line_options->bit_rate == 301)
    {
        gpio_set_dir(PROGRAM, GPIO_OUT);
        board_led_on();
        add_alarm_in_ms(DELAY, pin_callback, (void *)PROGRAM, false);
    }
    else if (line_options->bit_rate == 302)
    {
        gpio_set_dir(NMI, GPIO_OUT);
        board_led_on();
        add_alarm_in_ms(DELAY, pin_callback, (void *)NMI, false);
    }
    else
    {
        uart_set_baudrate(uart1, line_options->bit_rate);
    }
    /* Set the other line parameters NO ERROR CHECKING! */
    uart_parity_t p = line_options->parity;
    if (p > 2)
        p = 0;
    else if (p)
        p = 2 - p;
    uint32_t bits = line_options->data_bits == 16 ? 8 : line_options->data_bits;
    uart_set_format(uart1,
                    bits,
                    1 + (line_options->stop_bits >> 1),
                    p);
}