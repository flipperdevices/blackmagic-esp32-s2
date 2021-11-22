#include <string.h>
#include <sys/param.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
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

#include <driver/uart.h>
#include <soc/uart_reg.h>

#define PORT 4444
#define KEEPALIVE_IDLE 5
#define KEEPALIVE_INTERVAL 5
#define KEEPALIVE_COUNT 3
#define BUF_SIZE (128)
#define RD_BUF_SIZE (128)
#define TAG "network-uart"

static int socket_id;
static bool connected;
static QueueHandle_t uart0_queue;

static void receive_and_send_to_uart(const int socket_id) {
    int received;
    char rx_buffer[BUF_SIZE];

    do {
        received = recv(socket_id, rx_buffer, sizeof(rx_buffer) - 1, 0);
        led_set_green(255);
        if(received > 0) {
            uart_write_bytes(UART_NUM_0, rx_buffer, received);
        }
        led_set_green(0);

    } while(received > 0);
}

static void network_uart_rx_task(void* pvParameters) {
    uart_event_t event;
    uint8_t* rx_buffer = (uint8_t*)malloc(BUF_SIZE);
    int received = 0;

    for(;;) {
        //Waiting for UART event.
        if(xQueueReceive(uart0_queue, (void*)&event, (portTickType)portMAX_DELAY)) {
            bzero(rx_buffer, BUF_SIZE);
            switch(event.type) {
            case UART_DATA:
                received = uart_read_bytes(UART_NUM_0, rx_buffer, event.size, portMAX_DELAY);
                if(connected) {
                    int to_write = received;
                    while(to_write > 0) {
                        led_set_green(255);
                        int written =
                            send(socket_id, rx_buffer + (received - to_write), to_write, 0);
                        led_set_green(0);
                        to_write -= written;
                    }
                }
                break;
            default:
                break;
            }
        }
    }

    free(rx_buffer);
    rx_buffer = NULL;
    vTaskDelete(NULL);
}

static void network_uart_server_task(void* pvParameters) {
    char addr_str[128];
    int addr_family = (int)pvParameters;
    int ip_protocol = 0;
    int keepAlive = 1;
    int keepIdle = KEEPALIVE_IDLE;
    int keepInterval = KEEPALIVE_INTERVAL;
    int keepCount = KEEPALIVE_COUNT;
    connected = false;
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
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));

        // Convert ip address to string
        if(source_addr.ss_family == PF_INET) {
            inet_ntoa_r(
                ((struct sockaddr_in*)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
        }

        ESP_LOGI(TAG, "Socket accepted ip address: %s", addr_str);

        esp_log_level_set("*", ESP_LOG_NONE);

        led_set_green(255);
        delay(10);
        led_set_green(0);

        socket_id = sock;
        connected = true;

        receive_and_send_to_uart(sock);

        connected = false;
        socket_id = -1;

        led_set_green(255);
        delay(10);
        led_set_green(0);

        esp_log_level_set("*", CONFIG_LOG_DEFAULT_LEVEL);

        shutdown(sock, 0);
        close(sock);
    }

CLEAN_UP:
    close(listen_sock);
    vTaskDelete(NULL);
}

void network_uart_server_init(void) {
    xTaskCreate(network_uart_server_task, "network_uart_server", 4096, (void*)AF_INET, 5, NULL);

    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    uart_driver_install(UART_NUM_0, BUF_SIZE * 2, BUF_SIZE * 2, 20, &uart0_queue, 0);
    uart_param_config(UART_NUM_0, &uart_config);

    xTaskCreate(network_uart_rx_task, "tcp_uart_rx", 4096, (void*)AF_INET, 5, NULL);
}