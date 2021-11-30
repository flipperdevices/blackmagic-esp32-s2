#include <string.h>
#include <sys/param.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/stream_buffer.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <esp_netif.h>

#include <lwip/err.h>
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/netdb.h>

#include "led.h"
#include "delay.h"
#include "network-gdb.h"
#include <gdb-glue.h>

#define PORT 2345
#define KEEPALIVE_IDLE 5
#define KEEPALIVE_INTERVAL 5
#define KEEPALIVE_COUNT 3
#define TAG "network-gdb"

typedef struct {
    bool connected;
    int socket_id;
} NetworkGDB;

static NetworkGDB network_gdb;

bool network_gdb_connected(void) {
    return network_gdb.connected;
}

void network_gdb_send(uint8_t* buffer, size_t size) {
    int to_write = size;
    while(to_write > 0) {
        int written = send(network_gdb.socket_id, buffer + (size - to_write), to_write, 0);
        to_write -= written;
    }
};

void receive_and_send_to_gdb(void) {
    size_t rx_size = SIZE_MAX;
    size_t gdb_packet_size = gdb_glue_get_packet_size();
    uint8_t* buffer_rx = malloc(gdb_packet_size);

    do {
        if(gdb_glue_can_receive()) {
            size_t max_len = gdb_glue_get_free_size();
            if(max_len > gdb_packet_size) max_len = gdb_packet_size;
            rx_size = recv(network_gdb.socket_id, buffer_rx, max_len, 0);
            if(rx_size > 0) {
                gdb_glue_receive(buffer_rx, rx_size);
            }
        } else {
            delay(10);
        }
    } while(rx_size > 0);

    free(buffer_rx);
}

static void network_gdb_server_task(void* pvParameters) {
    char addr_str[128];
    int addr_family = (int)pvParameters;
    int ip_protocol = 0;
    int keepAlive = 1;
    int keepIdle = KEEPALIVE_IDLE;
    int keepInterval = KEEPALIVE_INTERVAL;
    int keepCount = KEEPALIVE_COUNT;
    network_gdb.connected = false;
    struct sockaddr_storage dest_addr;

    if(addr_family == AF_INET) {
        struct sockaddr_in* dest_addr_ip4 = (struct sockaddr_in*)&dest_addr;
        dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr_ip4->sin_family = AF_INET;
        dest_addr_ip4->sin_port = htons(PORT);
        ip_protocol = IPPROTO_IP;
    }

    int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if(listen_sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }
    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    ESP_LOGI(TAG, "Socket created");

    int err = bind(listen_sock, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
    if(err != 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        ESP_LOGE(TAG, "IPPROTO: %d", addr_family);
        goto CLEAN_UP;
    }
    ESP_LOGI(TAG, "Socket bound, port %d", PORT);

    err = listen(listen_sock, 1);
    if(err != 0) {
        ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        goto CLEAN_UP;
    }

    while(1) {
        ESP_LOGI(TAG, "Socket listening");

        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t addr_len = sizeof(source_addr);
        int sock = accept(listen_sock, (struct sockaddr*)&source_addr, &addr_len);
        if(sock < 0) {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }

        // Set tcp keepalive option
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &keepIdle, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));

        // Convert ip address to string
        if(source_addr.ss_family == PF_INET) {
            inet_ntoa_r(
                ((struct sockaddr_in*)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
        }

        ESP_LOGI(TAG, "Socket accepted ip address: %s", addr_str);

        led_set_blue(255);
        delay(10);
        led_set_blue(0);

        network_gdb.socket_id = sock;
        network_gdb.connected = true;

        receive_and_send_to_gdb();

        network_gdb.connected = false;
        network_gdb.socket_id = -1;

        led_set_blue(255);
        delay(10);
        led_set_blue(0);

        shutdown(sock, 0);
        close(sock);
    }

CLEAN_UP:
    close(listen_sock);
    vTaskDelete(NULL);
}

void network_gdb_server_init(void) {
    network_gdb.connected = false;
    network_gdb.socket_id = -1;

    esp_wifi_set_ps(WIFI_PS_NONE);
    xTaskCreate(network_gdb_server_task, "network_gdb_server", 4096, (void*)AF_INET, 5, NULL);
}
