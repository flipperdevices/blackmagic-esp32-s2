/**
 * @file network-http.h
 * @author Sergey Gavrilov (who.just.the.doctor@gmail.com)
 * @version 1.0
 * @date 2021-11-21
 * 
 * HTTP server API
 */
#pragma once

/**
 * Start HTTP server
 */
void network_http_server_init(void);

void network_http_uart_write_data(uint8_t* data, size_t size);