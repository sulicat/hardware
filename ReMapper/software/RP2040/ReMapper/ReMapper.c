
#include <stdio.h>
#include "pico/stdlib.h"

#include "pico/binary_info.h"
#include "hardware/spi.h"

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

    // Make the SPI pins available to picotool ???
    // bi_decl(bi_4pins_with_func(PICO_DEFAULT_SPI_RX_PIN, PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_SCK_PIN, PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI));
}

int main() {
    stdio_init_all();

    // debug gpio
    gpio_init(DEBUG_LED_PIN);
    gpio_set_dir(DEBUG_LED_PIN, GPIO_OUT);

    // spi
    init_spi();

    char data[255];
    data[0] = 'a';
    data[1] = 'b';
    data[2] = 'c';
    data[3] = 'd';
    char response[255];

    while (true) {
        gpio_put(DEBUG_LED_PIN, led_toggle);
        led_toggle = !led_toggle;

        // // int bytes = spi_write_blocking(spi_controller, data, 3);
        // printf("bytes written/read: %d   Got data: %x %x %x %x\n",
        //        bytes,
        //        response[0],
        //        response[1],
        //        response[2],
        //        response[3]);

        // int bytes = spi_read_blocking(spi_controller, 0, response, 4);
        int bytes = spi_write_read_blocking(spi_controller, data, response, 4);

        printf("READ: %d bytes  .... %x %x %x %x\n", bytes, response[0], response[1], response[2], response[3]);

        sleep_us(10);

    }
}
