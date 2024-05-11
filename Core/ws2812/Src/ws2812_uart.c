/*
 * ws2812_uart.c
 *
 *  Created on: Feb 28, 2024
 *      Author: chris
 */

#include "main.h"
#include "ws2812_uart.h"
#include "string.h"
#include "stdlib.h"

#if SPI
#include "ws2812_spi.h"
#endif


#if PWM
#include "ws2812_pwm.h"
#endif

#if GPIO
#include "ws2812_gpio.h"
#endif

#define MAX_COMMAND_SIZE 10


uint8_t initialized = 0;

bool valid_command_size(int commandSize, int expectedSize) {
    if (commandSize < expectedSize) {
        memset(rxBuff, 0, RX_BUFF_SIZE);
        return false;
    }
    return true;
}

void ws2812_uart_commands(uint8_t* data, uint16_t size) {

    char *token;
    char *tokenizedInput[MAX_COMMAND_SIZE];
    int commandSize = 0;


    token = strtok((char*)data, ",");
    while (token != NULL) {
        if (commandSize >= MAX_COMMAND_SIZE) {
            return;
        }
        tokenizedInput[commandSize++] = token;
        token = strtok(NULL, ",");
    }

    if (strcmp(tokenizedInput[0], "INIT") == 0) {
        if (!valid_command_size(commandSize, 3)) {
            return;
        }
        #if SPI
        if (initialized) {
            ws2812_spi_deinit(&ws2812_spi);
            initialized = 0;
            fade_flag = 0;
            fade_time = 0;
        }
        ws2812_spi.handle = &hspi1;
        ws2812_spi.led_num = (uint8_t)atoi(tokenizedInput[1]);
        ws2812_spi.brightness = (uint8_t)atoi(tokenizedInput[2]);
        ws2812_spi.dma = 1;
        ws2812_spi.ping_pong = true;
    
        if (ws2812_spi_init(&ws2812_spi)) {
            initialized = 1;
        }
        #endif

        #if PWM
        if (initialized) {
            ws2812_pwm_deinit(&ws2812_pwm);
            initialized = 0;
            fade_flag = 0;
            fade_time = 0;
        }
        ws2812_pwm.handle = &htim1;
        ws2812_pwm.led_num = (uint8_t)atoi(tokenizedInput[1]);
        ws2812_pwm.brightness = (uint8_t)atoi(tokenizedInput[2]);
        ws2812_pwm.dma = 1;
        ws2812_pwm.ping_pong = true;


        if(ws2812_pwm_init(&ws2812_pwm)) {
            initialized = 1;
        }
        #endif

        #if GPIO
        if (initialized) {
            ws2812_gpio_deinit(&ws2812_gpio);
            initialized = 0;
            fade_flag = 0;
            fade_time = 0;
        }
        ws2812_gpio.handle = (void*)GPIO_PIN_9;
        ws2812_gpio.led_num = (uint8_t)atoi(tokenizedInput[1]);
        ws2812_gpio.brightness = (uint8_t)atoi(tokenizedInput[2]);

        if (ws2812_gpio_init(&ws2812_gpio)) {
            initialized = 1;
        }
        #endif

    }
    else if (strcmp(tokenizedInput[0], "DEINIT") == 0) {
        #if SPI
        ws2812_spi_clear(&ws2812_spi);
        while(!transferDone){}; // Wait for ongoing transfer to finish
        ws2812_spi_deinit(&ws2812_spi);
        #endif

        #if PWM
        ws2812_pwm_clear(&ws2812_pwm);
        while(!transferDone){}; // Wait for ongoing transfer to finish
        ws2812_pwm_deinit(&ws2812_pwm);
        #endif

        #if GPIO
        ws2812_gpio_clear(&ws2812_gpio);
        ws2812_gpio_deinit(&ws2812_gpio);
        #endif
        initialized = 0;
        fade_flag = 0;
    }
    else if (strcmp(tokenizedInput[0], "SET_ALL") == 0 && initialized) {
        if (!valid_command_size(commandSize, 4)) {
            return;
        }
        #if SPI
        for (int i = 0; i < ws2812_spi.led_num; i++) {
            ws2812_set_led(&ws2812_spi, i, (uint8_t)atoi(tokenizedInput[1]), (uint8_t)atoi(tokenizedInput[2]), (uint8_t)atoi(tokenizedInput[3]));
        }
        ws2812_spi_send(&ws2812_spi);
        #endif

        #if PWM
        for (uint8_t i = 0; i < ws2812_pwm.led_num; i++) {
            ws2812_set_led(&ws2812_pwm, i, (uint8_t)atoi(tokenizedInput[1]), (uint8_t)atoi(tokenizedInput[2]), (uint8_t)atoi(tokenizedInput[3]));
        }
        ws2812_pwm_send(&ws2812_pwm);
        #endif

        #if GPIO
        for (uint8_t i = 0; i < ws2812_gpio.led_num; i++) {
            ws2812_set_led(&ws2812_gpio, i, (uint8_t)atoi(tokenizedInput[1]), (uint8_t)atoi(tokenizedInput[2]), (uint8_t)atoi(tokenizedInput[3]));
        }
        ws2812_gpio_send(&ws2812_gpio);
        #endif

    }
    else if (strcmp(tokenizedInput[0], "SET_SINGLE") == 0 && initialized) {
        if (!valid_command_size(commandSize, 5)) {
            return;
        }

        #if SPI
        ws2812_set_led(&ws2812_spi, (uint8_t)atoi(tokenizedInput[1]), (uint8_t)atoi(tokenizedInput[2]), (uint8_t)atoi(tokenizedInput[3]), (uint8_t)atoi(tokenizedInput[4]));
        ws2812_spi_send(&ws2812_spi);
        #endif

        #if PWM
        ws2812_set_led(&ws2812_pwm, (uint8_t)atoi(tokenizedInput[1]), (uint8_t)atoi(tokenizedInput[2]), (uint8_t)atoi(tokenizedInput[3]), (uint8_t)atoi(tokenizedInput[4]));
        ws2812_pwm_send(&ws2812_pwm);
        #endif

        #if GPIO
        ws2812_set_led(&ws2812_gpio, (uint8_t)atoi(tokenizedInput[1]), (uint8_t)atoi(tokenizedInput[2]), (uint8_t)atoi(tokenizedInput[3]), (uint8_t)atoi(tokenizedInput[4]));
        ws2812_gpio_send(&ws2812_gpio);
        #endif

    }
    else if (strcmp(tokenizedInput[0], "SET_BRIGHTNESS") == 0 && initialized) {
        if (!valid_command_size(commandSize, 2)) {
            return;
        }

        #if SPI
        ws2812_spi.brightness = (uint8_t)atoi(tokenizedInput[1]);
        ws2812_spi_send(&ws2812_spi);
        #endif

        #if PWM
        ws2812_pwm.brightness = (uint8_t)atoi(tokenizedInput[1]);
        ws2812_pwm_send(&ws2812_pwm);
        #endif

        #if GPIO
        ws2812_gpio.brightness = (uint8_t)atoi(tokenizedInput[1]);
        ws2812_gpio_send(&ws2812_gpio);
        #endif
    }
    else if (strcmp(tokenizedInput[0], "CLEAR_ALL") == 0 && initialized) {
        #if SPI
        ws2812_spi_clear(&ws2812_spi);
        #endif

        #if PWM
        ws2812_pwm_clear(&ws2812_pwm);
        #endif

        #if GPIO
        ws2812_gpio_clear(&ws2812_gpio);
        #endif
    }
    else if (strcmp(tokenizedInput[0], "CLEAR_SINGLE") == 0 && initialized) {
        if (!valid_command_size(commandSize, 2)) {
            return;
        }

        #if SPI
        ws2812_set_led(&ws2812_spi, (uint8_t)atoi(tokenizedInput[1]), 0, 0, 0);
        ws2812_spi_send(&ws2812_spi);
        #endif

        #if PWM
        ws2812_set_led(&ws2812_pwm, (uint8_t)atoi(tokenizedInput[1]), 0, 0, 0);
        ws2812_pwm_send(&ws2812_pwm);
        #endif

        #if GPIO
        ws2812_set_led(&ws2812_gpio, (uint8_t)atoi(tokenizedInput[1]), 0, 0, 0);
        ws2812_gpio_send(&ws2812_gpio);
        #endif
    }
    else if (strcmp(tokenizedInput[0], "START_FADE_ALL") == 0 && initialized) {
        if (!valid_command_size(commandSize, 5)) {
            return;
        }
        #if SPI
        for (uint8_t i = 0; i < ws2812_spi.led_num; i++) {
            ws2812_set_led(&ws2812_spi, i, (uint8_t)atoi(tokenizedInput[1]), (uint8_t)atoi(tokenizedInput[2]), (uint8_t)atoi(tokenizedInput[3]));
        }
        #endif
	
        #if PWM
        for (uint8_t i = 0; i < ws2812_pwm.led_num; i++) {
            ws2812_set_led(&ws2812_pwm, i, (uint8_t)atoi(tokenizedInput[1]), (uint8_t)atoi(tokenizedInput[2]), (uint8_t)atoi(tokenizedInput[3]));
        }
        #endif

        #if GPIO
        for (uint8_t i = 0; i < ws2812_gpio.led_num; i++) {
            ws2812_set_led(&ws2812_gpio, i, (uint8_t)atoi(tokenizedInput[1]), (uint8_t)atoi(tokenizedInput[2]), (uint8_t)atoi(tokenizedInput[3]));
        }
        #endif
	
        fade_time = (uint16_t)atoi(tokenizedInput[4]);
        fade_flag = 1;
    }
    else if (strcmp(tokenizedInput[0], "STOP_FADE_ALL") == 0 && initialized) {
        fade_flag = 0;
    }
    else if (strcmp(tokenizedInput[0], "START_FADE_SINGLE") == 0 && initialized) {
        if (!valid_command_size(commandSize, 6)) {
            return;
        }
        #if SPI
        ws2812_set_led(&ws2812_spi, (uint8_t)atoi(tokenizedInput[1]), (uint8_t)atoi(tokenizedInput[2]), (uint8_t)atoi(tokenizedInput[3]), (uint8_t)atoi(tokenizedInput[4]));
        #endif
        
        #if PWM
        ws2812_set_led(&ws2812_pwm, (uint8_t)atoi(tokenizedInput[1]), (uint8_t)atoi(tokenizedInput[2]), (uint8_t)atoi(tokenizedInput[3]), (uint8_t)atoi(tokenizedInput[4]));
        #endif

        #if GPIO
        ws2812_set_led(&ws2812_gpio, (uint8_t)atoi(tokenizedInput[1]), (uint8_t)atoi(tokenizedInput[2]), (uint8_t)atoi(tokenizedInput[3]), (uint8_t)atoi(tokenizedInput[4]));
        #endif
        
        fade_time = (uint16_t)atoi(tokenizedInput[5]);
        fade_flag = 1;
    }
    else if (strcmp(tokenizedInput[0], "STOP_FADE_SINGLE") == 0 && initialized) {
        if (!valid_command_size(commandSize, 2)) {
            return;
        }
        #if SPI
        ws2812_set_led(&ws2812_spi, (uint8_t)atoi(tokenizedInput[1]), 0, 0, 0);
        #endif
        
        #if PWM
        ws2812_set_led(&ws2812_pwm, (uint8_t)atoi(tokenizedInput[1]), 0, 0, 0);
        #endif

        #if GPIO
        ws2812_set_led(&ws2812_gpio, (uint8_t)atoi(tokenizedInput[1]), 0, 0, 0);
        #endif
    }

    token = NULL;
    memset(rxBuff, 0, RX_BUFF_SIZE);
}
