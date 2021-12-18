#pragma once

/**
 * Init usb subsystem
 */
void usb_cdc_init(void);

void usb_cdc_gdb_tx_char(uint8_t c, bool flush);

void usb_cdc_uart_tx_char(uint8_t c, bool flush);