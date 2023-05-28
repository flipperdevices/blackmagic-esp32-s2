/* SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2022 Koji KITAYAMA
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
 * THE SOFTWARE. */

#include <tusb.h>
#include "dap-link-descriptors.h"

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+

tusb_desc_device_t const dap_link_desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0210, // at least 2.1 or 3.x for BOS & webUSB

    // Use Interface Association Descriptor (IAD) for CDC
    // As required by USB Specs IAD's subclass must be common class (2) and protocol must be IAD (1)
    .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor = 0x303A, // USB_ESPRESSIF_VID,
    .idProduct = 0x4002, // USB_TUSB_PID,
    .bcdDevice = 0x0100,

    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,

    .bNumConfigurations = 0x01,
};

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

enum {
    ITF_NUM_CDC = 0,
    ITF_NUM_CDC_DATA,
    ITF_NUM_VENDOR,
    ITF_NUM_TOTAL,
};

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN + TUD_VENDOR_DESC_LEN)

#if CFG_TUSB_MCU == OPT_MCU_LPC175X_6X || CFG_TUSB_MCU == OPT_MCU_LPC177X_8X || \
    CFG_TUSB_MCU == OPT_MCU_LPC40XX
// LPC 17xx and 40xx endpoint type (bulk/interrupt/iso) are fixed by its number
// 0 control, 1 In, 2 Bulk, 3 Iso, 4 In etc ...
#define EPNUM_CDC_IN 2
#define EPNUM_CDC_OUT 2
#define EPNUM_VENDOR_IN 5
#define EPNUM_VENDOR_OUT 5
#elif CFG_TUSB_MCU == OPT_MCU_SAMG || CFG_TUSB_MCU == OPT_MCU_SAMX7X
// SAMG & SAME70 don't support a same endpoint number with different direction IN and OUT
//    e.g EP1 OUT & EP1 IN cannot exist together
#define EPNUM_CDC_IN 2
#define EPNUM_CDC_OUT 3
#define EPNUM_VENDOR_IN 4
#define EPNUM_VENDOR_OUT 5
#else
#define EPNUM_CDC_IN 2
#define EPNUM_CDC_OUT 2
#define EPNUM_VENDOR_IN 3
#define EPNUM_VENDOR_OUT 3
#endif

uint8_t const dap_link_desc_configuration[] = {
    // Config number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(
        1,
        ITF_NUM_TOTAL,
        0,
        CONFIG_TOTAL_LEN,
        TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP,
        100),

    // Interface number, string index, EP notification address and size, EP data address (out, in) and size.
    TUD_CDC_DESCRIPTOR(
        ITF_NUM_CDC,
        4,
        0x81,
        8,
        EPNUM_CDC_OUT,
        0x80 | EPNUM_CDC_IN,
        TUD_OPT_HIGH_SPEED ? 512 : 64),

    // Interface number, string index, EP Out & IN address, EP size
    TUD_VENDOR_DESCRIPTOR(
        ITF_NUM_VENDOR,
        5,
        EPNUM_VENDOR_OUT,
        0x80 | EPNUM_VENDOR_IN,
        TUD_OPT_HIGH_SPEED ? 512 : 64),
};

//--------------------------------------------------------------------+
// BOS Descriptor
//--------------------------------------------------------------------+

/* Microsoft OS 2.0 registry property descriptor
Per MS requirements https://msdn.microsoft.com/en-us/library/windows/hardware/hh450799(v=vs.85).aspx
device should create DeviceInterfaceGUIDs. It can be done by driver and
in case of real PnP solution device should expose MS "Microsoft OS 2.0
registry property descriptor". Such descriptor can insert any record
into Windows registry per device/configuration/interface. In our case it
will insert "DeviceInterfaceGUIDs" multistring property.
GUID is freshly generated and should be OK to use.
https://developers.google.com/web/fundamentals/native-hardware/build-for-webusb/
(Section Microsoft OS compatibility descriptors)
*/

#define BOS_TOTAL_LEN (TUD_BOS_DESC_LEN + TUD_BOS_WEBUSB_DESC_LEN + TUD_BOS_MICROSOFT_OS_DESC_LEN)

#define MS_OS_20_DESC_LEN 0xB2

// BOS Descriptor is required for webUSB
uint8_t const dap_link_desc_bos[] = {
    // total length, number of device caps
    TUD_BOS_DESCRIPTOR(BOS_TOTAL_LEN, 2),

    // Vendor Code, iLandingPage
    TUD_BOS_WEBUSB_DESCRIPTOR(VENDOR_REQUEST_WEBUSB, 1),

    // Microsoft OS 2.0 descriptor
    TUD_BOS_MS_OS_20_DESCRIPTOR(MS_OS_20_DESC_LEN, VENDOR_REQUEST_MICROSOFT),
};

TU_VERIFY_STATIC(sizeof(dap_link_desc_bos) == BOS_TOTAL_LEN, "Incorrect size");

uint8_t const dap_link_desc_ms_os_20[] = {
    // Set header: length, type, windows version, total length
    U16_TO_U8S_LE(0x000A),
    U16_TO_U8S_LE(MS_OS_20_SET_HEADER_DESCRIPTOR),
    U32_TO_U8S_LE(0x06030000),
    U16_TO_U8S_LE(MS_OS_20_DESC_LEN),

    // Configuration subset header: length, type, configuration index, reserved, configuration total length
    U16_TO_U8S_LE(0x0008),
    U16_TO_U8S_LE(MS_OS_20_SUBSET_HEADER_CONFIGURATION),
    0,
    0,
    U16_TO_U8S_LE(MS_OS_20_DESC_LEN - 0x0A),

    // Function Subset header: length, type, first interface, reserved, subset length
    U16_TO_U8S_LE(0x0008),
    U16_TO_U8S_LE(MS_OS_20_SUBSET_HEADER_FUNCTION),
    ITF_NUM_VENDOR,
    0,
    U16_TO_U8S_LE(MS_OS_20_DESC_LEN - 0x0A - 0x08),

    // MS OS 2.0 Compatible ID descriptor: length, type, compatible ID, sub compatible ID
    U16_TO_U8S_LE(0x0014),
    U16_TO_U8S_LE(MS_OS_20_FEATURE_COMPATBLE_ID),
    'W',
    'I',
    'N',
    'U',
    'S',
    'B',
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, // sub-compatible

    // MS OS 2.0 Registry property descriptor: length, type
    U16_TO_U8S_LE(MS_OS_20_DESC_LEN - 0x0A - 0x08 - 0x08 - 0x14),
    U16_TO_U8S_LE(MS_OS_20_FEATURE_REG_PROPERTY),
    U16_TO_U8S_LE(0x0007),
    U16_TO_U8S_LE(
        0x002A), // wPropertyDataType, wPropertyNameLength and PropertyName "DeviceInterfaceGUIDs\0" in UTF-16
    'D',
    0x00,
    'e',
    0x00,
    'v',
    0x00,
    'i',
    0x00,
    'c',
    0x00,
    'e',
    0x00,
    'I',
    0x00,
    'n',
    0x00,
    't',
    0x00,
    'e',
    0x00,
    'r',
    0x00,
    'f',
    0x00,
    'a',
    0x00,
    'c',
    0x00,
    'e',
    0x00,
    'G',
    0x00,
    'U',
    0x00,
    'I',
    0x00,
    'D',
    0x00,
    's',
    0x00,
    0x00,
    0x00,
    U16_TO_U8S_LE(0x0050), // wPropertyDataLength
    //bPropertyData: “{CDB3B5AD-293B-4663-AA36-1AAE46463776}”.
    '{',
    0x00,
    'C',
    0x00,
    'D',
    0x00,
    'B',
    0x00,
    '3',
    0x00,
    'B',
    0x00,
    '5',
    0x00,
    'A',
    0x00,
    'D',
    0x00,
    '-',
    0x00,
    '2',
    0x00,
    '9',
    0x00,
    '3',
    0x00,
    'B',
    0x00,
    '-',
    0x00,
    '4',
    0x00,
    '6',
    0x00,
    '6',
    0x00,
    '3',
    0x00,
    '-',
    0x00,
    'A',
    0x00,
    'A',
    0x00,
    '3',
    0x00,
    '6',
    0x00,
    '-',
    0x00,
    '1',
    0x00,
    'A',
    0x00,
    'A',
    0x00,
    'E',
    0x00,
    '4',
    0x00,
    '6',
    0x00,
    '4',
    0x00,
    '6',
    0x00,
    '3',
    0x00,
    '7',
    0x00,
    '7',
    0x00,
    '6',
    0x00,
    '}',
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
};

TU_VERIFY_STATIC(sizeof(dap_link_desc_ms_os_20) == MS_OS_20_DESC_LEN, "Incorrect size");

static char* string_desc_arr[] = {
    (char[]){0x09, 0x04}, // 0: is supported language is English (0x0409)
    "CMSIS-DAP", // 1: Manufacturer
    "CMSIS-DAP ESP32S2 Device", // 2: Product
    "DAP", // 3: Serials, should use chip ID
    "CMSIS-DAP CDC", // 4: CDC Interface
    "CMSIS-DAP DAP" // 5: Vendor Interface
};

void dap_link_set_serial_number(const char* serial_number) {
    string_desc_arr[3] = malloc(strlen("DAP_") + strlen(serial_number) + 1);
    strcpy(string_desc_arr[3], "DAP_");
    strcat(string_desc_arr[3], serial_number);
}

static uint16_t _desc_str[32];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const* dap_link_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void)langid;

    uint8_t chr_count;

    if(index == 0) {
        memcpy(&_desc_str[1], string_desc_arr[0], 2);
        chr_count = 1;
    } else {
        // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
        // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors

        if(!(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0]))) return NULL;

        const char* str = string_desc_arr[index];

        // Cap at max char
        chr_count = (uint8_t)strlen(str);
        if(chr_count > 31) chr_count = 31;

        // Convert ASCII string into UTF-16
        for(uint8_t i = 0; i < chr_count; i++) {
            _desc_str[1 + i] = str[i];
        }
    }

    // first byte is length (including header), second byte is string type
    _desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));

    return _desc_str;
}