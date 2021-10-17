#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <esp_log.h>

size_t usb_cdc_rx(uint8_t* buffer, size_t size);
size_t usb_cdc_rx_with_timeout(uint8_t* buffer, size_t size, uint32_t timeout);
void usb_cdc_tx_char(uint8_t c, bool flush);

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

void gdb_if_putchar(unsigned char c, int flush) {
    usb_cdc_tx_char(c, flush);
}