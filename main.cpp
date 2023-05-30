/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include <chrono>

DigitalIn blue_echo(ARDUINO_UNO_D11);
DigitalOut blue_trigger(ARDUINO_UNO_D6);

DigitalIn red_echo(ARDUINO_UNO_D12);
DigitalOut red_trigger(ARDUINO_UNO_D7);

// timer for echo code
Timer echoDuration;
Ticker ultrasonic_ticker;
Timer timer;
using namespace std::chrono; // namespace for timers

// example setup for light1 using D4, D5, D6
BusOut red_lane(
    ARDUINO_UNO_D5,     // GREEN
    ARDUINO_UNO_D4,     // YELLOW
    ARDUINO_UNO_D3      // RED
);

BusOut blue_lane(
    ARDUINO_UNO_D10,    // GREEN
    ARDUINO_UNO_D9,     // YELLOW
    ARDUINO_UNO_D8      // RED
);

volatile bool red_detected = false;
volatile bool blue_detected = false;

// defines to make our lives easier when using busses
// can use same defines for light1 and light2 as long as wiring is consistent
#define RED 0b100 // top bit / pin is red
#define YELLOW 0b010 // middle bit / pin is yellow 
#define GREEN 0b001 // lowest bit / pin is green

#define IN_RANGE_CM 10.0
#define RED_CONTEST_SECONDS 120
#define BLUE_CONTEST_SECONDS 60

typedef enum {
    Init,
    RedGo,
    RedContested,
    RedSlow,
    RedEnd,
    BlueGo,
    BlueContested,
    BlueSlow,
    BlueEnd,
} State;

State state = Init;

void ultrasonic_tick_fn() {
    // Red sensor
    echoDuration.reset();
    red_trigger = 1;
    wait_us(10);
    red_trigger = 0;
    while (red_echo != 1);
    echoDuration.start();
    while (red_echo == 1);
    echoDuration.stop();
    red_detected = IN_RANGE_CM >= (float)duration_cast<microseconds>(echoDuration.elapsed_time()).count() / 58.0;

    // Blue sensor
    echoDuration.reset();
    blue_trigger = 1;
    wait_us(10);
    blue_trigger = 0;
    while (blue_echo != 1);
    echoDuration.start();
    while (blue_echo == 1);
    echoDuration.stop();
    blue_detected = IN_RANGE_CM >= (float)duration_cast<microseconds>(echoDuration.elapsed_time()).count() / 58.0;
}

void tick_fn() {
    switch (state) {
        case Init: {
            timer.reset();
            timer.start();
            printf("Init -> RedGo\n");
            state = RedGo;
            break;
        }
        case RedGo: {
            red_lane = GREEN;
            blue_lane = RED;

            if (blue_detected) {
                if (red_detected) {
                    printf("RedGo -> RedContested\n");
                    state = RedContested;
                } else {
                    printf("RedGo -> RedSlow\n");
                    state = RedSlow;
                }
                timer.reset();
            }
            break;
        }
        case RedContested: {
            red_lane = GREEN;
            blue_lane = RED;

            auto contested_seconds = duration_cast<seconds>(timer.elapsed_time()).count();
            if (contested_seconds >= RED_CONTEST_SECONDS || !red_detected) {
                // have been contested for more than 2 minutes
                // or there aren't any more red cars
                printf("RedContested -> RedSlow\n");
                state = RedSlow;
                timer.reset();
            }
            break;
        }
        case RedSlow: {
            red_lane = YELLOW;
            blue_lane = RED;

            auto waiting_seconds = duration_cast<seconds>(timer.elapsed_time()).count();
            if (waiting_seconds >= 5) {
                printf("RedSlow -> RedEnd\n");
                state = RedEnd;
                timer.reset();
            }
            break;
        }
        case RedEnd: {
            red_lane = RED;
            blue_lane = RED;

            auto waiting_seconds = duration_cast<seconds>(timer.elapsed_time()).count();
            if (waiting_seconds >= 5) {
                printf("RedEnd -> BlueGo\n");
                state = BlueGo;
            }
            break;
        }
        case BlueGo: {
            red_lane = RED;
            blue_lane = GREEN;

            if (blue_detected) {
                if (red_detected) {
                    printf("BlueGo -> BlueContested\n");
                    state = BlueContested;
                    timer.reset();
                }
            } else {
                printf("BlueGo -> BlueSlow\n");
                state = BlueSlow;
                timer.reset();
            }
            break;
        }
        case BlueContested: {
            red_lane = RED;
            blue_lane = GREEN;

            auto contested_seconds = duration_cast<seconds>(timer.elapsed_time()).count();
            if (contested_seconds >= BLUE_CONTEST_SECONDS || !blue_detected) {
                // have been contested for more than 2 minutes
                // or there aren't any more blue cars
                printf("BlueContested -> BlueSlow\n");
                state = BlueSlow;
                timer.reset();
            }
            break;
        }
        case BlueSlow: {
            red_lane = RED;
            blue_lane = YELLOW;

            auto waiting_seconds = duration_cast<seconds>(timer.elapsed_time()).count();
            if (waiting_seconds >= 5) {
                printf("BlueSlow -> BlueEnd\n");
                state = BlueEnd;
                timer.reset();
            }
            break;
        }
        case BlueEnd: {
            red_lane = RED;
            blue_lane = RED;

            auto waiting_seconds = duration_cast<seconds>(timer.elapsed_time()).count();
            if (waiting_seconds >= 5) {
                printf("BlueEnd -> RedGo\n");
                state = RedGo;
            }
            break;
        }
    }
}

// void exampleGetDistance(void) {
//     // trigger pulse
//     trigger = 1;
//     wait_us(10);
//     trigger = 0;

//     while (echo != 1); // wait for echo to go high
//     echoDuration.start(); // start timer
//     while (echo == 1);
//     echoDuration.stop(); // stop timer

//     // print distance in cm (switch to divison by 148 for inches)
//     printf("%f cm\n",(float)duration_cast<microseconds>(echoDuration.elapsed_time()).count()/58.0);
//     echoDuration.reset(); // need to reset timer (doesn't happen automatically)

//     thread_sleep_for(100); // use 100 ms measurement cycle
// }

int main() {
    // needed to use thread_sleep_for in debugger
    // your board will get stuck without it :(
    #if defined(MBED_DEBUG) && DEVICE_SLEEP
        HAL_DBGMCU_EnableDBGSleepMode();
    #endif

    ultrasonic_ticker.attach(ultrasonic_tick_fn, 100ms);

    while (true) {
        tick_fn();

        // use 100 ms measurement cycle
        thread_sleep_for(100);
    }
}
