#include <tusb.h>

extern tusb_desc_device_t const blackmagic_desc_device;
extern uint8_t const blackmagic_desc_fs_configuration[];
uint16_t const* blackmagic_descriptor_string_cb(uint8_t index, uint16_t langid);
void blackmagic_set_serial_number(const char* serial_number);