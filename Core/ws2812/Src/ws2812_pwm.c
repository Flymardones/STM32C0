/*
 * ws2812_pwm.c
 *
 *  Created on: Feb 28, 2024
 *      Author: chris
 */


#include "ws2812_pwm.h"
#include "string.h"
#include "stdlib.h"


#pragma GCC optimize ("O3")
void ws2812_pwm_data(ws2812_configuration* ws2812_conf, uint8_t green, uint8_t red, uint8_t blue) {

    green = green * ws2812_conf->brightness / 100;
    red = red * ws2812_conf->brightness / 100;
    blue = blue * ws2812_conf->brightness / 100;

    uint32_t send_data[24];



    if (ws2812_conf->dma) {
        HAL_TIM_PWM_Start_DMA(ws2812_conf->handle, TIM_CHANNEL_1, send_data, sizeof(send_data));
    }
    else {
        HAL_TIM_PWM_Start(ws2812_conf->handle, TIM_CHANNEL_1);
        
    }
}

void ws2812_pwm_send_single(ws2812_configuration* ws2812_conf) {
    uint8_t (*led_data)[3] = (uint8_t(*)[3])ws2812_conf->buffer;

    for (int i = 0; i < ws2812_conf->led_num; i++) {
        ws2812_pwm_data(ws2812_conf, led_data[i][0],led_data[i][1],led_data[i][2]);
    }
    // ws2812_delay_us(240);
}

#pragma GCC optimize ("O3")
void ws2812_pwm_send(ws2812_configuration* ws2812_conf) {


}

void ws2812_pwm_init(ws2812_configuration* ws2812_conf) {

    uint8_t (*led_data)[3] = malloc(ws2812_conf->led_num * sizeof(*led_data));
    
    ws2812_conf->buffer = led_data;
    memset(ws2812_conf->buffer, 0, ws2812_conf->led_num * sizeof(*led_data));

    ws2812_pwm_send(ws2812_conf);
}