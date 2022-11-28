#include <tusb.h>
#include "dap-link/dap-link-descriptors.h"
#include "dual-cdc/dual-cdc-descriptors.h"
#include "usb-glue.h"

typedef struct {
    uint8_t const* desc_device;
    uint8_t const* desc_config;
    uint16_t const* (*desc_string_cb)(uint8_t, uint16_t);
} USBDevice;

const USBDevice usb_device[] = {
    [USBDeviceTypeDapLink] =
        {
            .desc_device = (uint8_t const*)&dap_link_desc_device,
            .desc_config = dap_link_desc_configuration,
            .desc_string_cb = dap_link_descriptor_string_cb,
        },
    [USBDeviceTypeDualCDC] =
        {
            .desc_device = (uint8_t const*)&blackmagic_desc_device,
            .desc_config = blackmagic_desc_fs_configuration,
            .desc_string_cb = blackmagic_descriptor_string_cb,
        },
};

static USBDeviceType usb_device_type = USBDeviceTypeDapLink;

void usb_glue_init(USBDeviceType device_type) {
    usb_device_type = device_type;
}

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const* tud_descriptor_device_cb(void) {
    return usb_device[usb_device_type].desc_device;
}

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const* tud_descriptor_configuration_cb(uint8_t index) {
    (void)index; // for multiple configurations
    return usb_device[usb_device_type].desc_config;
}

uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    return usb_device[usb_device_type].desc_string_cb(index, langid);
}

uint8_t const* tud_descriptor_bos_cb(void) {
    // return NULL;
    return dap_link_desc_bos;
}

bool tud_vendor_control_xfer_cb(
    uint8_t rhport,
    uint8_t stage,
    tusb_control_request_t const* request) {
    // nothing to with DATA & ACK stage
    if(stage != CONTROL_STAGE_SETUP) return true;

    switch(request->bRequest) {
    case VENDOR_REQUEST_MICROSOFT:
        if(request->wIndex == 7) {
            // Get Microsoft OS 2.0 compatible descriptor
            uint16_t total_len;
            memcpy(&total_len, dap_link_desc_ms_os_20 + 8, 2);

            return tud_control_xfer(rhport, request, (void*)dap_link_desc_ms_os_20, total_len);
        } else {
            return false;
        }

    default:
        // stall unknown request
        return false;
    }

    return true;
}