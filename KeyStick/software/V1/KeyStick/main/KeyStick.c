#include <stdio.h>

#include "sdkconfig.h"
#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void) {

    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = ((1ULL << 4) |
                            (1ULL << 5) |
                            (1ULL << 6));
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);


    int counter = 0;
    while (1) {

        gpio_set_level(4, counter == 0);
        gpio_set_level(5, counter == 1);
        gpio_set_level(6, counter == 2);
        counter += 1;
        counter = counter % 3;

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
