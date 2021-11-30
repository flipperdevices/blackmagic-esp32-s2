#pragma once

/**
 * Init usb subsystem
 */
void usb_cdc_init(void);

/**
 * Send data
 * @param buffer data
 * @param size data size
 * @param flush
 */
void usb_cdc_send(uint8_t* buffer, size_t size, bool flush);