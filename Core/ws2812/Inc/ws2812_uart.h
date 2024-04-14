/*
 * ws2812_uart.h
 *
 *  Created on: Feb 28, 2024
 *      Author: chris
 */

#ifndef WS2812_INC_WS2812_UART_H_
#define WS2812_INC_WS2812_UART_H_

#include "ws2812.h"

typedef struct {
    bool dataReceived;
    uint16_t dataSize;
} uart_data;

void ws2812_uart_commands(uint8_t* data, uint16_t size);


#endif /* WS2812_INC_WS2812_UART_H_ */
