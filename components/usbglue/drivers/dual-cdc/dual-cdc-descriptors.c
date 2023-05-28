/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <tusb.h>
#include <class/cdc/cdc_device.h>
#include "tusb_config.h"

#include <esp_log.h>

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t const blackmagic_desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,

    // Use Interface Association Descriptor (IAD) for CDC
    // As required by USB Specs IAD's subclass must be common class (2) and protocol must be IAD (1)
    .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor = 0x303A, // USB_ESPRESSIF_VID,
    .idProduct = 0x4001, // USB_TUSB_PID,
    .bcdDevice = 0x0100,

    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,

    .bNumConfigurations = 0x01,
};

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+
enum { ITF_NUM_CDC_0 = 0, ITF_NUM_CDC_0_DATA, ITF_NUM_CDC_1, ITF_NUM_CDC_1_DATA, ITF_NUM_TOTAL };

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + CFG_TUD_CDC * TUD_CDC_DESC_LEN)

#if CFG_TUSB_MCU == OPT_MCU_LPC175X_6X || CFG_TUSB_MCU == OPT_MCU_LPC177X_8X || \
    CFG_TUSB_MCU == OPT_MCU_LPC40XX
// LPC 17xx and 40xx endpoint type (bulk/interrupt/iso) are fixed by its number
// 0 control, 1 In, 2 Bulk, 3 Iso, 4 In etc ...
#define EPNUM_CDC_0_NOTIF 0x81
#define EPNUM_CDC_0_DATA 0x02

#define EPNUM_CDC_1_NOTIF 0x84
#define EPNUM_CDC_1_DATA 0x05
#else
#define EPNUM_CDC_0_NOTIF 0x81
#define EPNUM_CDC_0_DATA 0x02

#define EPNUM_CDC_1_NOTIF 0x83
#define EPNUM_CDC_1_DATA 0x04
#endif

uint8_t const blackmagic_desc_fs_configuration[] = {
    // Config number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(
        1,
        ITF_NUM_TOTAL,
        0,
        CONFIG_TOTAL_LEN,
        TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP,
        100),

    // 1st CDC: Interface number, string index, EP notification address and size, EP data address (out, in) and size.
    TUD_CDC_DESCRIPTOR(
        ITF_NUM_CDC_0,
        4,
        EPNUM_CDC_0_NOTIF,
        8,
        EPNUM_CDC_0_DATA,
        0x80 | EPNUM_CDC_0_DATA,
        64),

    // 2nd CDC: Interface number, string index, EP notification address and size, EP data address (out, in) and size.
    TUD_CDC_DESCRIPTOR(
        ITF_NUM_CDC_1,
        4,
        EPNUM_CDC_1_NOTIF,
        8,
        EPNUM_CDC_1_DATA,
        0x80 | EPNUM_CDC_1_DATA,
        64),
};

#if TUD_OPT_HIGH_SPEED
uint8_t const blackmagic_desc_hs_configuration[] = {
    // Config number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(
        1,
        ITF_NUM_TOTAL,
        0,
        CONFIG_TOTAL_LEN,
        TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP,
        100),

    // 1st CDC: Interface number, string index, EP notification address and size, EP data address (out, in) and size.
    TUD_CDC_DESCRIPTOR(
        ITF_NUM_CDC_0,
        4,
        EPNUM_CDC_0_NOTIF,
        8,
        EPNUM_CDC_0_DATA,
        0x80 | EPNUM_CDC_0_DATA,
        512),

    // 2nd CDC: Interface number, string index, EP notification address and size, EP data address (out, in) and size.
    TUD_CDC_DESCRIPTOR(
        ITF_NUM_CDC_1,
        4,
        EPNUM_CDC_1_NOTIF,
        8,
        EPNUM_CDC_1_DATA,
        0x80 | EPNUM_CDC_1_DATA,
        512),
};
#endif

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

// array of pointer to string descriptors
static char* blackmagic_string_desc[] = {
    (char[]){0x09, 0x04}, // 0: is supported language is English (0x0409)
    "Flipper Devices Inc.", // 1: Manufacturer
    "Blackmagic ESP32", // 2: Product
    "blackmagic", // 3: Serials, should use chip ID
    "Blackmagic ESP32", // 4: CDC Interface
    "", // 5: MSC Interface
    "", // 6: HIDs
};

void blackmagic_set_serial_number(const char* serial_number) {
    blackmagic_string_desc[3] = malloc(strlen("blackmagic_") + strlen(serial_number) + 1);
    strcpy(blackmagic_string_desc[3], "blackmagic_");
    strcat(blackmagic_string_desc[3], serial_number);
}

#define MAX_DESC_BUF_SIZE 32
static uint16_t _desc_str[MAX_DESC_BUF_SIZE];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const* blackmagic_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void)langid;

    uint8_t chr_count;

    if(index == 0) {
        memcpy(&_desc_str[1], blackmagic_string_desc[0], 2);
        chr_count = 1;
    } else {
        // Convert ASCII string into UTF-16

        if(index >= sizeof(blackmagic_string_desc) / sizeof(blackmagic_string_desc[0])) {
            // ESP_LOGI("usb-str", "answer NULL");
            return NULL;
        }

        const char* str = blackmagic_string_desc[index];

        // Cap at max char
        chr_count = strlen(str);
        if(chr_count > MAX_DESC_BUF_SIZE - 1) {
            chr_count = MAX_DESC_BUF_SIZE - 1;
        }

        for(uint8_t i = 0; i < chr_count; i++) {
            _desc_str[1 + i] = str[i];
        }
    }

    // first byte is length (including header), second byte is string type
    _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);

    // ESP_LOG_BUFFER_HEXDUMP("usb-str", _desc_str, (2 * chr_count + 2), ESP_LOG_INFO);

    return _desc_str;
}
