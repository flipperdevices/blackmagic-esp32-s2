#include "soft-uart.h"
#include <string.h>
#include <freertos/portmacro.h>
#include <esp32/clk.h>
#include <driver/gpio.h>

struct SoftUart {
    uint32_t baudrate;
    uint32_t bit_time;
    uint8_t tx_pin;
    bool invert;
};

#define wait_cycles(cycles) \
    for(uint32_t start = cycle_count_get(); cycle_count_get() - start < cycles;)

static inline uint32_t __attribute__((always_inline)) cycle_count_get() {
    uint32_t ccount;
    __asm__ __volatile__("esync; rsr %0,ccount" : "=a"(ccount));
    return ccount;
}

void soft_uart_transmit_byte(SoftUart* uart, uint8_t byte) {
    if(uart->invert) {
        byte = ~byte;
    }

    // disable interrupts
    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    portENTER_CRITICAL(&mux);

    // create tx interrupts to start bit.
    gpio_set_level(uart->tx_pin, 1), gpio_set_level(uart->tx_pin, 0);
    wait_cycles(uart->bit_time);

    for(uint8_t i = 0; i != 8; i++) {
        gpio_set_level(uart->tx_pin, (byte & 1) ? 1 : 0);
        wait_cycles(uart->bit_time);
        byte >>= 1;
    }

    // Stop bit
    gpio_set_level(uart->tx_pin, 1);
    wait_cycles(uart->bit_time);

    // re-enable interrupts
    portEXIT_CRITICAL(&mux);
}

SoftUart* soft_uart_init(uint32_t baudrate, uint8_t tx_pin) {
    SoftUart* uart = malloc(sizeof(SoftUart));

    uart->baudrate = baudrate;
    uart->tx_pin = tx_pin;
    uart->invert = false;

    uart->bit_time = (esp_clk_cpu_freq() / uart->baudrate);

    gpio_pad_select_gpio(uart->tx_pin);
    gpio_set_direction(uart->tx_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(uart->tx_pin, !uart->invert);
    return uart;
}

void soft_uart_transmit(SoftUart* uart, const uint8_t* data, uint32_t data_size) {
    for(size_t i = 0; i < data_size; i++) {
        soft_uart_transmit_byte(uart, data[i]);
    }
}