/*
 * ws2812_spi.c
 *
 *  Created on: Feb 22, 2024
 *      Author: chris
 */


#include "ws2812_spi.h"
#include "string.h"
#include "stdlib.h"

uint8_t ping_pong_data_spi[48]; // Buffer used for sending data in ping pong fashion

// void ws2812_spi_adjust_brightness(ws2812_configuration* ws2812_conf, uint8_t brightness) {
// 	uint8_t send_data[ws2812_conf->led_num * 24];
// 	uint8_t green, red, blue;
    
// 	for (int i = 0; i < ws2812_conf->led_num; i++) {
//         green = ws2812_conf->led_data[i][GREEN] * brightness / 100;
//         red = ws2812_conf->led_data[i][RED] * brightness / 100;
//         blue = ws2812_conf->led_data[i][BLUE] * brightness / 100;

//         for (int j = 0; j < 8; j++) {
// 			int index = i * 24 + j;
// 			send_data[index] = (green & (1 << (7 - j))) ? 0b110 : 0b100;
// 			send_data[index + 8] = (red & (1 << (7 - j))) ? 0b110 : 0b100;
// 			send_data[index + 16] = (blue & (1 << (7 - j))) ? 0b110 : 0b100;
//         }
//     }

//     if (ws2812_conf->dma) {
//         HAL_SPI_Transmit_DMA(ws2812_conf->handle, send_data, sizeof(send_data));
// 		while(!__HAL_DMA_GET_FLAG(&hdma_spi1_tx, DMA_FLAG_TC1)) {};
// 		//while(__HAL_SPI_GET_FLAG(&hspi1, SPI_FLAG_BSY ));
		

//     }
//     else {
//         HAL_SPI_Transmit(ws2812_conf->handle, send_data, sizeof(send_data), HAL_MAX_DELAY);
//     }
// 	ws2812_delay_us(280);
// }

void ws2812_spi_fade(ws2812_configuration* ws2812_conf, uint16_t fade_time_ms) {

  uint8_t (*led_data)[3] = (uint8_t(*)[3])ws2812_conf->led_data;
  uint16_t fade_delay = 0;
  
  /* Do nothing */
  if (fade_time_ms < ws2812_conf->brightness) {
	return;
  }
  
  fade_delay = (fade_time_ms / ws2812_conf->brightness) / 2;
  
  for (int fade = ws2812_conf->brightness; fade >= 0; fade -= 2) {
	for (int i = 0; i < ws2812_conf->led_num; i++) {
		ws2812_spi_data(ws2812_conf, led_data[i][GREEN], led_data[i][RED], led_data[i][BLUE], fade);
	}
	HAL_Delay(fade_delay);
  }
  
  for (int fade = 0; fade < ws2812_conf->brightness; fade += 2) {
	for (int i = 0; i < ws2812_conf->led_num; i++) {
		ws2812_spi_data(ws2812_conf, led_data[i][GREEN], led_data[i][RED], led_data[i][BLUE], fade);
	}
	HAL_Delay(fade_delay);
  }

}


void ws2812_spi_data(ws2812_configuration* ws2812_conf, uint8_t green, uint8_t red, uint8_t blue, uint8_t brightness) {

	green = green * brightness / 100;
	red = red * brightness / 100;
	blue = blue * brightness / 100;

	if (ws2812_conf->dma) {
		uint8_t *send_data = ws2812_conf->ping_pong ? ping_pong_data_spi : ping_pong_data_spi + 24;

		for (int i = 0; i < 8; i++) {
			send_data[i] = (green & (1 << (7 - i))) ? 0b110 : 0b100;
			send_data[i + 8] = (red & (1 << (7 - i))) ? 0b110 : 0b100;
			send_data[i + 16] = (blue & (1 << (7 - i))) ? 0b110 : 0b100;
		}

		while(!transferDone) {}; // Wait for DMA to finish
		HAL_SPI_Transmit_DMA(ws2812_conf->handle, send_data, 24);
		transferDone = 0;
		ws2812_conf->ping_pong = !ws2812_conf->ping_pong;
		
	}
	else {
		uint8_t send_data[24];

		for (int i = 0; i < 8; i++) {
			send_data[i] = (green & (1 << (7 - i))) ? 0b110 : 0b100;
			send_data[i + 8] = (red & (1 << (7 - i))) ? 0b110 : 0b100;
			send_data[i + 16] = (blue & (1 << (7 - i))) ? 0b110 : 0b100;
		}

		HAL_SPI_Transmit(ws2812_conf->handle, send_data, 24, HAL_MAX_DELAY);
	}
}


void ws2812_spi_send_single(ws2812_configuration* ws2812_conf) {
	
	uint8_t (*led_data)[3] = (uint8_t(*)[3])ws2812_conf->led_data;
	if (ws2812_conf->dma) {
		transferDone = 1;
	}

	for (int i = 0; i < ws2812_conf->led_num; i++) {
		ws2812_spi_data(ws2812_conf, led_data[i][GREEN], led_data[i][RED], led_data[i][BLUE], ws2812_conf->brightness);
	}
    ws2812_delay_us(280);
}



void ws2812_spi_send(ws2812_configuration* ws2812_conf) {
	
	uint8_t (*led_data)[3] = (uint8_t(*)[3])ws2812_conf->led_data;
	uint8_t send_data[ws2812_conf->led_num * 24];
	uint8_t green, red, blue;
    
	for (int i = 0; i < ws2812_conf->led_num; i++) {
        green = led_data[i][GREEN] * ws2812_conf->brightness / 100;
        red = led_data[i][RED] * ws2812_conf->brightness / 100;
        blue = led_data[i][BLUE] * ws2812_conf->brightness / 100;

        for (int j = 0; j < 8; j++) {
			int index = i * 24 + j;
			send_data[index] = (green & (1 << (7 - j))) ? 0b110 : 0b100;
			send_data[index + 8] = (red & (1 << (7 - j))) ? 0b110 : 0b100;
			send_data[index + 16] = (blue & (1 << (7 - j))) ? 0b110 : 0b100;
        }
    }

    if (ws2812_conf->dma) {
        HAL_SPI_Transmit_DMA(ws2812_conf->handle, send_data, sizeof(send_data));
		while(!__HAL_DMA_GET_FLAG(&hdma_spi1_tx, DMA_FLAG_TC1)) {};
		// while(__HAL_SPI_GET_FLAG(&hspi1, SPI_FLAG_BSY ));
		

    }
    else {
        HAL_SPI_Transmit(ws2812_conf->handle, send_data, sizeof(send_data), HAL_MAX_DELAY);
    }
	ws2812_delay_us(280);
}


bool ws2812_spi_init(ws2812_configuration* ws2812_conf) {

	// create a buffer for the data using ws2812_configuration
	ws2812_conf->led_data = malloc(sizeof(uint8_t[3]) * ws2812_conf->led_num);
    
	if (ws2812_conf->led_data == NULL) {
        return false;
    }

   	memset(ws2812_conf->led_data, 0, ws2812_conf->led_num * sizeof(*ws2812_conf->led_data));

	// Set all leds to 0
    // ws2812_spi_send(ws2812_conf);
	ws2812_spi_send_single(ws2812_conf);

	return true;
}

void ws2812_spi_clear(ws2812_configuration* ws2812_conf) {
    for (int i = 0; i < ws2812_conf->led_num; i++) {
        ws2812_set_led(ws2812_conf, i, 0, 0, 0);
    }
	// ws2812_spi_send(ws2812_conf);
	ws2812_spi_send_single(ws2812_conf);
}

void ws2812_spi_deinit(ws2812_configuration* ws2812_conf) {

	free(ws2812_conf->led_data);
	ws2812_conf->led_data = NULL;
	ws2812_conf->handle = NULL;
	ws2812_conf->led_num = 0;
	ws2812_conf->brightness = 0;
	ws2812_conf->dma = 0;
}






