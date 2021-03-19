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

//uint MICROSTEP = 1; // 1 or 4 or 8
//const uint STEPS_PER_ROTATION = 1;
//const double BELT_RATIO = 1;
uint MICROSTEP = 8; // 1 or 4 or 8
const uint STEPS_PER_ROTATION = 200;
const double BELT_RATIO = 100/7;    // TODO diameter hub / diameter stepper

#define BUILTIN_LED_PIN 25
const uint OUTPUT_PIN = 15;

const uint FLASH_PERIOD = 2e2;  // ms
const int CORECOM_FLASH = 1;
const int CORECOM_SOLID = 2;
const int CORECOM_ERROR = -1;

void core1_entry()
{
    int state = CORECOM_SOLID;
// main loop
    while (1) {
        if (multicore_fifo_rvalid()) state = multicore_fifo_pop_blocking();

        if (state == CORECOM_FLASH) {
            gpio_put(BUILTIN_LED_PIN, 0);
            sleep_ms(FLASH_PERIOD);
        }
        gpio_put(BUILTIN_LED_PIN, 1);
        sleep_ms(FLASH_PERIOD);
    }
}

void print_progbar(const uint len, const double lo, const double hi, const double pos, const double base_pos)
{
    printf("%.2lf |", lo);
    for (uint i=0; i<len; ++i) {
        if ((double)i/len <= pos && pos <= (double)(i+1)/len)
            printf("o");
        else if ((double)i/len <= base_pos)
            printf("-");
        else 
            printf(" ");
    }
    printf("| %.2lf ", hi);
}

uint elapsed() {
    return to_ms_since_boot(get_absolute_time());
}

double CLOCK_FREQ = -1;
const double DRIVE_FACTOR = STEPS_PER_ROTATION*BELT_RATIO;
double calc_delay_time(const double rpm)
{
}
double set_rpm(const PIO pio, const uint sm, const double rpm)
{
    if (CLOCK_FREQ < 0) CLOCK_FREQ = pio_clock_freq()/bitbang0_clock_divisor;
    double ans = 30/rpm *CLOCK_FREQ*MICROSTEP/DRIVE_FACTOR;
    if (ans < bitbang0_upkeep_instruction_count) return -1; // too fast, not possible
    if (ans > 1e6) return -1;                               // too slow, disallow
    const double reached = (double)30/((unsigned)ans)*CLOCK_FREQ*MICROSTEP/DRIVE_FACTOR;
    printf(" %uipt %.2lftps %.2lfrpm)\r", (unsigned)ans, CLOCK_FREQ/(unsigned)ans/2, reached);
    //printf("set %.2lftps %.2lfrpm\r", CLOCK_FREQ/(unsigned)ans/2, reached);
    pio_sm_put_blocking(pio, sm, (unsigned)ans-bitbang0_upkeep_instruction_count);
    return reached;
}

double TARGET_RPM = 0.01;
void ramp(const PIO pio, const uint sm, const double to, const double seconds)
{
    //printf("Ramping to target %.2lf RPM over %.2lf seconds...\n", to, seconds);
    multicore_fifo_push_blocking(CORECOM_FLASH);
    const double start_ms = elapsed();
    double cur_ms; unsigned prev=0;
    do {
        cur_ms = elapsed();
        // y=\frac{\left(\left(a-b\right)\cos\left(\frac{x\pi}{t}\right)+a+b\right)}{2}
        // where a = src, b = dst, t = len
        double inter = ((TARGET_RPM-to)*cos((cur_ms-start_ms)/1e3*M_PI/seconds) + TARGET_RPM + to) / 2;
        if ((unsigned)inter != prev) {
            print_progbar(50, fmin(TARGET_RPM, to), fmax(TARGET_RPM, to), (inter-fmin(to, TARGET_RPM))/fabs(to-TARGET_RPM), (cur_ms - start_ms)/seconds/1e3);
            set_rpm(pio, sm, inter);    // TODO: delayed due to queue, should calculate how much time will elapse before next and push based on that
            sleep_ms(inter-elapsed()+cur_ms-1);
        }
    } while (cur_ms - start_ms < seconds*1e3);
    const double reached = set_rpm(pio, sm, TARGET_RPM = to);
    printf("Target RPM %lf approximated by %lf.                                                       \n", TARGET_RPM, reached);
    multicore_fifo_push_blocking(CORECOM_SOLID);
}

char buf[64];
double arg;
int main() {
// metadata
    bi_decl(bi_program_description("Sine Blink"));
    bi_decl(bi_1pin_with_name(BUILTIN_LED_PIN, "On-board LED"));

// hardware setup
    stdio_init_all();
    gpio_init(BUILTIN_LED_PIN);
    gpio_set_dir(BUILTIN_LED_PIN, GPIO_OUT);

// multicore setup
    multicore_launch_core1(core1_entry);

// pio init
    const PIO pio = pio0;
    const int sm = 0;
    const uint offset = pio_add_program(pio, &bitbang0_program);
    bitbang0_init(pio, sm, offset, OUTPUT_PIN, 1);


// main loop
    //set_rpm(pio, sm, TARGET_RPM = 240); ramp(pio, sm, 480, 3); // TODO FOR LED TESTING ONLY: remove when using stepper
    while (1) {
        memset(buf, 0, sizeof buf); arg = 0;
        scanf("%s %lf", buf, &arg);
        if (!strcmp(buf, "set")) ramp(pio, sm, arg, fmax(3, log10(fabs(arg-TARGET_RPM))*4));
    }
}

