#include <esp_log.h>
#include <soft-uart.h>
#include <freertos/FreeRTOS.h>
#include <stdio.h>
#include <string.h>

extern void esp_log_impl_lock(void);
extern void esp_log_impl_unlock(void);

#define LOG_BUFFER_SIZE (256)
static SoftUart* log_uart = NULL;
static char log_buffer[LOG_BUFFER_SIZE];

static int soft_uart_log_vprintf(const char* str, va_list l) {
    esp_log_impl_lock();

    int len = vsnprintf(log_buffer, LOG_BUFFER_SIZE, str, l);
    soft_uart_transmit(log_uart, (uint8_t*)log_buffer, strlen(log_buffer));

    esp_log_impl_unlock();
    return len;
}

void soft_uart_log_init(uint8_t pin, uint32_t baudrate) {
    log_uart = soft_uart_init(baudrate, pin);

    esp_log_set_vprintf(soft_uart_log_vprintf);
}