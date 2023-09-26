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
    uint8_t uart_num; /*!< UART index */
    uint32_t baud_rate; /*!< UART baud rate*/
    uart_word_length_t data_bits; /*!< UART byte size*/
    uart_parity_t parity; /*!< UART parity mode*/
    uart_stop_bits_t stop_bits; /*!< UART stop bits*/
    int tx_pin_num; /*!< UART tx pin*/
    int rx_pin_num; /*!< UART rx pin*/
    void* isr_context; /*!< UART isr context*/
    uart_isr rx_isr; /*!< UART isr callback*/
} UartConfig;

/**
 * Init UART driver
 * @param config 
 */
void simple_uart_init(UartConfig* config);

/**
 * Write data to UART
 * @param uart_num 
 * @param data 
 * @param data_size 
 */
void simple_uart_write(uint8_t uart_num, const uint8_t* data, const uint32_t data_size);

/**
 * Check if rx data available
 * @param uart_num 
 * @return true 
 * @return false 
 */
bool simple_uart_available(uint8_t uart_num);

/**
 * Read rx data
 * @param uart_num 
 * @param data 
 * @param data_size 
 * @return uint32_t 
 */
uint32_t simple_uart_read(uint8_t uart_num, uint8_t* data, const uint32_t data_size);

/**
 * Set UART baud rate
 * @param uart_num 
 * @param baud_rate 
 */
void simple_uart_set_baud_rate(uint8_t uart_num, uint32_t baud_rate);

/**
 * Set UART stop bits
 * @param uart_num 
 * @param stop_bits 
 */
void simple_uart_set_stop_bits(uint8_t uart_num, uart_stop_bits_t stop_bits);

/**
 * Set UART parity
 * @param uart_num 
 * @param parity 
 */
void simple_uart_set_parity(uint8_t uart_num, uart_parity_t parity);

/**
 * Set UART data bits
 * @param uart_num 
 * @param data_bits 
 */
void simple_uart_set_data_bits(uint8_t uart_num, uart_word_length_t data_bits);

/**
 * @brief Get the UART baud rate
 * 
 * @param uart_num 
 * @return uint32_t 
 */
uint32_t simple_uart_get_baud_rate(uint8_t uart_num);

/**
 * @brief Get the UART stop bits
 * 
 * @param uart_num 
 * @return uart_stop_bits_t 
 */
uart_stop_bits_t simple_uart_get_stop_bits(uint8_t uart_num);

/**
 * @brief Get the UART parity
 * 
 * @param uart_num 
 * @return uart_parity_t 
 */
uart_parity_t simple_uart_get_parity(uint8_t uart_num);

/**
 * @brief Get the UART data bits
 * 
 * @param uart_num 
 * @return uart_word_length_t 
 */
uart_word_length_t simple_uart_get_data_bits(uint8_t uart_num);