#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/binary_info.h"
#include "pico/time.h"

const uint32_t HEARTBEAT = 42;
const uint32_t ERROR_FLAG = 127;

const uint LED_PIN = 25;

const uint SINE_PERIOD = 1e4;  // ms
const uint MIN_PERIOD = 50;     // ms
const uint MAX_PERIOD = 500;    // ms

void chain_flash(const uint pin, const uint n)
{   // flash the led n times in succession
    gpio_put(pin, 0);
    sleep_ms(500);
    for (uint i=1; i<=n; ++i) {
        gpio_put(pin, 1);
        sleep_ms(150);
        gpio_put(pin, 0);
        sleep_ms(150);
    }
    sleep_ms(450);
}

double getDelay()
{
    const double shift = (double)to_ms_since_boot(get_absolute_time())
        *M_PI*2/SINE_PERIOD;
    return ((sin(shift)+1)/2*(MAX_PERIOD-MIN_PERIOD))+MIN_PERIOD;
}

void core1_entry()
{
// send entry signal
    chain_flash(LED_PIN, 2);
    multicore_fifo_push_blocking(HEARTBEAT);
    chain_flash(LED_PIN, 3);

// recieve entry signal
    uint32_t g = multicore_fifo_pop_blocking();
    chain_flash(LED_PIN, 7);
    if (g == HEARTBEAT) printf("success!");
    else printf("failure!");

    sleep_ms(500);
// main loop
    while (1) {
        gpio_put(LED_PIN, 0);
        sleep_ms(getDelay());
        gpio_put(LED_PIN, 1);
        sleep_ms(getDelay());
        //printf("hello after %lu ms!", );
        //printf("hello world!");
    }
}

int main() {
// metadata
    bi_decl(bi_program_description("Sine Blink"));
    bi_decl(bi_1pin_with_name(LED_PIN, "On-board LED"));

// hardware setup
    //setup_default_uart();                                   // enable uart stdout
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

// multicore setup
    chain_flash(LED_PIN, 1);
    multicore_launch_core1(core1_entry);

    uint32_t g = multicore_fifo_pop_blocking();
    chain_flash(LED_PIN, 4);
    if (g != HEARTBEAT) return printf("Multicore launch failed\n"), 1;
    chain_flash(LED_PIN, 5);
    multicore_fifo_push_blocking(HEARTBEAT);
    chain_flash(LED_PIN, 6);

// main loop
    while (1) {
        //printf("running for %lu ms\n", to_ms_since_boot(get_absolute_time()));
        printf("%c", multicore_fifo_pop_blocking());
    }
}

