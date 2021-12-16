#include <tusb.h>
#include <class/cdc/cdc_device.h>
#include <driver/gpio.h>
#include <driver/periph_ctrl.h>
#include <hal/usb_hal.h>
#include <soc/gpio_periph.h>
#include <soc/usb_periph.h>
#include <esp_rom_gpio.h>
#include <hal/gpio_ll.h>
#include <esp_log.h>
#include <esp_check.h>

#include "dual-cdc-driver.h"

#define TAG "usb-dual-cdc"

#define CONFIG_TINYUSB_TASK_STACK_SIZE 4096
#define CONFIG_TINYUSB_TASK_PRIORITY 17

static void configure_pins(usb_hal_context_t* usb) {
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

static void tusb_device_task(void* arg) {
    ESP_LOGD(TAG, "tinyusb task started");
    while(1) { // RTOS forever loop
        tud_task();
    }
}

esp_err_t dual_cdc_driver_install(void) {
    // Enable APB CLK to USB peripheral
    periph_module_enable(PERIPH_USB_MODULE);
    periph_module_reset(PERIPH_USB_MODULE);

    // Initialize HAL layer
    usb_hal_context_t hal = {.use_external_phy = false};
    usb_hal_init(&hal);
    configure_pins(&hal);

    ESP_RETURN_ON_FALSE(tusb_init(), ESP_FAIL, TAG, "init TinyUSB failed");

    TaskHandle_t s_tusb_tskh;
    xTaskCreate(
        tusb_device_task,
        "TinyUSB",
        CONFIG_TINYUSB_TASK_STACK_SIZE,
        NULL,
        CONFIG_TINYUSB_TASK_PRIORITY,
        &s_tusb_tskh);
    ESP_RETURN_ON_FALSE(s_tusb_tskh, ESP_FAIL, TAG, "create TinyUSB main task failed");

    ESP_LOGI(TAG, "TinyUSB Driver installed");
    return ESP_OK;
}