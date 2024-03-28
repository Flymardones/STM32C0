/*
 * ws2812_pwm.c
 *
 *  Created on: Feb 28, 2024
 *      Author: chris
 */


#include "ws2812_pwm.h"
#include "string.h"
#include "stdlib.h"




//#pragma GCC optimize ("O3")
void ws2812_pwm_data(ws2812_configuration* ws2812_conf, uint8_t green, uint8_t red, uint8_t blue) {

	green = green * ws2812_conf->brightness / 100;
	red = red * ws2812_conf->brightness / 100;
	blue = blue * ws2812_conf->brightness / 100;

	uint16_t send_data[24];

    // if (ws2812_conf->dma) {
        for (int i = 0; i < 8; i++) {
            send_data[i] = (green & (1 << i)) ? 36 : 16;
            send_data[i + 8] = (red & (1 << i)) ? 36 : 16;
            send_data[i + 16] = (blue & (1 << i)) ? 36 : 16;
        }
    // }
    // else {
    //     for (int i = 0; i < 8; i++) {
    //         send_data[i] = (green & (1 << i)) ? 24 : 8;
    //         send_data[i + 8] = (red & (1 << i)) ? 24 : 8;
    //         send_data[i + 16] = (blue & (1 << i)) ? 24 : 8;
    //     }
    // }


	if (ws2812_conf->dma) {
        HAL_TIM_PWM_Start_DMA(ws2812_conf->handle, TIM_CHANNEL_1, (uint32_t *) send_data, sizeof(send_data));
        while(!datasentflag){};
        datasentflag = 0;
	}
	else {
        TIM1->CR1 |= TIM_CR1_CEN;

        for (int i = 0; i < 24; i++) {

           
            while(!(TIM1->SR & TIM_SR_UIF)) {};
            
            TIM1->CCR1 = send_data[i];
            TIM1->SR &= ~TIM_SR_UIF;




        }
        TIM1->CCR1 = 0;
        TIM1->CR1 &= ~TIM_CR1_CEN;
	}
}

void ws2812_pwm_send_single(ws2812_configuration* ws2812_conf) {
    uint8_t (*led_data)[3] = (uint8_t(*)[3])ws2812_conf->buffer;

    for (int i = 0; i < ws2812_conf->led_num; i++) {
        ws2812_pwm_data(ws2812_conf, led_data[i][0],led_data[i][1],led_data[i][2]);
    }

    //__HAL_TIM_DISABLE(&htim1);
    ws2812_delay_us(280);
}

//#pragma GCC optimize ("O3")
void ws2812_pwm_send(ws2812_configuration* ws2812_conf) {

    uint8_t (*led_data)[3] = (uint8_t(*)[3])ws2812_conf->buffer;
    uint16_t send_data[ws2812_conf->led_num * 24];
    uint8_t green, red, blue;


   	for (int i = 0; i < ws2812_conf->led_num; i++) {
        green = led_data[i][0] * ws2812_conf->brightness / 100;
        red = led_data[i][1] * ws2812_conf->brightness / 100;
        blue = led_data[i][2] * ws2812_conf->brightness / 100;

        for (int j = 0; j < 8; j++) {
            int index = i * 24 + j;
			send_data[index] = (green & (1 << j)) ? 36 : 16;
			send_data[index + 8] = (red & (1 << j)) ? 36 : 16;
			send_data[index + 16] = (blue & (1 << j)) ? 36 : 16;
        }
    }

    if (ws2812_conf->dma) {
        HAL_TIM_PWM_Start_DMA(ws2812_conf->handle, TIM_CHANNEL_1, (uint32_t *) send_data, sizeof(send_data));
        while(!datasentflag){};
        datasentflag = 0;
    }
    else {
        //TIM1->CCR1 = (uint32_t)send_data[0];
        TIM1->CR1 |= TIM_CR1_CEN;

        for (int i = 0; i < (ws2812_conf->led_num * 24); i++) {
            
            while(!(TIM1->SR & TIM_SR_UIF)) {};
            TIM1->CCR1 = send_data[i];
            TIM1->SR &= ~TIM_SR_UIF;
            

            //TIM1->CR1 |= TIM_CR1_CEN;
            //IM1->CCR1 = (uint32_t)send_data[i];
            //TIM1->CR1 |= TIM_CR1_CEN;

        }
        TIM1->CCR1 = 0;
        TIM1->CR1 &= ~TIM_CR1_CEN;
        
    }
    ws2812_delay_us(280);

}




void ws2812_pwm_init(ws2812_configuration* ws2812_conf) {

    uint8_t (*led_data)[3] = malloc(ws2812_conf->led_num * sizeof(*led_data));
    
    ws2812_conf->buffer = led_data;
    memset(ws2812_conf->buffer, 0, ws2812_conf->led_num * sizeof(*led_data));

    ws2812_pwm_send(ws2812_conf);
}