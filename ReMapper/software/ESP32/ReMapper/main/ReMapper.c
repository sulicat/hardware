#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "driver/spi_master.h"

#include "ReMapper.h"

// Prepare data to send
uint8_t tx_data[1024] = {0};
uint8_t rx_data[1024] = {0};

#define NEW_EVENTS_SIZE (30)
Event_t new_events[NEW_EVENTS_SIZE];



void print_event(Event_t* event){
    printf("EVENT\n");
}

int parse_hid_packet(HID_MESSAGE_PACKET_t* packet, Event_t* events, int max_events){
    int num_events = 0;
    print_hid_packet(packet);
    return num_events;
}

void uart_task(void *args) {

    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    const int UART_NUM = 1; // uart 1 for this
    uart_driver_install(UART_NUM, 1024 * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, 17, 18, 20, 21);

    uint8_t data[255];
    data[0] = 'a';
    data[1] = 'b';
    data[2] = 'c';

    while (1) {
        uart_write_bytes(UART_NUM, (const char *)data, 3);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void print_hid_packet(const HID_MESSAGE_PACKET_t *packet) {
    switch (packet->type) {
    case DEVICE_CONNECTED:
        printf("HID Message: DEVICE_CONNECTED\n");
        printf("  dev_addr: %d\n", packet->message.connected.dev_addr);
        break;

    case DEVICE_DISCONNECTED:
        printf("HID Message: DEVICE_DISCONNECTED\n");
        printf("  dev_addr: %d\n", packet->message.disconnected.dev_addr);
        break;

    case NEW_REPORT:
        printf("HID Message: NEW_REPORT\n");
        printf("  dev_addr : %d\n", packet->message.new_report.dev_addr);
        printf("  report_id: %d\n", packet->message.new_report.report_id);
        printf("  length   : %d\n", packet->message.new_report.length);
        printf("  data     :");
        for (int i = 0; i < packet->message.new_report.length; i++) {
            printf(" %02X", packet->message.new_report.data[i]);
        }
        printf("\n");
        break;

    default:
        printf("HID Message: UNKNOWN TYPE (%d)\n", packet->type);
        break;
    }
}

void spi_master_task(void *args) {

    spi_device_handle_t rpi_spi;

    const int PIN_NUM_MOSI = 4;
    const int PIN_NUM_MISO = 5;
    const int PIN_NUM_CLK = 6;
    const int PIN_NUM_CS = 7;

    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1, // Not used
        .quadhd_io_num = -1, // Not used
        .max_transfer_sz = 32,
    };

    esp_err_t ret = spi_bus_initialize(SPI2_HOST,
                                       &buscfg,
                                       SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 1 * 100 * 1000, // Clock out at 1 MHz
        .mode = 0,                        // SPI mode 0
        .spics_io_num = PIN_NUM_CS,       // CS pin
        .queue_size = 1,                  // We want to be able to queue 7 transactions at a time
    };

    ret = spi_bus_add_device(SPI2_HOST,
                             &devcfg,
                             &rpi_spi);
    ESP_ERROR_CHECK(ret);

    printf(" -> DONE WITH SPI INITIALIZATION: spi ptr: %p\n", rpi_spi);

    spi_transaction_t t = {
        .length = 8 * sizeof(HID_MESSAGE_PACKET_t),
        .tx_buffer = tx_data,
        .rx_buffer = rx_data,
    };

    while (1) {

        memset(rx_data, 0, sizeof(rx_data));

        ret = spi_device_transmit(rpi_spi, &t);
        ESP_ERROR_CHECK(ret);

        HID_MESSAGE_PACKET_t hid_packet;
        memcpy(&hid_packet, rx_data, sizeof(HID_MESSAGE_PACKET_t));

        if (hid_packet.sync_word[0] == 'D' && hid_packet.sync_word[1] == 'R' && hid_packet.sync_word[2] == 'E' && hid_packet.sync_word[3] == 'A' && hid_packet.sync_word[4] == 'M') {
            // for (int i = 0; i < sizeof(HID_MESSAGE_PACKET_t); i++) {
            //     if (i % 16 == 0) printf("\n");
            //     printf("%02x ", rx_data[i]);
            // }
            // printf("\n\n");
            // print_hid_packet(&hid_packet);

            int num_events = parse_hid_packet(&hid_packet, new_events, NEW_EVENTS_SIZE);
            for( int i = 0; i < num_events; i++ )
                print_event(&new_events[i]);
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void app_main(void) {

    // xTaskCreate(uart_task,
    //             "uart_task",
    //             2048,
    //             NULL,
    //             10,
    //             NULL);

    xTaskCreate(spi_master_task,
                "spi_master",
                20480,
                NULL,
                10,
                NULL);
}
