/**
 * @file soft-uart.h
 * @author Sergey Gavrilov (who.just.the.doctor@gmail.com)
 * @version 1.0
 * @date 2021-12-18
 * 
 * 
 */
#pragma once
#include <stdint.h>

typedef struct SoftUart SoftUart;

SoftUart* soft_uart_init(uint32_t baudrate, uint8_t tx_pin);

void soft_uart_transmit(SoftUart* uart, const uint8_t* data, uint32_t data_size);
