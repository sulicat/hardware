
#include <stdio.h>
#include "pico/stdlib.h"

#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "bsp/board_api.h"
#include "tusb.h"

#include "usb_descriptors.h"

const int DEBUG_LED_PIN = 25;
static bool led_toggle = false;

spi_inst_t *spi_controller;

void init_spi() {

    int SPI_RX_PIN = 16;
    int SPI_TX_PIN = 19;
    int SPI_SCK_PIN = 18;
    int SPI_CS_PIN = 17;

    spi_controller = spi0_hw;

    // Enable SPI 0 at 1 MHz and connect to GPIOs
    spi_init(spi_controller, 100 * 1000);
    spi_set_slave(spi_controller, true);

    gpio_set_function(SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SPI_TX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SPI_CS_PIN, GPIO_FUNC_SPI);
    spi_set_format(spi_controller, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_LSB_FIRST);
}

void step_spi() {
    char data[255];
    data[0] = 'a';
    data[1] = 'b';
    data[2] = 'c';
    data[3] = 'd';
    char response[255];

    int bytes = spi_write_read_blocking(spi_controller, data, response, 4);
    printf("READ: %d bytes  .... %x %x %x %x\n", bytes, response[0], response[1], response[2], response[3]);
}

// Invoked when device with HID interface is mounted
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *desc_report, uint16_t desc_len) {
    printf("HID device address %d mounted\n", dev_addr);
    tuh_hid_receive_report(dev_addr, instance); // Start receiving reports
}

// Invoked when device with HID is unmounted
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {
    printf("HID device address %d unmounted\n", dev_addr);
}

// Called when a HID report is received
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance,
                                uint8_t const *report, uint16_t len) {

    printf("HID report from device %d, instance %d: ", dev_addr, instance);
    for (int i = 0; i < len; i++) {
        printf("%02X ", report[i]);
    }
    printf("\n");

    // Continue receiving next report
    tuh_hid_receive_report(dev_addr, instance);
}

void init_hid() {
    tusb_init();
}

void step_hid() {
    tuh_task(); // tinyusb device task
}

int main() {
    stdio_init_all();

    printf("\n\n Started ReMapper V1\n");

    // debug gpio
    gpio_init(DEBUG_LED_PIN);
    gpio_set_dir(DEBUG_LED_PIN, GPIO_OUT);

    // board
    board_init();
    // spi
    init_spi();
    // USB
    init_hid();

    while (true) {
        gpio_put(DEBUG_LED_PIN, led_toggle);
        led_toggle = !led_toggle;

        // step_spi();
        step_hid();
        sleep_us(10);
    }
}
