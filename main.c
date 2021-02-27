#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
//#include "pico/time.h"

const uint LED_PIN = 25;

const uint SINE_PERIOD = 1e4;  // ms
const uint MIN_PERIOD = 50;     // ms
const uint MAX_PERIOD = 500;    // ms

double getDelay()
{
    const double shift = (double)to_ms_since_boot(get_absolute_time())
        *M_PI*2/SINE_PERIOD;
    return ((sin(shift)+1)/2*(MAX_PERIOD-MIN_PERIOD))+MIN_PERIOD;
}

int main() {
    //setup_default_uart();                                   // enable uart stdout


    bi_decl(bi_program_description("Sine Blink"));
    bi_decl(bi_1pin_with_name(LED_PIN, "On-board LED"));

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    while (1) {
        gpio_put(LED_PIN, 0);
        sleep_ms(getDelay());
        gpio_put(LED_PIN, 1);
        sleep_ms(getDelay());
        //printf("hello after %lu ms!", );
        //printf("hello world!");
    }
}

