#pragma once

typedef enum {
    USBDeviceTypeDapLink,
    USBDeviceTypeDualCDC,
} USBDeviceType;

void usb_glue_init(USBDeviceType device_type);