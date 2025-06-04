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

uint8_t last_report[64];

#define NEW_EVENTS_SIZE (30)
Event_t new_events[NEW_EVENTS_SIZE];

void print_event(Event_t *event) {
    // printf("EVENT: %s %c\n", event->type == EVENT_KEY_RELEASED ? "RELEASED" : "PRESSED", event->ascii);
}

char keycode_from_report(char report_val) {
    return report_val;
}

char keycode_to_ascii(uint8_t keycode, bool shift) {
    // HID usage IDs 0x04 to 0x1D: 'a' to 'z'
    if (keycode >= 0x04 && keycode <= 0x1D) {
        return (shift ? 'A' : 'a') + (keycode - 0x04);
    }

    // HID usage IDs 0x1E to 0x27: '1' to '0'
    if (keycode >= 0x1E && keycode <= 0x27) {
        const char unshifted[] = "1234567890";
        const char shifted[] = "!@#$%^&*()";
        return shift ? shifted[keycode - 0x1E] : unshifted[keycode - 0x1E];
    }

    // Basic punctuation and space
    switch (keycode) {
    case 0x2C:
        return ' '; // Space
    case 0x2D:
        return shift ? '_' : '-';
    case 0x2E:
        return shift ? '+' : '=';
    case 0x2F:
        return shift ? '{' : '[';
    case 0x30:
        return shift ? '}' : ']';
    case 0x31:
        return shift ? '|' : '\\';
    case 0x33:
        return shift ? ':' : ';';
    case 0x34:
        return shift ? '"' : '\'';
    case 0x35:
        return shift ? '~' : '`';
    case 0x36:
        return shift ? '<' : ',';
    case 0x37:
        return shift ? '>' : '.';
    case 0x38:
        return shift ? '?' : '/';
    }

    return 0; // Unknown or non-printable
}

int parse_hid_packet(HID_MESSAGE_PACKET_t *packet, Event_t *events, int max_events) {
    int num_events = 0;

    uint8_t *data = packet->message.new_report.data;
    int data_len = packet->message.new_report.length;

    char modifier = data[0];
    bool is_l_ctrl = (data[0] >> 0) & 0b1;
    bool is_l_shift = (data[0] >> 1) & 0b1;
    bool is_l_alt = (data[0] >> 2) & 0b1;
    bool is_l_cmd = (data[0] >> 3) & 0b1;

    bool is_r_ctrl = (data[0] >> 4) & 0b1;
    bool is_r_shift = (data[0] >> 5) & 0b1;
    bool is_r_alt = (data[0] >> 6) & 0b1;
    bool is_r_cmd = (data[0] >> 7) & 0b1;

    bool is_shift = is_l_shift || is_r_shift;
    bool is_ctrl = is_l_ctrl || is_r_ctrl;
    bool is_alt = is_l_alt || is_r_alt;
    bool is_cmd = is_l_cmd || is_r_cmd;

    // char reserved = 0;

    for (int i = 2; i < data_len; i++) {

        if (data[i] == 0x00 && last_report[i] != 0x00 && num_events < max_events - 1) {
            // key released, what was in the last report
            events[num_events].type = EVENT_KEY_RELEASED;
            events[num_events].keycode = keycode_from_report(last_report[i]); // from last report
            events[num_events].ascii = keycode_to_ascii(events[num_events].keycode, is_shift);
            num_events += 1;

        } else if (data[i] != last_report[i] && last_report[i] == 0x00 && num_events < max_events - 1) {
            // key pressed
            events[num_events].type = EVENT_KEY_PRESSED;
            events[num_events].keycode = keycode_from_report(data[i]); // from data
            events[num_events].ascii = keycode_to_ascii(events[num_events].keycode, is_shift);
            num_events += 1;
        }

        last_report[i] = data[i];
    }

    return num_events;
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
            for (int i = 0; i < num_events; i++) {
                if (new_events[i].type == EVENT_KEY_PRESSED)
                    printf("%c\n", new_events[i].ascii);
            }
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void app_main(void) {
    xTaskCreate(spi_master_task,
                "spi_master",
                20480,
                NULL,
                10,
                NULL);
}
