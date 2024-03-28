/*
 * ws2812_pwm.h
 *
 *  Created on: Feb 28, 2024
 *      Author: chris
 */

#ifndef WS2812_INC_WS2812_PWM_H_
#define WS2812_INC_WS2812_PWM_H_
#include "ws2812.h"


void ws2812_pwm_send(ws2812_configuration* ws2812_conf);

void ws2812_pwm_init(ws2812_configuration* ws2812_conf);

void ws2812_pwm_send_single(ws2812_configuration* ws2812_conf);

void ws2812_pwm_data(ws2812_configuration* ws2812_conf, uint8_t green, uint8_t red, uint8_t blue);

#endif /* WS2812_INC_WS2812_PWM_H_ */
