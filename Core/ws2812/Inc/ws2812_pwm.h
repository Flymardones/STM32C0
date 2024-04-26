/*
 * ws2812_pwm.h
 *
 *  Created on: Feb 28, 2024
 *      Author: chris
 */

#ifndef WS2812_INC_WS2812_PWM_H_
#define WS2812_INC_WS2812_PWM_H_
#include "ws2812.h"

extern ws2812_configuration ws2812_pwm;

void ws2812_pwm_send(ws2812_configuration* ws2812_conf);

bool ws2812_pwm_init(ws2812_configuration* ws2812_conf);

void ws2812_pwm_send_single(ws2812_configuration* ws2812_conf);

void ws2812_pwm_data(ws2812_configuration* ws2812_conf, uint8_t green, uint8_t red, uint8_t blue, uint8_t brightness);

void ws2812_pwm_fade(ws2812_configuration* ws2812_conf, uint16_t fade_time_ms);

void ws2812_pwm_clear(ws2812_configuration* ws2812_conf);

void ws2812_pwm_deinit(ws2812_configuration* ws2812_conf);

#endif /* WS2812_INC_WS2812_PWM_H_ */
