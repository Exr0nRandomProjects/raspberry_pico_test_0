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

uint MICROSTEP = 1; // 1 or 4 or 8
const uint STEPS_PER_ROTATION = 1;
const double BELT_RATIO = 1;
//uint MICROSTEP = 8; // 1 or 4 or 8
//const uint STEPS_PER_ROTATION = 400;
//const double BELT_RATIO = 4;    // TODO diameter hub / diameter stepper

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

double CLOCK_FREQ = -1;
int set_rpm(const PIO pio, const uint sm, const double rpm)
{
    if (CLOCK_FREQ < 0) CLOCK_FREQ = pio_clock_freq();
    double ans = 30/rpm * CLOCK_FREQ/bitbang0_clock_divisor * MICROSTEP / STEPS_PER_ROTATION/BELT_RATIO;
    if (ans < bitbang0_upkeep_instruction_count) return 1; // too fast, not possible
    printf("Setting output to %u tps (%lf rpm)\n", (unsigned)ans, (double)1.0/(unsigned)ans *BELT_RATIO*STEPS_PER_ROTATION/MICROSTEP*bitbang0_clock_divisor/CLOCK_FREQ/30);
    pio_sm_put_blocking(pio, sm, (unsigned)ans-bitbang0_upkeep_instruction_count);
}

double TARGET_RPM = 0.1;
void ramp(const PIO pio, const uint sm, const double to,
          const double seconds, const double resolution)
{
    const double start_ms = get_absolute_time();
    double elapsed = 0;
    while (elapsed - start_ms < seconds*1e3) {
        // y=\frac{\left(\left(a-b\right)\cos\left(\frac{x\pi}{t}\right)+a+b\right)}{2}
        // where a = src, b = dst, t = len
        elapsed = get_absolute_time() - start_ms;
        printf("elapsed = %lf\n", elapsed);
        double inter = ((TARGET_RPM-to)*cos(elapsed*M_PI/seconds) + TARGET_RPM + to) / 2;
        set_rpm(pio, sm, inter);
    }
    set_rpm(pio, sm, TARGET_RPM = to);
}

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

    for (int i=1; i<=10; ++i) printf("printf???\n");

    ramp(pio, sm, 140, 10, 0.1);
    while (1) {
        char buf[64];
        double arg;
        scanf("%s %lf", buf, &arg);
        printf("got '%s' and %lf\n", buf, arg);
    }

    //set_rpm(pio, sm, 60);
    //while(1) {
    //    pio_sm_put_blocking(pio0, 0, 1e2);
    //    sleep_ms(1e4);
    //    pio_sm_put_blocking(pio0, 0, 1e3);
    //    sleep_ms(1e4);
    //}

// main loop
    //char message[] = "hello world.";
    while (1) {
        //if (multicore_fifo_rvalid()) printf("%c", multicore_fifo_pop_blocking());
        //printf("running for %lu ms\n", to_ms_since_boot(get_absolute_time()));
        //display_morse(message, BUILTIN_LED_PIN, 80);
        pio_sm_put_blocking(pio0, 0, 1<<31);
    }
}

