#include <stdio.h>
#include "pico/stdlib.h"

int main() {
    stdio_init_all();
    
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    
    printf("Hello, world!\n");

    while (true) {
        gpio_put(PICO_DEFAULT_LED_PIN, true);
        printf("LED On!\n");
        sleep_ms(1000);
        gpio_put(PICO_DEFAULT_LED_PIN, false);
        printf("LED Off!\n");
        sleep_ms(1000);
    }
}