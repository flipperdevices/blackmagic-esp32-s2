#include <tusb.h>
#include "dap-link/dap-link-descriptors.h"
#include "dual-cdc/dual-cdc-descriptors.h"
#include "usb-glue.h"

#define TAG "usb-glue"
#define CONFIG_TINYUSB_TASK_STACK_SIZE 4096
#define CONFIG_TINYUSB_TASK_PRIORITY 17

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

static USBDeviceType usb_device_type = USBDeviceTypeDualCDC;

typedef enum {
    BlackmagicCDCTypeGDB = 0,
    BlackmagicCDCTypeUART = 1,
} BlackmagicCDCType;

typedef enum {
    DapCDCTypeUART = 0,
} DapCDCType;

typedef struct {
    void (*connected)(void* context);
    void* connected_context;
    void (*disconnected)(void* context);
    void* disconnected_context;
    void (*cdc_receive)(void* context);
    void* cdc_receive_context;
    void (*cdc_line_coding)(cdc_line_coding_t const* p_line_coding, void* context);
    void* cdc_line_coding_context;
    void (*cdc_line_state)(bool dtr, bool rts, void* context);
    void* cdc_line_state_context;
    void (*gdb_receive)(void* context);
    void* gdb_receive_context;
    void (*dap_receive)(void* context);
    void* dap_receive_context;
} USBGlueCallbacks;

static USBGlueCallbacks callbacks = {
    .connected = NULL,
    .connected_context = NULL,
    .disconnected = NULL,
    .disconnected_context = NULL,
    .cdc_receive = NULL,
    .cdc_receive_context = NULL,
    .cdc_line_coding = NULL,
    .cdc_line_coding_context = NULL,
    .cdc_line_state = NULL,
    .cdc_line_state_context = NULL,
    .gdb_receive = NULL,
    .gdb_receive_context = NULL,
    .dap_receive = NULL,
    .dap_receive_context = NULL,
};

/***** Callbacks *****/

static void callback_connected(void) {
    if(callbacks.connected) {
        callbacks.connected(callbacks.connected_context);
    }
}

static void callback_disconnected(void) {
    if(callbacks.disconnected) {
        callbacks.disconnected(callbacks.disconnected_context);
    }
}

static void callback_cdc_receive() {
    if(callbacks.cdc_receive) {
        callbacks.cdc_receive(callbacks.cdc_receive_context);
    }
}

static void callback_cdc_line_coding(cdc_line_coding_t const* p_line_coding) {
    if(callbacks.cdc_line_coding) {
        callbacks.cdc_line_coding(p_line_coding, callbacks.cdc_line_coding_context);
    }
}

static void callback_cdc_line_state(bool dtr, bool rts) {
    if(callbacks.cdc_line_state) {
        callbacks.cdc_line_state(dtr, rts, callbacks.cdc_line_state_context);
    }
}

static void callback_gdb_receive() {
    if(callbacks.gdb_receive) {
        callbacks.gdb_receive(callbacks.gdb_receive_context);
    }
}

static void callback_dap_receive() {
    if(callbacks.dap_receive) {
        callbacks.dap_receive(callbacks.dap_receive_context);
    }
}

/***** Tiny USB *****/

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

void tud_vendor_rx_cb(uint8_t itf) {
    (void)itf;
    callback_dap_receive();
}

void tud_mount_cb(void) {
    callback_connected();
}

void tud_umount_cb(void) {
    callback_disconnected();
}

void tud_resume_cb(void) {
    callback_connected();
}

void tud_suspend_cb(bool remote_wakeup_en) {
    callback_disconnected();
}

void tud_cdc_rx_cb(uint8_t interface) {
    do {
        if(usb_device_type == USBDeviceTypeDualCDC) {
            if(interface == BlackmagicCDCTypeGDB) {
                callback_gdb_receive();
                break;
            } else if(interface == BlackmagicCDCTypeUART) {
                callback_cdc_receive();
                break;
            }
        } else if(usb_device_type == USBDeviceTypeDapLink) {
            if(interface == DapCDCTypeUART) {
                callback_cdc_receive();
                break;
            }
        }

        tud_cdc_n_read_flush(interface);
    } while(false);
}

void tud_cdc_line_state_cb(uint8_t interface, bool dtr, bool rts) {
    if(usb_device_type == USBDeviceTypeDualCDC) {
        if(interface == BlackmagicCDCTypeUART) {
            callback_cdc_line_state(dtr, rts);
        }
    } else if(usb_device_type == USBDeviceTypeDapLink) {
        if(interface == DapCDCTypeUART) {
            callback_cdc_line_state(dtr, rts);
        }
    }
}

void tud_cdc_line_coding_cb(uint8_t interface, cdc_line_coding_t const* p_line_coding) {
    if(usb_device_type == USBDeviceTypeDualCDC) {
        if(interface == BlackmagicCDCTypeUART) {
            callback_cdc_line_coding(p_line_coding);
        }
    } else if(usb_device_type == USBDeviceTypeDapLink) {
        if(interface == DapCDCTypeUART) {
            callback_cdc_line_coding(p_line_coding);
        }
    }
}

/***** HAL *****/

#include <driver/gpio.h>
#include <driver/periph_ctrl.h>
#include <hal/usb_hal.h>
#include <soc/usb_periph.h>
#include <esp_rom_gpio.h>
#include <hal/gpio_ll.h>
#include <delay.h>
#include <esp_log.h>
#include <esp_check.h>

static void usb_hal_init_pins(usb_hal_context_t* usb) {
    /* usb_periph_iopins currently configures USB_OTG as USB Device.
     * Introduce additional parameters in usb_hal_context_t when adding support
     * for USB Host.
     */
    for(const usb_iopin_dsc_t* iopin = usb_periph_iopins; iopin->pin != -1; ++iopin) {
        if((usb->use_external_phy) || (iopin->ext_phy_only == 0)) {
            esp_rom_gpio_pad_select_gpio(iopin->pin);
            if(iopin->is_output) {
                esp_rom_gpio_connect_out_signal(iopin->pin, iopin->func, false, false);
            } else {
                esp_rom_gpio_connect_in_signal(iopin->pin, iopin->func, false);
                if((iopin->pin != GPIO_FUNC_IN_LOW) && (iopin->pin != GPIO_FUNC_IN_HIGH)) {
                    gpio_ll_input_enable(&GPIO, iopin->pin);
                }
            }
            esp_rom_gpio_pad_unhold(iopin->pin);
        }
    }
    if(!usb->use_external_phy) {
        gpio_set_drive_capability(USBPHY_DM_NUM, GPIO_DRIVE_CAP_3);
        gpio_set_drive_capability(USBPHY_DP_NUM, GPIO_DRIVE_CAP_3);
    }
}

static void usb_hal_bus_reset() {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT_OD;
    io_conf.pin_bit_mask = ((1 << USBPHY_DM_NUM) | (1 << USBPHY_DP_NUM));
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    gpio_set_level(USBPHY_DM_NUM, 0);
    gpio_set_level(USBPHY_DP_NUM, 0);
    delay(100);
    gpio_set_level(USBPHY_DM_NUM, 1);
    gpio_set_level(USBPHY_DP_NUM, 1);
}

static void usb_hal_tusb_device_task(void* arg) {
    ESP_LOGD(TAG, "tinyusb task started");
    while(1) { // RTOS forever loop
        tud_task();
    }
}

/***** Glue *****/
char* serial_desc = NULL;
char dap_serial_number[32];

static void usb_glue_set_serial_number(uint8_t* serial_number, uint8_t length) {
    if(serial_desc != NULL) {
        free(serial_desc);
    }

    serial_desc = malloc(length * 2 + 1);
    for(uint8_t i = 0; i < length; i++) {
        uint8_t nibble = serial_number[i] >> 4;
        serial_desc[i * 2 + 0] = nibble < 10 ? '0' + nibble : 'A' + nibble - 10;
        nibble = serial_number[i] & 0x0F;
        serial_desc[i * 2 + 1] = nibble < 10 ? '0' + nibble : 'A' + nibble - 10;
    }
    serial_desc[length * 2] = 0;
}

const char* usb_glue_get_serial_number() {
    return serial_desc;
}

esp_err_t usb_glue_init(USBDeviceType device_type) {
    usb_device_type = device_type;

    uint8_t mac[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    if(esp_efuse_mac_get_default(mac) == ESP_OK) {
        usb_glue_set_serial_number(mac, 6);
        dap_link_set_serial_number(usb_glue_get_serial_number());
        blackmagic_set_serial_number(usb_glue_get_serial_number());
        strncpy(dap_serial_number, usb_glue_get_serial_number(), sizeof(dap_serial_number) - 1);
        dap_serial_number[sizeof(dap_serial_number) - 1] = '\0';
        ESP_LOGI(TAG, "Serial number: %s", usb_glue_get_serial_number());
        ESP_LOGI(TAG, "Dap serial number: %s", dap_serial_number);
    }

    usb_hal_bus_reset();

    // Enable APB CLK to USB peripheral
    periph_module_enable(PERIPH_USB_MODULE);
    periph_module_reset(PERIPH_USB_MODULE);

    // Initialize HAL layer
    usb_hal_context_t hal = {.use_external_phy = false};
    usb_hal_init(&hal);
    usb_hal_init_pins(&hal);

    ESP_RETURN_ON_FALSE(tusb_init(), ESP_FAIL, TAG, "init TinyUSB failed");

    TaskHandle_t s_tusb_tskh;
    xTaskCreate(
        usb_hal_tusb_device_task,
        "TinyUSB",
        CONFIG_TINYUSB_TASK_STACK_SIZE,
        NULL,
        CONFIG_TINYUSB_TASK_PRIORITY,
        &s_tusb_tskh);
    ESP_RETURN_ON_FALSE(s_tusb_tskh, ESP_FAIL, TAG, "create TinyUSB main task failed");

    ESP_LOGI(TAG, "TinyUSB Driver installed");

    return ESP_OK;
}

void usb_glue_reset_bus() {
    usb_hal_bus_reset();
}

void usb_glue_set_connected_callback(void (*callback)(void* context), void* context) {
    callbacks.connected = callback;
    callbacks.connected_context = context;
}

void usb_glue_set_disconnected_callback(void (*callback)(void* context), void* context) {
    callbacks.disconnected = callback;
    callbacks.disconnected_context = context;
}

void usb_glue_cdc_set_receive_callback(void (*callback)(void* context), void* context) {
    callbacks.cdc_receive = callback;
    callbacks.cdc_receive_context = context;
}

void usb_glue_cdc_set_line_coding_callback(
    void (*callback)(cdc_line_coding_t const* p_line_coding, void* context),
    void* context) {
    callbacks.cdc_line_coding = callback;
    callbacks.cdc_line_coding_context = context;
}

void usb_glue_cdc_set_line_state_callback(
    void (*callback)(bool dtr, bool rts, void* context),
    void* context) {
    callbacks.cdc_line_state = callback;
    callbacks.cdc_line_state_context = context;
}

void usb_glue_gdb_set_receive_callback(void (*callback)(void* context), void* context) {
    callbacks.gdb_receive = callback;
    callbacks.gdb_receive_context = context;
}

void usb_glue_dap_set_receive_callback(void (*callback)(void* context), void* context) {
    callbacks.dap_receive = callback;
    callbacks.dap_receive_context = context;
}

void usb_glue_cdc_send(const uint8_t* buf, size_t len, bool flush) {
    if(usb_device_type == USBDeviceTypeDualCDC) {
        tud_cdc_n_write(BlackmagicCDCTypeUART, buf, len);
        if(flush) {
            tud_cdc_n_write_flush(BlackmagicCDCTypeUART);
        }
    } else {
        tud_cdc_n_write(DapCDCTypeUART, buf, len);
        if(flush) {
            tud_cdc_n_write_flush(DapCDCTypeUART);
        }
    }
}

size_t usb_glue_cdc_receive(uint8_t* buf, size_t len) {
    if(usb_device_type == USBDeviceTypeDualCDC) {
        return tud_cdc_n_read(BlackmagicCDCTypeUART, buf, len);
    } else {
        return tud_cdc_n_read(DapCDCTypeUART, buf, len);
    }
}

void usb_glue_gdb_send(const uint8_t* buf, size_t len, bool flush) {
    if(usb_device_type == USBDeviceTypeDualCDC) {
        tud_cdc_n_write(BlackmagicCDCTypeGDB, buf, len);
        if(flush) {
            tud_cdc_n_write_flush(BlackmagicCDCTypeGDB);
        }
    } else {
        esp_system_abort("Wrong USB device type");
    }
}

size_t usb_glue_gdb_receive(uint8_t* buf, size_t len) {
    return tud_cdc_n_read(BlackmagicCDCTypeGDB, buf, len);
}

void usb_glue_dap_send(const uint8_t* buf, size_t len, bool flush) {
    if(usb_device_type == USBDeviceTypeDapLink) {
        tud_vendor_write(buf, len);
    } else {
        esp_system_abort("Wrong USB device type");
    }
}

size_t usb_glue_dap_receive(uint8_t* buf, size_t len) {
    return tud_vendor_read(buf, len);
}
