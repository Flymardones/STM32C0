/*
 * ws2812_uart.h
 *
 *  Created on: Feb 28, 2024
 *      Author: chris
 */

#ifndef WS2812_INC_WS2812_UART_H_
#define WS2812_INC_WS2812_UART_H_

#include "ws2812.h"

void ws2812_uart_init(ws2812_configuration* ws2812_conf);

void ws2812_uart_send(ws2812_configuration* ws2812_conf);


#endif /* WS2812_INC_WS2812_UART_H_ */
