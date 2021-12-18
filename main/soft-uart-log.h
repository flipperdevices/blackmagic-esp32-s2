/**
 * @file soft-uart-log.h
 * @author Sergey Gavrilov (who.just.the.doctor@gmail.com)
 * @version 1.0
 * @date 2021-12-18
 * 
 * 
 */
#pragma once
#include <stdint.h>

void soft_uart_log_init(uint8_t pin, uint32_t baudrate);