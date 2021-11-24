/**
 * @file swd-spi-tap.c
 * @author Sergey Gavrilov (who.just.the.doctor@gmail.com)
 * @version 1.0
 * @date 2021-11-25
 * 
 * Does not work due to bug with switching 3-wire mode to RX.
 * 
 * https://github.com/espressif/esp-idf/issues/7800
 * 
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <adiv5.h>

#include <esp_log.h>
#include <driver/spi_master.h>
#include "../platform.h"
#include <string.h>

#define SWDTAP_DEBUG 0

typedef enum {
    SpiSwdDirFloat,
    SpiSwdDirDrive,
} SpiSwdDirection;

static bool spi_bus_initialized = false;
static spi_device_handle_t swd_spi_device;

inline static void swd_spi_transmit(spi_transaction_t* swd_spi_transaction) {
    ESP_ERROR_CHECK(spi_device_polling_transmit(swd_spi_device, swd_spi_transaction));
}

inline static uint32_t swd_spi_rx(int ticks) {
    uint32_t data = 0;
    spi_transaction_t swd_spi_transaction = {
        .rxlength = ticks,
        .tx_buffer = NULL,
        .rx_buffer = &data,
    };

    swd_spi_transmit(&swd_spi_transaction);

#if SWDTAP_DEBUG == 1
    ESP_LOGW("spi_rx", "< [%02u] 0x%08x", ticks, data);
#endif

    return data;
}

inline static void swd_spi_tx(uint32_t data, int ticks) {
    spi_transaction_t swd_spi_transaction = {
        .length = ticks,
        .tx_buffer = &data,
        .rx_buffer = NULL,
    };

    swd_spi_transmit(&swd_spi_transaction);

#if SWDTAP_DEBUG == 1
    ESP_LOGI("spi_tx", "> [%02u] 0x%08x", ticks, data);
#endif
}

static void swdspitap_turnaround(SpiSwdDirection direction) {
    static SpiSwdDirection old_direction = SpiSwdDirFloat;

    if(direction == old_direction) return;
    old_direction = direction;

    swd_spi_tx(1, 1);
}

static uint32_t swdspitap_seq_in(int ticks) {
    swdspitap_turnaround(SpiSwdDirFloat);
    return swd_spi_rx(ticks);
}

static bool swdspitap_seq_in_parity(uint32_t* ret, int ticks) {
    swdspitap_turnaround(SpiSwdDirFloat);
    *ret = swd_spi_rx(ticks);
    int parity = __builtin_popcount(*ret);
    uint32_t data = swd_spi_rx(1);
    parity += (data & 1);
    swdspitap_turnaround(SpiSwdDirDrive);
    return (parity & 1);
}

static void swdspitap_seq_out(uint32_t MS, int ticks) {
    swdspitap_turnaround(SpiSwdDirDrive);
    swd_spi_tx(MS, ticks);
}

static void swdspitap_seq_out_parity(uint32_t MS, int ticks) {
    int parity = __builtin_popcount(MS);
    swdspitap_turnaround(SpiSwdDirDrive);
    swd_spi_tx(MS, ticks);
    swd_spi_tx(parity & 1, 1);
}

int swdptap_init(ADIv5_DP_t* dp) {
    if(!spi_bus_initialized) {
        // config bus
        spi_bus_config_t swd_spi_pins = {
            .mosi_io_num = SWDIO_PIN, // SWD I/O
            .miso_io_num = -1,
            .sclk_io_num = SWCLK_PIN, // SWD CLK
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
        };
        ESP_ERROR_CHECK(spi_bus_initialize(HSPI_HOST, &swd_spi_pins, SPI_DMA_DISABLED));

        // add device to bus with config
        spi_device_interface_config_t swd_spi_config = {
            .mode = 0,
            .clock_speed_hz = 10 * 1000,
            .spics_io_num = -1,
            .flags = SPI_DEVICE_3WIRE | SPI_DEVICE_HALFDUPLEX | SPI_DEVICE_BIT_LSBFIRST,
            .queue_size = 24,
            .pre_cb = NULL,
            .post_cb = NULL,
        };
        ESP_ERROR_CHECK(spi_bus_add_device(HSPI_HOST, &swd_spi_config, &swd_spi_device));

        spi_bus_initialized = true;
    }

    // set functions
    dp->seq_in = swdspitap_seq_in;
    dp->seq_in_parity = swdspitap_seq_in_parity;
    dp->seq_out = swdspitap_seq_out;
    dp->seq_out_parity = swdspitap_seq_out_parity;

    return 0;
}
