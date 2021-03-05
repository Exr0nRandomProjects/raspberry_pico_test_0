#include <stdio.h>
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/binary_info.h"
#include "pico/time.h"

#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "main.pio.h"

//const uint32_t HEARTBEAT = 42;
//const uint32_t ERROR_FLAG = 127;

#define BUILTIN_LED_PIN 25
const uint OUTPUT_PIN = 15;

const uint SINE_PERIOD = 1e4;  // ms
const uint MIN_PERIOD = 30;     // ms
const uint MAX_PERIOD = 100;    // ms

void display_morse(const char* str, const uint pin, const uint ms)
{
#define SHORT gpio_put(pin, 1); sleep_ms( ms ); gpio_put(pin, 0);
#define LONG  gpio_put(pin, 1); sleep_ms(ms*3); gpio_put(pin, 0);
    for (; *str; ++str) {
        switch (tolower(*str)) {
            case 'a': SHORT LONG break;
            case 'b': LONG SHORT SHORT SHORT break;
            case 'c': LONG SHORT LONG SHORT break;
            case 'd': LONG SHORT SHORT break;
            case 'e': SHORT break;
            case 'f': SHORT SHORT LONG SHORT break;
            case 'g': LONG LONG SHORT break;
            case 'h': SHORT SHORT SHORT SHORT break;
            case 'i': SHORT SHORT break;
            case 'j': SHORT LONG LONG LONG break;
            case 'k': LONG SHORT LONG break;
            case 'l': SHORT LONG SHORT SHORT break;
            case 'm': LONG LONG break;
            case 'n': LONG SHORT break;
            case 'o': LONG LONG LONG break;
            case 'p': SHORT LONG LONG SHORT break;
            case 'q': LONG LONG SHORT LONG break;
            case 'r': SHORT LONG SHORT break;
            case 's': SHORT SHORT SHORT break;
            case 't': LONG break;
            case 'u': SHORT SHORT LONG break;
            case 'v': SHORT SHORT SHORT LONG break;
            case 'w': SHORT LONG LONG break;
            case 'x': LONG SHORT SHORT LONG break;
            case 'y': LONG SHORT LONG LONG break;
            case 'z': LONG LONG SHORT LONG break;
            case '1': SHORT LONG LONG LONG LONG break;
            case '2': SHORT SHORT LONG LONG LONG break;
            case '3': SHORT SHORT SHORT LONG LONG break;
            case '4': SHORT SHORT SHORT SHORT LONG break;
            case '5': SHORT SHORT SHORT SHORT SHORT break;
            case '6': LONG SHORT SHORT SHORT SHORT break;
            case '7': LONG LONG SHORT SHORT SHORT break;
            case '8': LONG LONG LONG SHORT SHORT break;
            case '9': LONG LONG LONG LONG SHORT break;
            case '0': LONG LONG LONG LONG LONG break;
            case ' ': sleep_ms(4*ms); break;
            case '.': sleep_ms(6*ms); break;
            default: printf("unknown morse character!");
        }
        sleep_ms(ms);
    }
    sleep_ms(15*ms);
#undef SHORT
#undef LONG
}

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

//double getSineDelay()
//{
//    const double shift = (double)to_ms_since_boot(get_absolute_time())
//        *M_PI*2/SINE_PERIOD;
//    return ((sin(shift)+1)/2*(MAX_PERIOD-MIN_PERIOD))+MIN_PERIOD;
//}

//void core1_entry()
//{
//    //gpio_init(BUILTIN_LED_PIN);
//    //gpio_set_dir(BUILTIN_LED_PIN, GPIO_OUT);
//// send entry signal
//    chain_flash(BUILTIN_LED_PIN, 2);
//    multicore_fifo_push_blocking(HEARTBEAT);
//    chain_flash(BUILTIN_LED_PIN, 3);
//
//// recieve entry signal
//    uint32_t g = multicore_fifo_pop_blocking();
//    chain_flash(BUILTIN_LED_PIN, 7);
//    if (g == HEARTBEAT) printf("success!");
//    else printf("failure!");
//
//    sleep_ms(500);
//// main loop
//    while (1) {
//        gpio_put(BUILTIN_LED_PIN, 0);
//        sleep_ms(getSineDelay());
//        gpio_put(BUILTIN_LED_PIN, 1);
//        sleep_ms(getSineDelay());
//        //printf("hello after %lu ms!", );
//        //printf("hello world!");
//    }
//}

int main() {
// metadata
    bi_decl(bi_program_description("Sine Blink"));
    bi_decl(bi_1pin_with_name(BUILTIN_LED_PIN, "On-board LED"));

// hardware setup
    //setup_default_uart();                                   // enable uart stdout
    stdio_init_all();
    gpio_init(BUILTIN_LED_PIN);
    gpio_set_dir(BUILTIN_LED_PIN, GPIO_OUT);

//// multicore setup
//    chain_flash(BUILTIN_LED_PIN, 1);
//    multicore_launch_core1(core1_entry);
//
//    uint32_t g = multicore_fifo_pop_blocking();
//    chain_flash(BUILTIN_LED_PIN, 4);
//    if (g != HEARTBEAT) return printf("Multicore launch failed\n"), 1;
//    chain_flash(BUILTIN_LED_PIN, 5);
//    multicore_fifo_push_blocking(HEARTBEAT);
//    chain_flash(BUILTIN_LED_PIN, 6);

    // pio init
    printf("starting state machine...\n");
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &bitbang0_program);
    bitbang0_init(pio, sm, offset, OUTPUT_PIN, 1);

    pio_sm_put_blocking(pio0, 0, 1e2);
    while (1) {
        printf("value is %x\n", pio_sm_get(pio0, 0));
    }
    //while(1) {
    //    pio_sm_put_blocking(pio0, 0, 1e2);    // ms
    //    pio_sm_put_blocking(pio0, 0, 9e2);    // ms
    //}

// main loop
    char message[] = "hello world.";
    while (1) {
        //if (multicore_fifo_rvalid()) printf("%c", multicore_fifo_pop_blocking());
        //printf("running for %lu ms\n", to_ms_since_boot(get_absolute_time()));
        //display_morse(message, BUILTIN_LED_PIN, 80);
        pio_sm_put_blocking(pio0, 0, 1<<31);
    }
}

