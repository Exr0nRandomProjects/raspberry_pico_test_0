#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
//#include "pico/time.h"

const uint LED_PIN = 25;

int main() {
    //setup_default_uart();                                   // enable uart stdout


    bi_decl(bi_program_description("Gobbles Pico Test"));
    bi_decl(bi_1pin_with_name(LED_PIN, "On-board LED"));

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    while (1) {
        gpio_put(LED_PIN, 0);
        sleep_ms(50);
        gpio_put(LED_PIN, 1);
        sleep_ms(2000);
        //printf("hello after %lu ms!", to_ms_since_boot(get_absolute_time()));
        printf("hello world!");
    }
}

