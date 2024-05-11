/*
 * ws2812_pwm.c
 *
 *  Created on: Feb 28, 2024
 *      Author: chris
 */


#include "ws2812_pwm.h"
#include "string.h"
#include "stdlib.h"

#if PWM

uint8_t indx = 0;

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim) {

    // Check if this is the TIM peripheral we're interested in
    if (htim == &htim1) {
        if (indx < ws2812_pwm.led_num) {
            // Fill second half of dma buffer
            ws2812_pwm_data(&ws2812_pwm, ws2812_pwm.led_data[indx][GREEN], ws2812_pwm.led_data[indx][RED], ws2812_pwm.led_data[indx][BLUE]);
            indx++;
        }
        else if (indx < ws2812_pwm.led_num + 5) {
            for (uint8_t i = 24; i < 48; i++) { // Reset pulse (48 bits * 1.25 us = 60 us * 5 = 300 us)
                ws2812_pwm.circBuffer[i] = 0;
            }
            indx++;
        }
        else {
            HAL_TIM_PWM_Stop_DMA(&htim1, TIM_CHANNEL_1);
            transferDone = 1;
            indx = 0;
        }
    }
}

void HAL_TIM_PWM_PulseFinishedHalfCpltCallback(TIM_HandleTypeDef *htim) {

    // Check if this is the TIM peripheral we're interested in
    if (htim == &htim1) {
        if (indx < ws2812_pwm.led_num) {
            // Fill first half of dma buffer
            ws2812_pwm_data(&ws2812_pwm, ws2812_pwm.led_data[indx][GREEN], ws2812_pwm.led_data[indx][RED], ws2812_pwm.led_data[indx][BLUE]);
            indx++;
        }
        else if(indx < ws2812_pwm.led_num + 5) { // Reset pulse (48 bits * 1.25 us = 60 us * 5 = 300 us)
            for (uint8_t i = 0; i < 24; i++) {
                ws2812_pwm.circBuffer[i] = 0;
            }
            indx++;
        }
    }
}

void ws2812_pwm_fade(ws2812_configuration* ws2812_conf, uint16_t fade_time_ms) {

  uint16_t fade_delay = 0;
  
  /* Do nothing */
  if (fade_time_ms < ws2812_conf->brightness) {
	return;
  }
  
  fade_delay = (fade_time_ms / ws2812_conf->brightness);
  
  for (ws2812_conf->fade = ws2812_conf->brightness; ws2812_conf->fade >= 0; ws2812_conf->fade -= 2) {
    ws2812_pwm_send(ws2812_conf);
    HAL_Delay(fade_delay);
  }
  
  for (ws2812_conf->fade = 0; ws2812_conf->fade < ws2812_conf->brightness; ws2812_conf->fade += 2) {
    ws2812_pwm_send(ws2812_conf);
	HAL_Delay(fade_delay);

  }
}

#pragma GCC optimize ("O1")
void ws2812_pwm_data(ws2812_configuration* ws2812_conf, uint8_t green, uint8_t red, uint8_t blue) {

    if (fade_flag) {
        green = green * ws2812_conf->fade / 100;
        red = red * ws2812_conf->fade / 100;
        blue = blue * ws2812_conf->fade / 100;
    }
    else {
        green = green * ws2812_conf->brightness / 100;
        red = red * ws2812_conf->brightness / 100;
        blue = blue * ws2812_conf->brightness / 100;
    }

    if (ws2812_conf->dma) {
        uint8_t *send_data = ws2812_conf->ping_pong ? &ws2812_conf->circBuffer[0] : &ws2812_conf->circBuffer[24];
        
        for (uint8_t i = 0; i < 8; i++) {
            uint8_t mask = 1 << (7 - i);
            send_data[i] = (green & mask) ? 36 : 16;
            send_data[i + 8] = (red & mask) ? 36 : 16;
            send_data[i + 16] = (blue & mask) ? 36 : 16;
        }

        ws2812_conf->ping_pong = !ws2812_conf->ping_pong;
    }
    else {
        uint8_t send_data[24];
        
        for (uint8_t i = 0; i < 8; i++) {
            uint8_t mask = 1 << (7 - i);
            send_data[i] = (green & mask) ? 20 : 44;
            send_data[i + 8] = (red & mask) ? 20 : 44;
            send_data[i + 16] = (blue & mask) ? 20 : 44;
        }

        for (uint8_t i = 0; i < 24; i++) {
            TIM1->CCR1 = send_data[i];
            TIM1->CR1 |= TIM_CR1_CEN;
            while(TIM1->CR1 & TIM_CR1_CEN) {}; // wait for CEN bit to be cleared by hardware
        }  
    }
}

void ws2812_pwm_send(ws2812_configuration* ws2812_conf) {
    
    if (ws2812_conf->dma) {
        ws2812_conf->ping_pong = true; // Start with first half of dma buffer

        // Initialise data for one transfer and start DMA in circular mode  
        for (uint8_t i = 0; i < 2; i++) {
            ws2812_pwm_data(ws2812_conf, ws2812_conf->led_data[i][GREEN], ws2812_conf->led_data[i][RED], ws2812_conf->led_data[i][BLUE]);
        }
      
        indx = 2;
        transferDone = 0;
        HAL_TIM_PWM_Start_DMA(ws2812_conf->handle, TIM_CHANNEL_1, (uint32_t *) ws2812_conf->circBuffer, 48);
    }
    else {
        for (uint8_t i = 0; i < ws2812_conf->led_num; i++) {
            ws2812_pwm_data(ws2812_conf, ws2812_conf->led_data[i][GREEN], ws2812_conf->led_data[i][RED], ws2812_conf->led_data[i][BLUE]);
        }
     
        ws2812_delay_us(280);
    }
}


void ws2812_pwm_send_burst(ws2812_configuration* ws2812_conf) {

    uint8_t send_data[ws2812_conf->led_num * 24];
    uint8_t green, red, blue;

   	for (uint8_t i = 0; i < ws2812_conf->led_num; i++) {
        if (fade_flag) {
            green = ws2812_conf->led_data[i][GREEN] * ws2812_conf->fade / 100;
            red = ws2812_conf->led_data[i][RED] * ws2812_conf->fade / 100;
            blue = ws2812_conf->led_data[i][BLUE] * ws2812_conf->fade / 100;
        }
        else {
            green = ws2812_conf->led_data[i][GREEN] * ws2812_conf->brightness / 100;
            red = ws2812_conf->led_data[i][RED] * ws2812_conf->brightness / 100;
            blue = ws2812_conf->led_data[i][BLUE] * ws2812_conf->brightness / 100;
        }

        if (ws2812_conf->dma) {
            for (uint8_t j = 0; j < 8; j++) {
                int index = i * 24 + j;
                send_data[index] = (green & (1 << (7 - j))) ? 36 : 16;
                send_data[index + 8] = (red & (1 << (7 - j))) ? 36 : 16;
                send_data[index + 16] = (blue & (1 << (7 - j))) ? 36 : 16;
            }
        }
        else {
            for (uint8_t j = 0; j < 8; j++) {
                int index = i * 24 + j;
                send_data[index] = (green & (1 << (7 - j))) ? 20 : 44;
                send_data[index + 8] = (red & (1 << (7 - j))) ? 20 : 44;
                send_data[index + 16] = (blue & (1 << (7 - j))) ? 20 : 44;
            }
        }
    }

    if (ws2812_conf->dma) { // Only works if DMA is set to normal mode (not circular)
        HAL_TIM_PWM_Start_DMA(ws2812_conf->handle, TIM_CHANNEL_1, (uint32_t *) send_data, ws2812_conf->led_num*24);
        while(!transferDone){};
        transferDone = 0;
    }
    else {
        for (uint8_t i = 0; i < (ws2812_conf->led_num * 24); i++) {
            TIM1->CCR1 = (uint32_t)send_data[i];
            TIM1->CR1 |= TIM_CR1_CEN;
            while(TIM1->CR1 & TIM_CR1_CEN) {}; // wait for CEN bit to be cleared by hardware
        }
    }
    ws2812_delay_us(280);
}


void ws2812_pwm_clear(ws2812_configuration* ws2812_conf) {
    memset(ws2812_conf->led_data, 0, ws2812_conf->led_num * sizeof(*ws2812_conf->led_data));
    ws2812_pwm_send(ws2812_conf);
}


bool ws2812_pwm_init(ws2812_configuration* ws2812_conf) {

    ws2812_conf->led_data = malloc(sizeof(uint8_t[3]) * ws2812_conf->led_num);
    
    if (ws2812_conf->led_data == NULL) {
        return false;
    }
    
    memset(ws2812_conf->led_data, 0, ws2812_conf->led_num * sizeof(*ws2812_conf->led_data));

    ws2812_pwm_send(ws2812_conf);

    return true;
}

void ws2812_pwm_deinit(ws2812_configuration* ws2812_conf) {

    free(ws2812_conf->led_data);
    ws2812_conf->led_data = NULL;
	ws2812_conf->handle = NULL;
	ws2812_conf->led_num = 0;
	ws2812_conf->brightness = 0;
	ws2812_conf->dma = 0;
}


#endif