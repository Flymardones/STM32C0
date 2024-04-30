/*
 * ws2812_pwm.c
 *
 *  Created on: Feb 28, 2024
 *      Author: chris
 */

#include "ws2812_pwm.h"
#include "string.h"
#include "stdlib.h"


uint16_t ping_pong_data[48]; // Buffer used for sending data in ping pong fashion

// #pragma GCC optimize ("O3")
void ws2812_pwm_adjust_brightness(ws2812_configuration* ws2812_conf, uint8_t brightness) {
    uint8_t (*led_data)[3] = (uint8_t(*)[3])ws2812_conf->buffer;
    uint8_t send_data[ws2812_conf->led_num * 24];
    uint8_t green, red, blue;

   	for (int i = 0; i < ws2812_conf->led_num; i++) {
        green = led_data[i][GREEN] * brightness / 100;
        red = led_data[i][RED] * brightness / 100;
        blue = led_data[i][BLUE] * brightness / 100;

        if (ws2812_conf->dma) {
            for (int j = 0; j < 8; j++) {
                int index = i * 24 + j;
                send_data[index] = (green & (1 << (7 - j))) ? 36 : 16;
                send_data[index + 8] = (red & (1 << (7 - j))) ? 36 : 16;
                send_data[index + 16] = (blue & (1 << (7 - j))) ? 36 : 16;
            }
        }
        else {
            for (int j = 0; j < 8; j++) {
                int index = i * 24 + j;
                send_data[index] = (green & (1 << (7 - j))) ? 20 : 44;
                send_data[index + 8] = (red & (1 << (7 - j))) ? 20 : 44;
                send_data[index + 16] = (blue & (1 << (7 - j))) ? 20 : 44;
            }
        }
    }

    if (ws2812_conf->dma) {
        HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_1, (uint32_t *) send_data, sizeof(send_data) / sizeof(send_data[0]));
        // while(!__HAL_DMA_GET_FLAG(&hdma_tim1_ch1, DMA_FLAG_TC3)) {};
        while(!datasentflag){};
        datasentflag = 0;
    }
    else {
        for (int i = 0; i < (ws2812_conf->led_num * 24); i++) {
            TIM1->CCR1 = (uint32_t)send_data[i];
            TIM1->CR1 |= TIM_CR1_CEN;
            while(!(TIM1->SR & TIM_SR_CC1IF)) {}; // Wait for the update event
        }
    }
    ws2812_delay_us(280);
}

// #pragma GCC optimize ("O3")
void ws2812_pwm_fade(ws2812_configuration* ws2812_conf, uint16_t fade_time_ms) {

  uint16_t fade_delay = 0;
  uint8_t (*led_data)[3] = (uint8_t(*)[3])ws2812_conf->buffer;
  
  /* Do nothing */
  if (fade_time_ms < ws2812_conf->brightness) {
	return;
  }
  
  fade_delay = (fade_time_ms / ws2812_conf->brightness) / 2;
  
  for (int fade = ws2812_conf->brightness; fade >= 0; fade -= 2) {
    if (ws2812_conf->dma) {
        for (int i = 0; i < ws2812_conf->led_num; i++) {
            ws2812_pwm_data(ws2812_conf, led_data[i][GREEN],led_data[i][RED],led_data[i][BLUE], fade);
        }
    }
    else {
        ws2812_pwm_adjust_brightness(ws2812_conf, fade);
    }
    
	HAL_Delay(fade_delay);
  }
  
  for (int fade = 0; fade < ws2812_conf->brightness; fade += 2) {
    if (ws2812_conf->dma) {
        for (int i = 0; i < ws2812_conf->led_num; i++) {
            ws2812_pwm_data(ws2812_conf, led_data[i][GREEN],led_data[i][RED],led_data[i][BLUE], fade);
        }
    }
    else {
        ws2812_pwm_adjust_brightness(ws2812_conf, fade);
    }
	HAL_Delay(fade_delay);
  }
}

// #pragma GCC optimize ("O3")
void ws2812_pwm_data(ws2812_configuration* ws2812_conf, uint8_t green, uint8_t red, uint8_t blue, uint8_t brightness) {

	green = green * brightness / 100;
	red = red * brightness / 100;
	blue = blue * brightness / 100;

    if (ws2812_conf->dma) {
        uint16_t *send_data = ws2812_conf->ping_pong ? ping_pong_data : ping_pong_data + 24;

        for (int i = 0; i < 8; i++) {
            send_data[i] = (green & (1 << (7 - i))) ? 36 : 16;
            send_data[i + 8] = (red & (1 << (7 - i))) ? 36 : 16;
            send_data[i + 16] = (blue & (1 << (7 - i))) ? 36 : 16;
        }

        while(!datasentflag){}; // Wait for DMA to finish
        
        HAL_TIM_PWM_Start_DMA(ws2812_conf->handle, TIM_CHANNEL_1, (uint32_t *) send_data, 24 * sizeof(uint16_t));
        datasentflag = 0;
        ws2812_conf->ping_pong = !ws2812_conf->ping_pong;
        
    }
    else {
        uint8_t send_data[24];

        for (int i = 0; i < 8; i++) {
            send_data[i] = (green & (1 << (7 - i))) ? 20 : 44;
            send_data[i + 8] = (red & (1 << (7 - i))) ? 20 : 44;
            send_data[i + 16] = (blue & (1 << (7 - i))) ? 20 : 44;
        }

        for (int i = 0; i < 24; i++) {
            TIM1->CCR1 = (uint32_t)send_data[i];
            TIM1->CR1 |= TIM_CR1_CEN;
            while(!(TIM1->SR & TIM_SR_CC1IF)) {}; // Wait for the update event
        }
    }
}

void ws2812_pwm_send_single(ws2812_configuration* ws2812_conf) {
    uint8_t (*led_data)[3] = (uint8_t(*)[3])ws2812_conf->buffer;
    datasentflag = 1;
    
    for (int i = 0; i < ws2812_conf->led_num; i++) {
        ws2812_pwm_data(ws2812_conf, led_data[i][GREEN],led_data[i][RED],led_data[i][BLUE], ws2812_conf->brightness);

    }

    ws2812_delay_us(280);
}

// #pragma GCC optimize ("O3")
void ws2812_pwm_send(ws2812_configuration* ws2812_conf) {

    uint8_t (*led_data)[3] = (uint8_t(*)[3])ws2812_conf->buffer;
    uint8_t send_data[ws2812_conf->led_num * 24];
    uint8_t green, red, blue;


   	for (int i = 0; i < ws2812_conf->led_num; i++) {
        green = led_data[i][GREEN] * ws2812_conf->brightness / 100;
        red = led_data[i][RED] * ws2812_conf->brightness / 100;
        blue = led_data[i][BLUE] * ws2812_conf->brightness / 100;

        if (ws2812_conf->dma) {
            for (int j = 0; j < 8; j++) {
                int index = i * 24 + j;
                send_data[index] = (green & (1 << (7 - j))) ? 36 : 16;
                send_data[index + 8] = (red & (1 << (7 - j))) ? 36 : 16;
                send_data[index + 16] = (blue & (1 << (7 - j))) ? 36 : 16;
            }
        }
        else {
            for (int j = 0; j < 8; j++) {
                int index = i * 24 + j;
                send_data[index] = (green & (1 << (7 - j))) ? 36 : 16;
                send_data[index + 8] = (red & (1 << (7 - j))) ? 36 : 16;
                send_data[index + 16] = (blue & (1 << (7 - j))) ? 36 : 16;
            }
        }
    }

    if (ws2812_conf->dma) {
        HAL_TIM_PWM_Start_DMA(ws2812_conf->handle, TIM_CHANNEL_1, (uint32_t *) send_data, sizeof(send_data) / sizeof(send_data[0]));
        // while(!__HAL_DMA_GET_FLAG(&hdma_tim1_ch1, DMA_FLAG_TC3)) {};
        while(!datasentflag){};
        datasentflag = 0;
    }
    else {
        for (int i = 0; i < (ws2812_conf->led_num * 24); i++) {
            TIM1->CCR1 = (uint32_t)send_data[i];
            TIM1->CR1 |= TIM_CR1_CEN;
            while(!(TIM1->SR & TIM_SR_CC1IF)) {}; // Wait for the update event
        }
    }
    ws2812_delay_us(280);
}

void ws2812_pwm_clear(ws2812_configuration* ws2812_conf) {
    for (int i = 0; i < ws2812_conf->led_num; i++) {
        ws2812_set_led(ws2812_conf, i, 0, 0, 0);
    }
    // ws2812_pwm_send(ws2812_conf);
    ws2812_pwm_send_single(ws2812_conf);
}


bool ws2812_pwm_init(ws2812_configuration* ws2812_conf) {

    uint8_t (*led_data)[3] = malloc(ws2812_conf->led_num * sizeof(*led_data));

    if (led_data == NULL) {
        return false;
    }
    
    ws2812_conf->buffer = led_data;
    memset(ws2812_conf->buffer, 0, ws2812_conf->led_num * sizeof(*led_data));

    // ws2812_pwm_send(ws2812_conf);
    ws2812_pwm_send_single(ws2812_conf);

    return true;
}

void ws2812_pwm_deinit(ws2812_configuration* ws2812_conf) {

    free(ws2812_conf->buffer);
    ws2812_conf->buffer = NULL;
	ws2812_conf->handle = NULL;
	ws2812_conf->led_num = 0;
	ws2812_conf->brightness = 0;
	ws2812_conf->dma = 0;
}