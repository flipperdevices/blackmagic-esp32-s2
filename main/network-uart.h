#pragma once
#include <stdint.h>

/**
 * Start uart server
 */
void network_uart_server_init(void);

/**
 * Checks if someone is connected to the uart port
 * @return bool
 */
bool network_uart_connected(void);

/**
 * Send data
 * @param buffer data
 * @param size data size
 */
void network_uart_send(uint8_t* buffer, size_t size);