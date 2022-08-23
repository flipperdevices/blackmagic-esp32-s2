#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/stream_buffer.h>

#define GDB_TX_BUFFER_SIZE 4096
#define GDB_RX_BUFFER_SIZE 4096
#define GDB_RX_PACKET_MAX_SIZE 64
#define TAG "gdb-glue"

typedef struct {
    StreamBufferHandle_t rx_stream;
    bool rx_stream_full;
    uint8_t tx_buffer[GDB_TX_BUFFER_SIZE];
    size_t tx_buffer_index;
} GDBGlue;

static GDBGlue gdb_glue;

/* GDB socket */
bool network_gdb_connected(void);
void network_gdb_send(uint8_t* buffer, size_t size);

/* USB-CDC */
void usb_cdc_gdb_tx_char(uint8_t c, bool flush);

size_t gdb_glue_get_free_size(void) {
    return xStreamBufferSpacesAvailable(gdb_glue.rx_stream);
}

void gdb_glue_receive(uint8_t* buffer, size_t size) {
    size_t ret = xStreamBufferSend(gdb_glue.rx_stream, buffer, size, portMAX_DELAY);
    ESP_ERROR_CHECK(ret != size);
}

bool gdb_glue_can_receive() {
    uint16_t max_len = xStreamBufferSpacesAvailable(gdb_glue.rx_stream);
    bool can_receive = true;

    if(max_len <= 0) {
        gdb_glue.rx_stream_full = true;
        ESP_LOGE(TAG, "Stream is full");
        can_receive = false;
    };

    return can_receive;
}

size_t gdb_glue_get_packet_size() {
    return GDB_RX_PACKET_MAX_SIZE;
}

const char* gdb_glue_get_bm_version() {
    return FIRMWARE_VERSION;
}

void gdb_glue_init(void) {
    gdb_glue.rx_stream = xStreamBufferCreate(GDB_RX_BUFFER_SIZE, 1);
    gdb_glue.rx_stream_full = false;
    gdb_glue.tx_buffer_index = 0;
}

unsigned char gdb_if_getchar_to(int timeout) {
    uint8_t data;
    size_t received = xStreamBufferReceive(gdb_glue.rx_stream, &data, sizeof(uint8_t), timeout);

    if(received == 0) {
        return -1;
    }

    if(gdb_glue.rx_stream_full &&
       xStreamBufferSpacesAvailable(gdb_glue.rx_stream) >= GDB_RX_PACKET_MAX_SIZE) {
        gdb_glue.rx_stream_full = false;
        ESP_LOGW(TAG, "Stream freed");
    }

    return data;
}

unsigned char gdb_if_getchar(void) {
    return gdb_if_getchar_to(portMAX_DELAY);
}

void gdb_if_putchar(unsigned char c, int flush) {
    if(network_gdb_connected()) {
        gdb_glue.tx_buffer[gdb_glue.tx_buffer_index] = c;
        gdb_glue.tx_buffer_index++;

        if(gdb_glue.tx_buffer_index == GDB_TX_BUFFER_SIZE || flush) {
            network_gdb_send(gdb_glue.tx_buffer, gdb_glue.tx_buffer_index);
            gdb_glue.tx_buffer_index = 0;
        }
    } else {
        // Not sure why, but I could not get it to work with buffer
        // usb_cdc_gdb_tx_char(c, flush);
    }
}