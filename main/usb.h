#pragma once

/**
 * Init usb subsystem
 */
void usb_init(void);

void usb_gdb_tx_char(uint8_t c, bool flush);

void usb_uart_tx_char(uint8_t c, bool flush);

bool dap_is_connected(void);