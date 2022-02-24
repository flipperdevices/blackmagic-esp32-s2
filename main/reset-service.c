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
static StaticTimer_t reset_tmdef;
static TimerHandle_t reset_tm;
static size_t reset_counter = 0;

static void start_timer() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if(xPortInIsrContext()) {
        xTimerStartFromISR(reset_tm, &xHigherPriorityTaskWoken);
    } else {
        xTimerStart(reset_tm, portMAX_DELAY);
    }

    if(xHigherPriorityTaskWoken != pdFALSE) {
        portYIELD_FROM_ISR(pdTRUE);
    }
}

static void stop_timer() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if(xPortInIsrContext()) {
        xTimerStopFromISR(reset_tm, &xHigherPriorityTaskWoken);
    } else {
        xTimerStop(reset_tm, portMAX_DELAY);
    }

    reset_counter = 0;
    led_set(0, 0, 0);

    if(xHigherPriorityTaskWoken != pdFALSE) {
        portYIELD_FROM_ISR(pdTRUE);
    }
}

static void IRAM_ATTR gpio_isr_handler(void* arg) {
    // uint32_t gpio_num = (uint32_t)arg;
    // xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
    // ESP_LOGI("isr", "test");

    if(gpio_get_level(BOOT_PIN) == 0) {
        start_timer();
    } else {
        stop_timer();
    }
}

static void reset_tm_cb(TimerHandle_t xTimer) {
    (void)xTimer;

    if(gpio_get_level(BOOT_PIN) == 1) {
        stop_timer();
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
            stop_timer();

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

void reset_service_init(void) {
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
    gpio_isr_handler_add(BOOT_PIN, gpio_isr_handler, (void*)BOOT_PIN);

    // soft timers
    reset_tm = xTimerCreateStatic(
        NULL, pdMS_TO_TICKS(RESET_TIMER_TICK), true, NULL, reset_tm_cb, &reset_tmdef);
}
