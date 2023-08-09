/**
 * @file usb-uart.h
 * @author Sergey Gavrilov (who.just.the.doctor@gmail.com)
 * @version 1.0
 * @date 2021-12-17
 * 
 * 
 */
#pragma once

void usb_uart_init();

void usb_uart_write(const uint8_t* data, size_t data_size);

void usb_uart_set_line_state(bool dtr, bool rts);

typedef struct {
    uint32_t bit_rate;
    uint8_t stop_bits;
    uint8_t parity;
    uint8_t data_bits;
} UsbUartConfig;

void usb_uart_set_line_coding(UsbUartConfig config);

UsbUartConfig usb_uart_get_line_coding();