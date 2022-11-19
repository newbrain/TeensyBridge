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
#include "get_serial.h"
// #include <stdio.h>

#define UART0_TX 0
#define UART0_RX 1
#define UART1_TX 4
#define UART1_RX 5

#define PROGRAM 2
#define NMI     3

#define DELAY 50 /* ms */

int main(void)
{
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
    usb_serial_init();
    tusb_init();

    uart_init(uart0, 115200);
    uart_init(uart1, 115200);

    gpio_set_function(UART0_RX, GPIO_FUNC_UART);
    gpio_set_function(UART0_TX, GPIO_FUNC_UART);
    gpio_set_function(UART1_RX, GPIO_FUNC_UART);
    gpio_set_function(UART1_TX, GPIO_FUNC_UART);

    alarm_pool_init_default();

    for (;;)
    {
        tud_task();
        for (uint8_t itf = 0; itf < 2; itf++)
        {
            uart_inst_t *uart = uart_get_instance(itf);
            while (uart_is_writable(uart) && tud_cdc_n_available(itf))
            {
                char c = tud_cdc_n_read_char(itf);
                uart_putc_raw(uart, c);
            }
            while (uart_is_readable(uart) && tud_cdc_n_write_available(itf))
            {
                char c = uart_getc(uart);
                tud_cdc_n_write_char(itf, c);
            }
            tud_cdc_n_write_flush(itf);
        }
    }
    return 0;
}

static int64_t key_release(alarm_id_t id, void *pin_vp)
{
    (void)id;
    uintptr_t pin = (uintptr_t)pin_vp;
    gpio_set_dir(pin, GPIO_IN);
    board_led_off();
    return 0;
}

static void key_press(uint32_t key)
{
    gpio_set_dir(key, GPIO_OUT);
    board_led_on();
    add_alarm_in_ms(DELAY, key_release, (void *)key, false);
}

void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const *line_options)
{
    uint32_t pin;
    if (itf == 1) /* Only do this for the debug interface */
    {
        if (line_options->bit_rate == 301)
        {
            key_press(PROGRAM);
            return; /* Do nothing for the other options */
        }
        else if (line_options->bit_rate == 302)
        {
            key_press(NMI);
            return; /* Do nothing for the other options */
        }
    }
    uart_inst_t *uart = uart_get_instance(itf);
    uart_set_baudrate(uart, line_options->bit_rate);
    /* Set the other line parameters NO ERROR CHECKING! */
    uart_parity_t p = line_options->parity;
    if (p > 2)
        p = 0;
    else if (p)
        p = 2 - p;
    uint32_t bits = line_options->data_bits == 16 ? 8 : line_options->data_bits;
    uart_set_format(uart,
                    bits,
                    1 + (line_options->stop_bits >> 1),
                    p);
}