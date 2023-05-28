#include <tusb.h>

enum { VENDOR_REQUEST_WEBUSB = 1, VENDOR_REQUEST_MICROSOFT = 2 };

extern tusb_desc_device_t const dap_link_desc_device;
extern uint8_t const dap_link_desc_configuration[];
extern uint8_t const dap_link_desc_bos[];
extern uint8_t const dap_link_desc_ms_os_20[];
uint16_t const* dap_link_descriptor_string_cb(uint8_t index, uint16_t langid);
void dap_link_set_serial_number(const char* serial_number);