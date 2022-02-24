#include "driver/gpio.h"
#include <string.h>
#include <esp_log.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "led.h"
#include "nvs.h"

#define BOOT_PIN (0)
#define ESP_INTR_FLAG_DEFAULT 0

#define RESET_TIMER_TICK 250
#define RESET_TIMER_TICKS_TO_RESET 40

// static timer
static StaticTimer_t reset_timer_data;
static TimerHandle_t reset_timer;
static size_t reset_counter = 0;

static void start_reset_timer() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if(xPortInIsrContext()) {
        xTimerStartFromISR(reset_timer, &xHigherPriorityTaskWoken);
    } else {
        xTimerStart(reset_timer, portMAX_DELAY);
    }

    if(xHigherPriorityTaskWoken != pdFALSE) {
        portYIELD_FROM_ISR(pdTRUE);
    }
}

static void stop_reset_timer() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if(xPortInIsrContext()) {
        xTimerStopFromISR(reset_timer, &xHigherPriorityTaskWoken);
    } else {
        xTimerStop(reset_timer, portMAX_DELAY);
    }

    reset_counter = 0;
    led_set(0, 0, 0);

    if(xHigherPriorityTaskWoken != pdFALSE) {
        portYIELD_FROM_ISR(pdTRUE);
    }
}

static void IRAM_ATTR boot_button_isr_handler(void* arg) {
    if(gpio_get_level(BOOT_PIN) == 0) {
        start_reset_timer();
    } else {
        stop_reset_timer();
    }
}

static void reset_timer_cb(TimerHandle_t xTimer) {
    (void)xTimer;

    if(gpio_get_level(BOOT_PIN) == 1) {
        stop_reset_timer();
    } else {
        reset_counter++;

        uint8_t led_color = 0;
        if(reset_counter < (RESET_TIMER_TICKS_TO_RESET / 2)) {
            // slow blink
            if(reset_counter % 4 < 2) {
                led_color = 0xFF;
            }
        } else {
            // fast blink
            if(reset_counter % 2 != 0) {
                led_color = 0xFF;
            }
        }

        if(reset_counter > RESET_TIMER_TICKS_TO_RESET) {
            stop_reset_timer();

            led_set(255, 0, 0);

            // waiting for button release
            while(gpio_get_level(BOOT_PIN) == 0) {
            }

            led_set(0, 255, 0);
            nvs_erase();
            led_set(0, 0, 255);
            esp_restart();

        } else {
            led_set(led_color, led_color, led_color);
        }
    }
}

void factory_reset_service_init(void) {
    gpio_config_t io_conf;
    memset(&io_conf, 0, sizeof(gpio_config_t));

    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = ((1 << BOOT_PIN));
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    // install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    // attach interrupt
    gpio_isr_handler_add(BOOT_PIN, boot_button_isr_handler, (void*)BOOT_PIN);

    // soft timers
    reset_timer = xTimerCreateStatic(
        NULL, pdMS_TO_TICKS(RESET_TIMER_TICK), true, NULL, reset_timer_cb, &reset_timer_data);
}
