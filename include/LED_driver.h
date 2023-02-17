#ifndef LED_DRIVER_H
#define LED_DRIVER_H

#include <LED_Status.h>

static constexpr int led_pin_red   = 4;
static constexpr int led_pin_green = 16;
static constexpr int led_pin_blue  = 17;

StatusLED SLED(
    led_pin_red, 
    led_pin_green, 
    led_pin_blue
);

#endif