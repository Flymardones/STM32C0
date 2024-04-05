/*
 * ws2812_uart.c
 *
 *  Created on: Feb 28, 2024
 *      Author: chris
 */

#include "ws2812_uart.h"
#include "ws2812_spi.h"
#include "string.h"
#include "stdlib.h"

uint8_t initialized = 0;

void ws2812_uart_commands(uint8_t* data, uint16_t size) {

    char *token;
    char *tokenizedInput[10];
    int i = 0;


    token = strtok(data, ",");
    while (token != NULL) {
        tokenizedInput[i++] = token;
        token = strtok(NULL, ",");
    }



    if (strcmp(tokenizedInput[0], "INIT") == 0) {
        ws2812_spi.handle = &hspi1;
        ws2812_spi.led_num = atoi(tokenizedInput[1]);
        ws2812_spi.brightness = atoi(tokenizedInput[2]);
        ws2812_spi.dma = 1;
    

        ws2812_spi_init(&ws2812_spi);
        initialized = 1;
    }
    else if (strcmp(tokenizedInput[0], "DEINIT") == 0) {
        for (int i = 0; i < ws2812_spi.led_num; i++) {
            ws2812_set_led(&ws2812_spi, i, 0, 0, 0);
        }
        ws2812_spi_send(&ws2812_spi);
        ws2812_spi_deinit(&ws2812_spi);
        initialized = 0;
    }
    else if (strcmp(tokenizedInput[0], "SET_ALL") == 0 && initialized) {
        for (int i = 0; i < ws2812_spi.led_num; i++) {
            ws2812_set_led(&ws2812_spi, i, atoi(tokenizedInput[1]), atoi(tokenizedInput[2]), atoi(tokenizedInput[3]));
        }
        ws2812_spi_send(&ws2812_spi);
    }
    else if (strcmp(tokenizedInput[0], "SET_SINGLE") == 0 && initialized) {
        ws2812_set_led(&ws2812_spi, atoi(tokenizedInput[1]), atoi(tokenizedInput[2]), atoi(tokenizedInput[3]), atoi(tokenizedInput[4]));
        ws2812_spi_send(&ws2812_spi);
    }
    else if (strcmp(tokenizedInput[0], "SET_BRIGHTNESS") == 0 && initialized) {
        ws2812_spi.brightness = atoi(tokenizedInput[1]);
        ws2812_spi_send(&ws2812_spi);
    }
    else if (strcmp(tokenizedInput[0], "CLEAR_ALL") == 0 && initialized) {
        for (int i = 0; i < ws2812_spi.led_num; i++) {
            ws2812_set_led(&ws2812_spi, i, 0, 0, 0);
        }
        ws2812_spi_send(&ws2812_spi);
    }
    else if (strcmp(tokenizedInput[0], "CLEAR_SINGLE") == 0 && initialized) {
        ws2812_set_led(&ws2812_spi, atoi(tokenizedInput[1]), 0, 0, 0);
        ws2812_spi_send(&ws2812_spi);
    }

    token = NULL;
    memset(rxBuff, 0, RX_BUFF_SIZE);
}