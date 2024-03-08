/*
 * ws2812_uart.c
 *
 *  Created on: Feb 28, 2024
 *      Author: chris
 */

#include "ws2812_uart.h"
#include "string.h"
#include "stdlib.h"



void ws2812_uart_init(ws2812_configuration* ws2812_conf) {
	// create a buffer for the data using ws2812_configuration
	uint8_t (*led_data)[3] = malloc(ws2812_conf->led_num * sizeof(*led_data));

	ws2812_conf->buffer = led_data;
   	memset(ws2812_conf->buffer, 0, ws2812_conf->led_num * sizeof(*led_data));

	// Set all leds to 0
    ws2812_uart_send(ws2812_conf);
}

void ws2812_uart_send(ws2812_configuration* ws2812_conf) {
    uint8_t (*led_data)[3] = (uint8_t(*)[3])ws2812_conf->buffer;
    uint8_t send_data[ws2812_conf->led_num * 24];
    uint8_t green, red, blue;


   	for (int i = 0; i < ws2812_conf->led_num; i++) {
        green = led_data[i][0] * ws2812_conf->brightness / 100;
        red = led_data[i][1] * ws2812_conf->brightness / 100;
        blue = led_data[i][2] * ws2812_conf->brightness / 100;

        for (int j = 0; j < 8; j++) {
			send_data[i * 24 + j] = (green & (1 << j)) ? 0b110: 0b100;
			send_data[i * 24 + j + 8] = (red & (1 << j)) ? 0b110 : 0b100;
			send_data[i * 24 + j + 16] = (blue & (1 << j)) ? 0b110 : 0b100;
        }
    }

    if (ws2812_conf->dma) {
        HAL_UART_Transmit_DMA(ws2812_conf->handle, send_data, sizeof(send_data));
    }
    else {
        HAL_UART_Transmit(ws2812_conf->handle, send_data, sizeof(send_data), HAL_MAX_DELAY);
    }
}