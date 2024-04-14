/*
 * ws2812_spi.h
 *
 *  Created on: Feb 22, 2024
 *      Author: chris
 */

#ifndef WS2812_INC_WS2812_SPI_H_
#define WS2812_INC_WS2812_SPI_H_

#include "ws2812.h"

extern ws2812_configuration ws2812_spi;

bool ws2812_spi_init(ws2812_configuration* ws2812_conf);

void ws2812_spi_send(ws2812_configuration* ws2812_conf);

void ws2812_spi_data(ws2812_configuration* ws2812_conf, uint8_t green, uint8_t red, uint8_t blue, uint8_t brightness);

void ws2812_spi_fade(ws2812_configuration* ws2812_conf, uint16_t fade_time_ms);

void ws2812_spi_send_single(ws2812_configuration* ws2812_conf);

void ws2812_spi_clear(ws2812_configuration* ws2812_conf);

void ws2812_spi_deinit(ws2812_configuration* ws2812_conf);

#endif /* WS2812_INC_WS2812_SPI_H_ */
