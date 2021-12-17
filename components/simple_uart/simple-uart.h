/**
 * @file uart.h
 * @author Sergey Gavrilov (who.just.the.doctor@gmail.com)
 * @version 1.0
 * @date 2021-12-17
 * 
 * 
 */
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <hal/uart_types.h>
#include <soc/soc_caps.h>

#define UART_NUM_0 (0) /*!< UART port 0 */
#define UART_NUM_1 (1) /*!< UART port 1 */
#define UART_NUM_MAX (SOC_UART_NUM)

typedef void (*uart_isr)(void* context);

typedef struct {
    uint8_t uart_num;
    int baud_rate; /*!< UART baud rate*/
    uart_word_length_t data_bits; /*!< UART byte size*/
    uart_parity_t parity; /*!< UART parity mode*/
    uart_stop_bits_t stop_bits; /*!< UART stop bits*/
    int tx_pin_num;
    int rx_pin_num;
    void* isr_context;
    uart_isr rx_isr;
} UartConfig;

void simple_uart_init(UartConfig* config);

void simple_uart_write(uint8_t uart_num, const uint8_t* data, const uint32_t data_size);

bool simple_uart_available(uint8_t uart_num);

uint32_t simple_uart_read(uint8_t uart_num, uint8_t* data, const uint32_t data_size);