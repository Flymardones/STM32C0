/*
 * ws2812_spi.c
 *
 *  Created on: Feb 22, 2024
 *      Author: chris
 */
#include "ws2812_spi.h"
#include "string.h"
#include "stdlib.h"

#if SPI

uint8_t indx = 0;

// This function will be called when a SPI transmit via DMA is complete
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {

    // Check if this is the SPI peripheral we're interested in
    if (hspi == &hspi1) {
		if (indx < ws2812_spi.led_num) {
			if (fade_flag) {
				ws2812_spi_data(&ws2812_spi, ws2812_spi.led_data[indx][GREEN], ws2812_spi.led_data[indx][RED], ws2812_spi.led_data[indx][BLUE], ws2812_spi.fade);
			}
			else {
				ws2812_spi_data(&ws2812_spi, ws2812_spi.led_data[indx][GREEN], ws2812_spi.led_data[indx][RED], ws2812_spi.led_data[indx][BLUE], ws2812_spi.brightness);
			}
			indx++;
		}
		else if (indx < ws2812_spi.led_num + 5) { // Reset pulse (48 bits * 1.25 us = 60 us * 5 = 300 us)
			for (uint8_t i = 24; i < 48; i++) { // Reset pulse (48 bits * 1.25 us = 60 us * 5 = 300 us)
				ws2812_spi.circBuffer[i] = 0;
			}
			indx++;
		}
		else {
			HAL_SPI_DMAStop(&hspi1);
			transferDone = 1;
			indx = 0;
		}
    }
}

// This function will be called when a SPI transmit via DMA is half complete
void HAL_SPI_TxHalfCpltCallback(SPI_HandleTypeDef *hspi) {

	// Check if this is the SPI peripheral we're interested in
	if (hspi == &hspi1) {
		if (indx < ws2812_spi.led_num) {
			if (fade_flag) {
				ws2812_spi_data(&ws2812_spi, ws2812_spi.led_data[indx][GREEN], ws2812_spi.led_data[indx][RED], ws2812_spi.led_data[indx][BLUE], ws2812_spi.fade);
			}
			else {
				ws2812_spi_data(&ws2812_spi, ws2812_spi.led_data[indx][GREEN], ws2812_spi.led_data[indx][RED], ws2812_spi.led_data[indx][BLUE], ws2812_spi.brightness);
			}
			indx++;
		}
		else if (indx < ws2812_spi.led_num + 5) { // Reset pulse (48 bits * 1.25 us = 60 us * 5 = 300 us)
	        for (uint8_t i = 0; i < 24; i++) {
            	ws2812_spi.circBuffer[i] = 0;
        	}
       	 	indx++;
		}
	}
}


void ws2812_spi_fade(ws2812_configuration* ws2812_conf, uint16_t fade_time_ms) {

  uint16_t fade_delay = 0;
  
  /* Do nothing */
  if (fade_time_ms < ws2812_conf->brightness) {
	return;
  }
  
  fade_delay = (fade_time_ms / ws2812_conf->brightness);
  
  for (ws2812_conf->fade = ws2812_conf->brightness; ws2812_conf->fade >= 0; ws2812_conf->fade -= 2) {
	ws2812_spi_send(ws2812_conf);
	HAL_Delay(fade_delay);
  }
  
  for (ws2812_conf->fade = 0; ws2812_conf->fade < ws2812_conf->brightness; ws2812_conf->fade += 2) {
	ws2812_spi_send(ws2812_conf);
	HAL_Delay(fade_delay);
  }

}

#pragma GCC optimize ("O1")
void ws2812_spi_data(ws2812_configuration* ws2812_conf, uint8_t green, uint8_t red, uint8_t blue, uint8_t brightness) {

	green = green * brightness / 100;
	red = red * brightness / 100;
	blue = blue * brightness / 100;

	if (ws2812_conf->dma) {
		uint8_t *send_data = ws2812_conf->ping_pong ? ws2812_conf->circBuffer : ws2812_conf->circBuffer + 24;

		for (uint8_t i = 0; i < 8; i++) {
			uint8_t mask = 1 << (7 - i);
			send_data[i] = (green & mask) ? 0b110 : 0b100;
			send_data[i + 8] = (red & mask) ? 0b110 : 0b100;
			send_data[i + 16] = (blue & mask) ? 0b110 : 0b100;
		}
		ws2812_conf->ping_pong = !ws2812_conf->ping_pong;
	}
	else {
		uint8_t send_data[24];

		for (uint8_t i = 0; i < 8; i++) {
			uint8_t mask = 1 << (7 - i);
			send_data[i] = (green & mask) ? 0b110 : 0b100;
			send_data[i + 8] = (red & mask) ? 0b110 : 0b100;
			send_data[i + 16] = (blue & mask) ? 0b110 : 0b100;
		}

		HAL_SPI_Transmit(ws2812_conf->handle, send_data, 24, HAL_MAX_DELAY);
	}
}


void ws2812_spi_send(ws2812_configuration* ws2812_conf) {

	if (ws2812_conf->dma) {
		ws2812_conf->ping_pong = true;

		// Initialise data for one transfer and start DMA in circular mode  
		if (fade_flag) {
			for (uint8_t i = 0; i < 2; i++) {
				ws2812_spi_data(ws2812_conf, ws2812_conf->led_data[i][GREEN], ws2812_conf->led_data[i][RED], ws2812_conf->led_data[i][BLUE], ws2812_conf->fade);
			}
		}
		else {
			for (uint8_t i = 0; i < 2; i++) {
				ws2812_spi_data(ws2812_conf, ws2812_conf->led_data[i][GREEN], ws2812_conf->led_data[i][RED], ws2812_conf->led_data[i][BLUE], ws2812_conf->brightness);
			}
		}
		indx = 2;
		transferDone = 0;
		HAL_SPI_Transmit_DMA(ws2812_conf->handle, ws2812_conf->circBuffer, 48);
	}
	else {
		if (fade_flag) {
			for (uint8_t i = 0; i < ws2812_conf->led_num; i++) {
				ws2812_spi_data(ws2812_conf, ws2812_conf->led_data[i][GREEN], ws2812_conf->led_data[i][RED], ws2812_conf->led_data[i][BLUE], ws2812_conf->fade);
			}
		}
		else {
			for (uint8_t i = 0; i < ws2812_conf->led_num; i++) {
				ws2812_spi_data(ws2812_conf, ws2812_conf->led_data[i][GREEN], ws2812_conf->led_data[i][RED], ws2812_conf->led_data[i][BLUE], ws2812_conf->brightness);
			}
		}
		ws2812_delay_us(280);
	}
}

void ws2812_spi_send_burst(ws2812_configuration* ws2812_conf) {
	
	uint8_t (*led_data)[3] = (uint8_t(*)[3])ws2812_conf->led_data;
	uint8_t send_data[ws2812_conf->led_num * 24];
	uint8_t green, red, blue;
    
	for (uint8_t i = 0; i < ws2812_conf->led_num; i++) {
        green = led_data[i][GREEN] * ws2812_conf->brightness / 100;
        red = led_data[i][RED] * ws2812_conf->brightness / 100;
        blue = led_data[i][BLUE] * ws2812_conf->brightness / 100;

        for (uint8_t j = 0; j < 8; j++) {
			int index = i * 24 + j;
			send_data[index] = (green & (1 << (7 - j))) ? 0b110 : 0b100;
			send_data[index + 8] = (red & (1 << (7 - j))) ? 0b110 : 0b100;
			send_data[index + 16] = (blue & (1 << (7 - j))) ? 0b110 : 0b100;
        }
    }

    if (ws2812_conf->dma) { // Only works if DMA is set to normal mode (not circular)
        HAL_SPI_Transmit_DMA(ws2812_conf->handle, send_data, sizeof(send_data));
		while(!__HAL_DMA_GET_FLAG(&hdma_spi1_tx, DMA_FLAG_TC1)) {};
    }
    else {
        HAL_SPI_Transmit(ws2812_conf->handle, send_data, sizeof(send_data), HAL_MAX_DELAY);
    }
	ws2812_delay_us(280);
}


bool ws2812_spi_init(ws2812_configuration* ws2812_conf) {

	// allocate the memory for the leds
	ws2812_conf->led_data = malloc(sizeof(uint8_t[3]) * ws2812_conf->led_num);
    
	if (ws2812_conf->led_data == NULL) {
        return false;
    }

   	memset(ws2812_conf->led_data, 0, ws2812_conf->led_num * sizeof(*ws2812_conf->led_data));

	// Set all leds to 0
	ws2812_spi_send(ws2812_conf);

	return true;
}

void ws2812_spi_clear(ws2812_configuration* ws2812_conf) {
   	memset(ws2812_conf->led_data, 0, ws2812_conf->led_num * sizeof(*ws2812_conf->led_data));
	ws2812_spi_send(ws2812_conf);
}

void ws2812_spi_deinit(ws2812_configuration* ws2812_conf) {

	free(ws2812_conf->led_data);
	ws2812_conf->led_data = NULL;
	ws2812_conf->handle = NULL;
	ws2812_conf->led_num = 0;
	ws2812_conf->brightness = 0;
	ws2812_conf->dma = 0;
}


#endif



