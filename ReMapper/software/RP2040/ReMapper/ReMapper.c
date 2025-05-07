
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

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
    (void)instance;

    if (report_type == HID_REPORT_TYPE_OUTPUT) {
        // Set keyboard LED e.g Capslock, Numlock etc...
        if (report_id == REPORT_ID_KEYBOARD) {
            // bufsize should be (at least) 1
            if (bufsize < 1) return;

            uint8_t const kbd_leds = buffer[0];

            if (kbd_leds & KEYBOARD_LED_CAPSLOCK) {
                // Capslock On: disable blink, turn led on
                board_led_write(true);
            } else {
                // Caplocks Off: back to normal blink
                board_led_write(false);
            }
        }
    }


    if (report_type == HID_REPORT_TYPE_INPUT) {
        printf("GOT IN REPORT\n");
    }

}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

  return 0;
}


void init_hid() {
    tud_init(BOARD_TUD_RHPORT);
}

void step_hid() {
    tud_task(); // tinyusb device task
}

int main() {
    stdio_init_all();

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
        // gpio_put(DEBUG_LED_PIN, led_toggle);
        // led_toggle = !led_toggle;

        step_spi();
        step_hid();
        sleep_us(10);
    }
}
