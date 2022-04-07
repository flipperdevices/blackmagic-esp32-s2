/**
 * @file network-uart.h
 * @author Sergey Gavrilov (who.just.the.doctor@gmail.com)
 * @version 1.0
 * @date 2021-11-23
 * 
 * Uart server API
 */
#pragma once

/**
 * Start UART server
 */
void network_uart_server_init(void);

/**
 * Checks if someone is connected to the UART server
 * @return bool
 */
bool network_uart_connected(void);

/**
 * Send data
 * @param buffer data
 * @param size data size
 */
void network_uart_send(uint8_t* buffer, size_t size);