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

void ws2812_spi_init(ws2812_configuration* ws2812_conf);

void ws2812_spi_send(ws2812_configuration* ws2812_conf);

void ws2812_spi_send_single(ws2812_configuration* ws2812_conf);

void ws2812_spi_deinit(ws2812_configuration* ws2812_conf);

#endif /* WS2812_INC_WS2812_SPI_H_ */
