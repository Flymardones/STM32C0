/*
 * ws2812.c
 *
 *  Created on: Feb 22, 2024
 *      Author: chris
 */

#include "ws2812.h"


// Stores all LEDs in memory (3 bytes per LED * ws2812_conf->led_num = 75 bytes)
void ws2812_set_led(ws2812_configuration* ws2812_conf, uint8_t led, uint8_t red, uint8_t green, uint8_t blue) {

    if (led >= ws2812_conf->led_num) {
        return;
    }
    uint8_t (*led_data)[3] = ws2812_conf->buffer;

    led_data[led][0] = green;
    led_data[led][1] = red;
    led_data[led][2] = blue;
}


void ws2812_delay_us(uint16_t us) {
    __HAL_TIM_SET_COUNTER(&htim1, 0);
    while (__HAL_TIM_GET_COUNTER(&htim1) < us);
}

