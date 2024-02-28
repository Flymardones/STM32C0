/*
 * ws2812_spi.c
 *
 *  Created on: Feb 22, 2024
 *      Author: chris
 */


#include "ws2812_spi.h"
#include "string.h"
#include "stdlib.h"

#pragma GCC optimize ("O3")
void ws2812_spi_data(ws2812_configuration* ws2812_conf, uint8_t green, uint8_t red, uint8_t blue) {

	green = green * ws2812_conf->brightness / 100;
	red = red * ws2812_conf->brightness / 100;
	blue = blue * ws2812_conf->brightness / 100;

	uint8_t send_data[24];

	for (int i = 0; i < 8; i++) {
		send_data[i] = (green & (1 << i)) ? 0b110 : 0b100;
		send_data[i + 8] = (red & (1 << i)) ? 0b110 : 0b100;
		send_data[i + 16] = (blue & (1 << i)) ? 0b110 : 0b100;
	}

	if (ws2812_conf->dma) {
		HAL_SPI_Transmit_DMA(ws2812_conf->handle, send_data, sizeof(send_data));
	}
	else {
		HAL_SPI_Transmit(ws2812_conf->handle, send_data, sizeof(send_data), HAL_MAX_DELAY);
	}
}

#pragma GCC optimize ("O3")
void ws2812_spi_send_single(ws2812_configuration* ws2812_conf) {
	uint8_t (*led_data)[3] = (uint8_t(*)[3])ws2812_conf->buffer;

	for (int i = 0; i < ws2812_conf->led_num; i++) {
		ws2812_spi_data(ws2812_conf, led_data[i][0],led_data[i][1],led_data[i][2]);
	}
    ws2812_delay_us(240);
}


#pragma GCC optimize ("O3")
void ws2812_spi_send(ws2812_configuration* ws2812_conf) {
	uint8_t (*led_data)[3] = (uint8_t(*)[3])ws2812_conf->buffer;
	uint8_t send_data[ws2812_conf->led_num * 24];
    
	for (int i = 0; i < ws2812_conf->led_num; i++) {
        uint8_t green = led_data[i][0] * ws2812_conf->brightness / 100;
        uint8_t red = led_data[i][1] * ws2812_conf->brightness / 100;
        uint8_t blue = led_data[i][2] * ws2812_conf->brightness / 100;

        for (int j = 0; j < 8; j++) {
			send_data[i * 24 + j] = (green & (1 << j)) ? 0b110 : 0b100;
			send_data[i * 24 + j + 8] = (red & (1 << j)) ? 0b110 : 0b100;
			send_data[i * 24 + j + 16] = (blue & (1 << j)) ? 0b110 : 0b100;
        }
    }

    if (ws2812_conf->dma) {
        HAL_SPI_Transmit_DMA(ws2812_conf->handle, send_data, sizeof(send_data));
		// ws2812_delay_us(300);
    }
    else {
        HAL_SPI_Transmit(ws2812_conf->handle, send_data, sizeof(send_data), HAL_MAX_DELAY);
		ws2812_delay_us(240);
    }
	
}


void ws2812_spi_init(ws2812_configuration* ws2812_conf) {

	// create a buffer for the data using ws2812_configuration
	uint8_t (*led_data)[3] = malloc(ws2812_conf->led_num * sizeof(*led_data));

	ws2812_conf->buffer = led_data;
   	memset(ws2812_conf->buffer, 0, ws2812_conf->led_num * sizeof(*led_data));

	// Set all leds to 0
    ws2812_spi_send(ws2812_conf);
}

void ws2812_spi_deinit(ws2812_configuration* ws2812_conf) {

	free(ws2812_conf->buffer);
	ws2812_conf->buffer = NULL;
}






