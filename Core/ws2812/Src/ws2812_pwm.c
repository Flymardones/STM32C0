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
void ws2812_pwm_fade(ws2812_configuration* ws2812_conf, uint16_t fade_time_ms) {

  uint16_t fade_delay = 0;
  uint8_t (*led_data)[3] = (uint8_t(*)[3])ws2812_conf->buffer;
  
  /* Do nothing */
  if (fade_time_ms < ws2812_conf->brightness) {
	return;
  }
  
  fade_delay = (fade_time_ms / ws2812_conf->brightness) / 2;
  
  for (int fade = ws2812_conf->brightness; fade >= 0; fade--) {
	// for (int i = 0; i < ws2812_conf->led_num; i++) {
	//   ws2812_pwm_data(ws2812_conf, led_data[i][0], led_data[i][1], led_data[i][2], (uint8_t)fade);
	// }
    ws2812_pwm_send(ws2812_conf, fade);
	HAL_Delay(fade_delay);
  }
  
  for (int fade = 0; fade < ws2812_conf->brightness; fade++) {
	// for (int i = 0; i < ws2812_conf->led_num; i++) {
	//   ws2812_pwm_data(ws2812_conf, led_data[i][0] , led_data[i][1], led_data[i][2], (uint8_t)fade);
	// }
    ws2812_pwm_send(ws2812_conf, fade);
	HAL_Delay(fade_delay);
  }

}

#pragma GCC optimize ("O3")
void ws2812_pwm_data(ws2812_configuration* ws2812_conf, uint8_t green, uint8_t red, uint8_t blue, uint8_t brightness) {

	green = green * brightness / 100;
	red = red * brightness / 100;
	blue = blue * brightness / 100;

	uint16_t send_data[24];

    if (ws2812_conf->dma) {
        for (int i = 0; i < 8; i++) {
            send_data[i] = (green & (1 << (7 - i))) ? 36 : 16;
            send_data[i + 8] = (red & (1 << (7 - i))) ? 36 : 16;
            send_data[i + 16] = (blue & (1 << (7 - i))) ? 36 : 16;
        }
    }
    else {
        for (int i = 0; i < 8; i++) {
            send_data[i] = (green & (1 << (7 - i))) ? 20 : 44;
            send_data[i + 8] = (red & (1 << (7 - i))) ? 20 : 44;
            send_data[i + 16] = (blue & (1 << (7 - i))) ? 20 : 44;
        }
     }


	if (ws2812_conf->dma) {
        HAL_TIM_PWM_Start_DMA(ws2812_conf->handle, TIM_CHANNEL_1, (uint32_t *) send_data, sizeof(send_data));
        while(!datasentflag){};
        datasentflag = 0;
	}
	else {
        for (int i = 0; i < 24; i++) {
            TIM1->CCR1 = send_data[i];
            TIM1->CR1 |= TIM_CR1_CEN;
            while(!(TIM1->SR & TIM_SR_CC1IF)) {}; // Wait for the update event
            TIM1->SR &= ~TIM_SR_CC1IF; // Clear the update event flag
        }
	}
}

void ws2812_pwm_send_single(ws2812_configuration* ws2812_conf) {
    uint8_t (*led_data)[3] = (uint8_t(*)[3])ws2812_conf->buffer;

    for (int i = 0; i < ws2812_conf->led_num; i++) {
        ws2812_pwm_data(ws2812_conf, led_data[i][0],led_data[i][1],led_data[i][2], ws2812_conf->brightness);
    }

    ws2812_delay_us(280);
}

#pragma GCC optimize ("O3")
void ws2812_pwm_send(ws2812_configuration* ws2812_conf, uint8_t brightness) {

    uint8_t (*led_data)[3] = (uint8_t(*)[3])ws2812_conf->buffer;
    uint16_t send_data[ws2812_conf->led_num * 24];
    uint8_t green, red, blue;


   	for (int i = 0; i < ws2812_conf->led_num; i++) {
        green = led_data[i][0] * brightness / 100;
        red = led_data[i][1] * brightness / 100;
        blue = led_data[i][2] * brightness / 100;

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
        HAL_TIM_PWM_Start_DMA(ws2812_conf->handle, TIM_CHANNEL_1, (uint32_t *) send_data, sizeof(send_data));
        //ws2812_delay_us(1000);
        // Wait for dma to finish
        //while(HAL_DMA_GetState(htim1.hdma[TIM_DMA_ID_CC1]) != HAL_DMA_STATE_READY);
        while(!datasentflag){};
        datasentflag = 0;
    }
    else {
        for (int i = 0; i < (ws2812_conf->led_num * 24); i++) {
            TIM1->CCR1 = send_data[i];
            TIM1->CR1 |= TIM_CR1_CEN;
            while(!(TIM1->SR & TIM_SR_UIF)) {}; // Wait for the update event
            TIM1->SR &= ~TIM_SR_UIF; // Clear the update event flag
        }
    }
    ws2812_delay_us(280);


}

void ws2812_pwm_clear(ws2812_configuration* ws2812_conf) {
    for (int i = 0; i < ws2812_conf->led_num; i++) {
        ws2812_set_led(ws2812_conf, i, 0, 0, 0);
    }
    ws2812_pwm_send(ws2812_conf, ws2812_conf->brightness);
}


bool ws2812_pwm_init(ws2812_configuration* ws2812_conf) {

    uint8_t (*led_data)[3] = malloc(ws2812_conf->led_num * sizeof(*led_data));

    if (led_data == NULL) {
        return false;
    }
    
    ws2812_conf->buffer = led_data;
    memset(ws2812_conf->buffer, 0, ws2812_conf->led_num * sizeof(*led_data));

    ws2812_pwm_send(ws2812_conf, ws2812_conf->brightness);

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