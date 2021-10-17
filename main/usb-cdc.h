#pragma once

void usb_cdc_init(void);

size_t usb_cdc_rx(uint8_t* buffer, size_t size);
size_t usb_cdc_rx_with_timeout(uint8_t* buffer, size_t size, uint32_t timeout);
void usb_cdc_tx(uint8_t* buffer, size_t size, bool flush);