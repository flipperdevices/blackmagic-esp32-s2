#pragma once
#include <stdint.h>
#include <stddef.h>
#include <esp_err.h>
#include <tusb.h>

typedef enum {
    USBDeviceTypeDapLink,
    USBDeviceTypeDualCDC,
} USBDeviceType;

/***** Common *****/

esp_err_t usb_glue_init(USBDeviceType device_type);

void usb_glue_reset_bus();

void usb_glue_set_connected_callback(void (*callback)(void* context), void* context);

void usb_glue_set_disconnected_callback(void (*callback)(void* context), void* context);

const char* usb_glue_get_serial_number();

/***** USB-UART *****/

void usb_glue_cdc_send(const uint8_t* buf, size_t len, bool flush);

void usb_glue_cdc_set_receive_callback(void (*callback)(void* context), void* context);

size_t usb_glue_cdc_receive(uint8_t* buf, size_t len);

void usb_glue_cdc_set_line_coding_callback(
    void (*callback)(cdc_line_coding_t const* p_line_coding, void* context),
    void* context);

void usb_glue_cdc_set_line_state_callback(
    void (*callback)(bool dtr, bool rts, void* context),
    void* context);

/***** USB-GDB *****/

void usb_glue_gdb_send(const uint8_t* buf, size_t len, bool flush);

void usb_glue_gdb_set_receive_callback(void (*callback)(void* context), void* context);

size_t usb_glue_gdb_receive(uint8_t* buf, size_t len);

/***** USB-DAP *****/

void usb_glue_dap_send(const uint8_t* buf, size_t len, bool flush);

void usb_glue_dap_set_receive_callback(void (*callback)(void* context), void* context);

size_t usb_glue_dap_receive(uint8_t* buf, size_t len);
