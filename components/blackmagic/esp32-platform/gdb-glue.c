#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <esp_log.h>

size_t usb_cdc_rx(uint8_t* buffer, size_t size);
size_t usb_cdc_rx_with_timeout(uint8_t* buffer, size_t size, uint32_t timeout);
void usb_cdc_tx(uint8_t* buffer, size_t size, bool flush);

unsigned char gdb_if_getchar_to(int timeout) {
    const uint8_t buffer_size = 1;
    uint8_t buffer[buffer_size];

    size_t cnt = usb_cdc_rx_with_timeout(buffer, buffer_size, timeout);
    if(cnt) {
        return buffer[buffer_size - 1];
    } else {
        return -1;
    }
}

unsigned char gdb_if_getchar(void) {
    const uint8_t buffer_size = 1;
    uint8_t buffer[buffer_size];

    size_t cnt = usb_cdc_rx(buffer, buffer_size);
    if(cnt) {
        return buffer[buffer_size - 1];
    } else {
        return -1;
    }
}

#define BUFFER_TX_SIZE 64
static uint8_t buffer_tx[BUFFER_TX_SIZE + 1];
static size_t buffer_tx_counter = 0;

void gdb_if_putchar(unsigned char c, int flush) {
    // usb_cdc_tx(&c, 1, flush);

    buffer_tx[buffer_tx_counter] = c;
    buffer_tx_counter += 1;

    if(flush || (buffer_tx_counter == BUFFER_TX_SIZE)) {
        usb_cdc_tx(buffer_tx, buffer_tx_counter, flush);

        // buffer_tx[buffer_tx_counter] = '\0';
        // ESP_LOGW("gdb_if", "tx> %s", buffer_tx);

        /*if(flush && (buffer_tx_counter == BUFFER_TX_SIZE)) {
            uint8_t zero_byte = 0;
            usb_cdc_tx(&zero_byte, 1, 1);
            // ESP_LOGW("gdb_if", "tx> [zb]");
        }*/

        buffer_tx_counter = 0;
    }
}