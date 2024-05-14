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


bool initialized = false;
bool front_initialized = false;
bool back_initialized = false;



bool valid_command_size(int commandSize, int expectedSize) {
    if (commandSize < expectedSize) {
        memset(rxBuff, 0, RX_BUFF_SIZE);
        return false;
    }
    return true;
}

#pragma GCC optimize ("Os") 
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
        ws2812_spi.ping_pong = true;
    
        if (ws2812_spi_init(&ws2812_spi)) {
            initialized = true;
        }
        #endif

        #if PWM
        if (front_initialized && back_initialized) {
            ws2812_pwm_deinit(&ws2812_pwm_front);
            ws2812_pwm_deinit(&ws2812_pwm_back);
            front_initialized = false;
            back_initialized = false;
            fade_front = false;
            fade_back = false;
            fade_time = 0;
        }
        else if (front_initialized) {
            ws2812_pwm_deinit(&ws2812_pwm_front);
            front_initialized = false;
            fade_front = false;
            fade_time = 0;
        }
        else if (back_initialized) {
            ws2812_pwm_deinit(&ws2812_pwm_back);
            back_initialized = false;
            fade_back = false;
            fade_time = 0;
        }

        ws2812_pwm_front.handle = &htim1;
        ws2812_pwm_front.led_num = (uint8_t)atoi(tokenizedInput[1]);
        ws2812_pwm_front.brightness = (uint8_t)atoi(tokenizedInput[2]);
        
        ws2812_pwm_back.handle = &htim3;
        ws2812_pwm_back.led_num = (uint8_t)atoi(tokenizedInput[1]);
        ws2812_pwm_back.brightness = (uint8_t)atoi(tokenizedInput[2]);
       

        #if DMA
        ws2812_pwm_front.ping_pong = true;
        ws2812_pwm_back.ping_pong = true;
        send_both = true;
        #endif

        if(ws2812_pwm_init(&ws2812_pwm_front)) {
            front_initialized = true;
        }

        if(ws2812_pwm_init(&ws2812_pwm_back)) {
            back_initialized = true;
        }
        
        #if DMA
        ws2812_pwm_send(&ws2812_pwm_front);
        #endif

        #endif

        #if GPIO
        if (initialized) {
            ws2812_gpio_deinit(&ws2812_gpio);
            initialized = false;
            fade_flag = 0;
            fade_time = 0;
        }
        ws2812_gpio.handle = (void*)GPIO_PIN_9;
        ws2812_gpio.led_num = (uint8_t)atoi(tokenizedInput[1]);
        ws2812_gpio.brightness = (uint8_t)atoi(tokenizedInput[2]);

        if (ws2812_gpio_init(&ws2812_gpio)) {
            initialized = true;
        }
        #endif

    }
    else if (strcmp(tokenizedInput[0], "INIT_FRONT") == 0) {
        #if PWM
        if (!valid_command_size(commandSize, 3)) {
            return;
        }

        if (front_initialized) {
            ws2812_pwm_deinit(&ws2812_pwm_front);
            front_initialized = false;
            fade_front = false;
            fade_time = 0;
        }

        ws2812_pwm_front.handle = &htim1;
        ws2812_pwm_front.led_num = (uint8_t)atoi(tokenizedInput[1]);
        ws2812_pwm_front.brightness = (uint8_t)atoi(tokenizedInput[2]);
        ws2812_pwm_front.ping_pong = true;

        if (ws2812_pwm_init(&ws2812_pwm_front)) {
            front_initialized = true;
        }
        #endif

    }
    else if (strcmp(tokenizedInput[0], "INIT_BACK") == 0) {
        #if PWM
        if (!valid_command_size(commandSize, 3)) {
            return;
        }
        if (back_initialized) {
            ws2812_pwm_deinit(&ws2812_pwm_back);
            back_initialized = false;
            fade_back = false;
            fade_time = 0;
        }

        ws2812_pwm_back.handle = &htim3;
        ws2812_pwm_back.led_num = (uint8_t)atoi(tokenizedInput[1]);
        ws2812_pwm_back.brightness = (uint8_t)atoi(tokenizedInput[2]);
        ws2812_pwm_back.ping_pong = true;

        if (ws2812_pwm_init(&ws2812_pwm_back)) {
            back_initialized = true;
        }
        #endif

    }
    else if (strcmp(tokenizedInput[0], "DEINIT") == 0 && (initialized || (front_initialized && back_initialized))) {
        #if SPI
        ws2812_spi_clear(&ws2812_spi);
        while(!transferDone){}; // Wait for ongoing transfer to finish
        ws2812_spi_deinit(&ws2812_spi);
        initialized = false;
        fade_flag = 0;
        #endif

        #if PWM

        #if DMA
        send_both = true;
        ws2812_pwm_clear(&ws2812_pwm_front);
        #else
        ws2812_pwm_clear(&ws2812_pwm_front);
        ws2812_pwm_clear(&ws2812_pwm_back);
        #endif

        while(!transferDone){}; // Wait for ongoing transfer to finish
        ws2812_pwm_deinit(&ws2812_pwm_front);
        ws2812_pwm_deinit(&ws2812_pwm_back);
        front_initialized = false;
        back_initialized = false;
        fade_front = false;
        fade_back = false;
        #endif

        #if GPIO
        ws2812_gpio_clear(&ws2812_gpio);
        ws2812_gpio_deinit(&ws2812_gpio);
        initialized = false;
        fade_flag = 0;
        #endif

    }
    else if (strcmp(tokenizedInput[0], "DEINIT_FRONT") == 0 && front_initialized) {
        #if PWM
        ws2812_pwm_clear(&ws2812_pwm_front);
        while(!transferDone){}; // Wait for ongoing transfer to finish
        ws2812_pwm_deinit(&ws2812_pwm_front);
        front_initialized = false;
        fade_front = false;
        #endif
    }
    else if (strcmp(tokenizedInput[0], "DEINIT_BACK") == 0 && back_initialized) {
        #if PWM
        ws2812_pwm_clear(&ws2812_pwm_back);
        while(!transferDone){}; // Wait for ongoing transfer to finish
        ws2812_pwm_deinit(&ws2812_pwm_back);
        back_initialized = false;
        fade_back = false;
        #endif
    }
    else if (strcmp(tokenizedInput[0], "SET_ALL") == 0 && (initialized || (front_initialized && back_initialized))) {
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
        for (uint8_t i = 0; i < ws2812_pwm_front.led_num; i++) {
            ws2812_set_led(&ws2812_pwm_front, i, (uint8_t)atoi(tokenizedInput[1]), (uint8_t)atoi(tokenizedInput[2]), (uint8_t)atoi(tokenizedInput[3]));
        }
        for (uint8_t i = 0; i < ws2812_pwm_back.led_num; i++) {
            ws2812_set_led(&ws2812_pwm_back, i, (uint8_t)atoi(tokenizedInput[1]), (uint8_t)atoi(tokenizedInput[2]), (uint8_t)atoi(tokenizedInput[3]));
        }
        #if DMA
        send_both = true;
        ws2812_pwm_send(&ws2812_pwm_front);
        #else
        ws2812_pwm_send(&ws2812_pwm_front);
        ws2812_pwm_send(&ws2812_pwm_back);
        #endif
        #endif

        #if GPIO
        for (uint8_t i = 0; i < ws2812_gpio.led_num; i++) {
            ws2812_set_led(&ws2812_gpio, i, (uint8_t)atoi(tokenizedInput[1]), (uint8_t)atoi(tokenizedInput[2]), (uint8_t)atoi(tokenizedInput[3]));
        }
        ws2812_gpio_send(&ws2812_gpio);
        #endif
    }
    else if (strcmp(tokenizedInput[0], "SET_ALL_FRONT") == 0 && front_initialized) {
        if (!valid_command_size(commandSize, 4)) {
            return;
        }
        #if PWM
        for (uint8_t i = 0; i < ws2812_pwm_front.led_num; i++) {
            ws2812_set_led(&ws2812_pwm_front, i, (uint8_t)atoi(tokenizedInput[1]), (uint8_t)atoi(tokenizedInput[2]), (uint8_t)atoi(tokenizedInput[3]));
        }
        ws2812_pwm_send(&ws2812_pwm_front);
        #endif
    }
    else if (strcmp(tokenizedInput[0], "SET_ALL_BACK") == 0 && back_initialized) {
        if (!valid_command_size(commandSize, 4)) {
            return;
        }

        #if PWM
        for (uint8_t i = 0; i < ws2812_pwm_back.led_num; i++) {
            ws2812_set_led(&ws2812_pwm_back, i, (uint8_t)atoi(tokenizedInput[1]), (uint8_t)atoi(tokenizedInput[2]), (uint8_t)atoi(tokenizedInput[3]));
        }
        ws2812_pwm_send(&ws2812_pwm_back);
        #endif
    }
    else if (strcmp(tokenizedInput[0], "SET_SINGLE") == 0 && (initialized || (front_initialized && back_initialized))) {
        if (!valid_command_size(commandSize, 5)) {
            return;
        }

        #if SPI
        ws2812_set_led(&ws2812_spi, (uint8_t)atoi(tokenizedInput[1]), (uint8_t)atoi(tokenizedInput[2]), (uint8_t)atoi(tokenizedInput[3]), (uint8_t)atoi(tokenizedInput[4]));
        ws2812_spi_send(&ws2812_spi);
        #endif

        #if PWM
        ws2812_set_led(&ws2812_pwm_front, (uint8_t)atoi(tokenizedInput[1]), (uint8_t)atoi(tokenizedInput[2]), (uint8_t)atoi(tokenizedInput[3]), (uint8_t)atoi(tokenizedInput[4]));
        ws2812_set_led(&ws2812_pwm_back, (uint8_t)atoi(tokenizedInput[1]), (uint8_t)atoi(tokenizedInput[2]), (uint8_t)atoi(tokenizedInput[3]), (uint8_t)atoi(tokenizedInput[4]));
        
        #if DMA
        send_both = true;
        ws2812_pwm_send(&ws2812_pwm_front);
        #else
        ws2812_pwm_send(&ws2812_pwm_front);
        ws2812_pwm_send(&ws2812_pwm_back);
        #endif // DMA
        #endif // PWM

        #if GPIO
        ws2812_set_led(&ws2812_gpio, (uint8_t)atoi(tokenizedInput[1]), (uint8_t)atoi(tokenizedInput[2]), (uint8_t)atoi(tokenizedInput[3]), (uint8_t)atoi(tokenizedInput[4]));
        ws2812_gpio_send(&ws2812_gpio);
        #endif

    }
    else if (strcmp(tokenizedInput[0], "SET_SINGLE_FRONT") == 0 && front_initialized) {
        if (!valid_command_size(commandSize, 5)) {
            return;
        }
        #if PWM
        ws2812_set_led(&ws2812_pwm_front, (uint8_t)atoi(tokenizedInput[1]), (uint8_t)atoi(tokenizedInput[2]), (uint8_t)atoi(tokenizedInput[3]), (uint8_t)atoi(tokenizedInput[4]));
        ws2812_pwm_send(&ws2812_pwm_front);
        #endif
    }
    else if (strcmp(tokenizedInput[0], "SET_SINGLE_BACK") == 0 && back_initialized) {
        if (!valid_command_size(commandSize, 5)) {
            return;
        }
        #if PWM
        ws2812_set_led(&ws2812_pwm_back, (uint8_t)atoi(tokenizedInput[1]), (uint8_t)atoi(tokenizedInput[2]), (uint8_t)atoi(tokenizedInput[3]), (uint8_t)atoi(tokenizedInput[4]));
        ws2812_pwm_send(&ws2812_pwm_back);
        #endif
    }
    else if (strcmp(tokenizedInput[0], "SET_BRIGHTNESS") == 0 && (initialized || (front_initialized && back_initialized))) {
        if (!valid_command_size(commandSize, 2)) {
            return;
        }

        #if SPI
        ws2812_spi.brightness = (uint8_t)atoi(tokenizedInput[1]);
        ws2812_spi_send(&ws2812_spi);
        #endif

        #if PWM
        ws2812_pwm_front.brightness = (uint8_t)atoi(tokenizedInput[1]);
        ws2812_pwm_back.brightness = (uint8_t)atoi(tokenizedInput[1]);
        #if DMA
        send_both = true;
        ws2812_pwm_send(&ws2812_pwm_front);

        #else
        ws2812_pwm_send(&ws2812_pwm_front);
        ws2812_pwm_send(&ws2812_pwm_back);
        #endif // DMA
        #endif // PWM

        #if GPIO
        ws2812_gpio.brightness = (uint8_t)atoi(tokenizedInput[1]);
        ws2812_gpio_send(&ws2812_gpio);
        #endif
    }
    else if (strcmp(tokenizedInput[0], "SET_BRIGHTNESS_FRONT") == 0 && front_initialized) {
        #if PWM
        ws2812_pwm_front.brightness = (uint8_t)atoi(tokenizedInput[1]);
        ws2812_pwm_send(&ws2812_pwm_front);
        #endif
    }
    else if (strcmp(tokenizedInput[0], "SET_BRIGHTNESS_BACK") == 0 && back_initialized) {
        #if PWM
        ws2812_pwm_back.brightness = (uint8_t)atoi(tokenizedInput[1]);
        ws2812_pwm_send(&ws2812_pwm_back);
        #endif
    }
    else if (strcmp(tokenizedInput[0], "CLEAR_ALL") == 0 && (initialized || (front_initialized && back_initialized))) {
        #if SPI
        ws2812_spi_clear(&ws2812_spi);
        #endif

        #if PWM
        #if DMA
        send_both = true;
        ws2812_pwm_clear(&ws2812_pwm_front);
        #else
        ws2812_pwm_clear(&ws2812_pwm_front);
        ws2812_pwm_clear(&ws2812_pwm_back);
        #endif // DMA
        #endif // PWM

        #if GPIO
        ws2812_gpio_clear(&ws2812_gpio);
        #endif
    }
    else if (strcmp(tokenizedInput[0], "CLEAR_ALL_FRONT") == 0 && front_initialized) {
        #if PWM
        ws2812_pwm_clear(&ws2812_pwm_front);
        #endif
    }
    else if (strcmp(tokenizedInput[0], "CLEAR_ALL_BACK") == 0 && back_initialized) {
        #if PWM
        ws2812_pwm_clear(&ws2812_pwm_back);
        #endif
    }
    else if (strcmp(tokenizedInput[0], "CLEAR_SINGLE") == 0 && (initialized || (front_initialized && back_initialized))) {
        if (!valid_command_size(commandSize, 2)) {
            return;
        }

        #if SPI
        ws2812_set_led(&ws2812_spi, (uint8_t)atoi(tokenizedInput[1]), 0, 0, 0);
        ws2812_spi_send(&ws2812_spi);
        #endif

        #if PWM
        ws2812_set_led(&ws2812_pwm_front, (uint8_t)atoi(tokenizedInput[1]), 0, 0, 0);
        ws2812_set_led(&ws2812_pwm_back, (uint8_t)atoi(tokenizedInput[1]), 0, 0, 0);
        #if DMA
        send_both = true;
        ws2812_pwm_send(&ws2812_pwm_front);
        #else
        ws2812_pwm_send(&ws2812_pwm_front);
        ws2812_pwm_send(&ws2812_pwm_back);
        #endif // DMA
        #endif // PWM

        #if GPIO
        ws2812_set_led(&ws2812_gpio, (uint8_t)atoi(tokenizedInput[1]), 0, 0, 0);
        ws2812_gpio_send(&ws2812_gpio);
        #endif
    }
    else if (strcmp(tokenizedInput[0], "CLEAR_SINGLE_FRONT") == 0 && front_initialized) {
        if (!valid_command_size(commandSize, 2)) {
            return;
        }
        #if PWM
        ws2812_set_led(&ws2812_pwm_front, (uint8_t)atoi(tokenizedInput[1]), 0, 0, 0);
        ws2812_pwm_send(&ws2812_pwm_front);
        #endif
    }
    else if (strcmp(tokenizedInput[0], "CLEAR_SINGLE_BACK") == 0 && back_initialized) {
        if (!valid_command_size(commandSize, 2)) {
            return;
        }
        #if PWM
        ws2812_set_led(&ws2812_pwm_back, (uint8_t)atoi(tokenizedInput[1]), 0, 0, 0);
        ws2812_pwm_send(&ws2812_pwm_back);
        #endif
    }
    else if (strcmp(tokenizedInput[0], "START_FADE_ALL") == 0 && (initialized || (front_initialized && back_initialized))) {
        if (!valid_command_size(commandSize, 5)) {
            return;
        }
        #if SPI
        for (uint8_t i = 0; i < ws2812_spi.led_num; i++) {
            ws2812_set_led(&ws2812_spi, i, (uint8_t)atoi(tokenizedInput[1]), (uint8_t)atoi(tokenizedInput[2]), (uint8_t)atoi(tokenizedInput[3]));
        }
        fade_flag = 1;
        #endif
	
        #if PWM
        for (uint8_t i = 0; i < ws2812_pwm_front.led_num; i++) {
            ws2812_set_led(&ws2812_pwm_front, i, (uint8_t)atoi(tokenizedInput[1]), (uint8_t)atoi(tokenizedInput[2]), (uint8_t)atoi(tokenizedInput[3]));
        }
        for (uint8_t i = 0; i < ws2812_pwm_back.led_num; i++) {
            ws2812_set_led(&ws2812_pwm_back, i, (uint8_t)atoi(tokenizedInput[1]), (uint8_t)atoi(tokenizedInput[2]), (uint8_t)atoi(tokenizedInput[3]));
        }
        
        send_both = true;
        fade_front = true;
        fade_back = true;
        #endif

        #if GPIO
        for (uint8_t i = 0; i < ws2812_gpio.led_num; i++) {
            ws2812_set_led(&ws2812_gpio, i, (uint8_t)atoi(tokenizedInput[1]), (uint8_t)atoi(tokenizedInput[2]), (uint8_t)atoi(tokenizedInput[3]));
        }
        fade_flag = 1;
        #endif
	
        fade_time = (uint16_t)atoi(tokenizedInput[4]);

    }
    else if (strcmp(tokenizedInput[0], "START_FADE_ALL_FRONT") == 0 && front_initialized) {
        if (!valid_command_size(commandSize, 5)) {
            return;
        }
        #if PWM
        for (uint8_t i = 0; i < ws2812_pwm_front.led_num; i++) {
            ws2812_set_led(&ws2812_pwm_front, i, (uint8_t)atoi(tokenizedInput[1]), (uint8_t)atoi(tokenizedInput[2]), (uint8_t)atoi(tokenizedInput[3]));
        }
        fade_front = true;
        fade_time = (uint16_t)atoi(tokenizedInput[4]);
        #endif
    }
    else if (strcmp(tokenizedInput[0], "START_FADE_ALL_BACK") == 0 && back_initialized) {
        if (!valid_command_size(commandSize, 5)) {
            return;
        }
        #if PWM
        for (uint8_t i = 0; i < ws2812_pwm_back.led_num; i++) {
            ws2812_set_led(&ws2812_pwm_back, i, (uint8_t)atoi(tokenizedInput[1]), (uint8_t)atoi(tokenizedInput[2]), (uint8_t)atoi(tokenizedInput[3]));
        }
        fade_back = true;
        fade_time = (uint16_t)atoi(tokenizedInput[4]);
        #endif
    }
    else if (strcmp(tokenizedInput[0], "STOP_FADE_ALL") == 0 && (initialized || (front_initialized && back_initialized))) {
        #if PWM
        fade_front = false;
        fade_back = false;
        #else
        fade_flag = 0;
        #endif
    }
    else if (strcmp(tokenizedInput[0], "STOP_FADE_FRONT") == 0 && front_initialized) {
        #if PWM
        fade_front = false;
        if (send_both) {
            send_both = false;
        }
        #endif
    }
    else if (strcmp(tokenizedInput[0], "STOP_FADE_BACK") == 0 && back_initialized) {
        #if PWM
        fade_back = false;
        if (send_both) {
            send_both = false;
        }
        #endif
    }
    else if (strcmp(tokenizedInput[0], "START_FADE_SINGLE") == 0 && (initialized || (front_initialized && back_initialized))) {
        if (!valid_command_size(commandSize, 6)) {
            return;
        }
        #if SPI
        ws2812_set_led(&ws2812_spi, (uint8_t)atoi(tokenizedInput[1]), (uint8_t)atoi(tokenizedInput[2]), (uint8_t)atoi(tokenizedInput[3]), (uint8_t)atoi(tokenizedInput[4]));
        fade_flag = 1;
        #endif
        
        #if PWM
        ws2812_set_led(&ws2812_pwm_front, (uint8_t)atoi(tokenizedInput[1]), (uint8_t)atoi(tokenizedInput[2]), (uint8_t)atoi(tokenizedInput[3]), (uint8_t)atoi(tokenizedInput[4]));
        ws2812_set_led(&ws2812_pwm_back, (uint8_t)atoi(tokenizedInput[1]), (uint8_t)atoi(tokenizedInput[2]), (uint8_t)atoi(tokenizedInput[3]), (uint8_t)atoi(tokenizedInput[4]));
        #if DMA
        send_both = true;
        #endif
        fade_front = true;
        fade_back = true;
        #endif

        #if GPIO
        ws2812_set_led(&ws2812_gpio, (uint8_t)atoi(tokenizedInput[1]), (uint8_t)atoi(tokenizedInput[2]), (uint8_t)atoi(tokenizedInput[3]), (uint8_t)atoi(tokenizedInput[4]));
        fade_flag = 1;
        #endif
        
        fade_time = (uint16_t)atoi(tokenizedInput[5]);
    }
    else if (strcmp(tokenizedInput[0], "START_FADE_SINGLE_FRONT") == 0 && front_initialized) {
        if (!valid_command_size(commandSize, 6)) {
            return;
        }
        #if PWM
        ws2812_set_led(&ws2812_pwm_front, (uint8_t)atoi(tokenizedInput[1]), (uint8_t)atoi(tokenizedInput[2]), (uint8_t)atoi(tokenizedInput[3]), (uint8_t)atoi(tokenizedInput[4]));
        fade_front = true;
        fade_time = (uint16_t)atoi(tokenizedInput[5]);
        #endif
    }
    else if (strcmp(tokenizedInput[0], "START_FADE_SINGLE_BACK") == 0 && back_initialized) {
        if (!valid_command_size(commandSize, 6)) {
            return;
        }
        #if PWM
        ws2812_set_led(&ws2812_pwm_back, (uint8_t)atoi(tokenizedInput[1]), (uint8_t)atoi(tokenizedInput[2]), (uint8_t)atoi(tokenizedInput[3]), (uint8_t)atoi(tokenizedInput[4]));
        fade_back = true;
        fade_time = (uint16_t)atoi(tokenizedInput[5]);
        #endif
    }
    else if (strcmp(tokenizedInput[0], "STOP_FADE_SINGLE") == 0 && (initialized || (front_initialized && back_initialized))) {
        if (!valid_command_size(commandSize, 2)) {
            return;
        }
        #if SPI
        ws2812_set_led(&ws2812_spi, (uint8_t)atoi(tokenizedInput[1]), 0, 0, 0);
        fade_flag = 0;
        #endif
        
        #if PWM
        ws2812_set_led(&ws2812_pwm_front, (uint8_t)atoi(tokenizedInput[1]), 0, 0, 0);
        ws2812_set_led(&ws2812_pwm_back, (uint8_t)atoi(tokenizedInput[1]), 0, 0, 0);
        #if DMA
        send_both = true;
        #endif // DMA

        fade_front = false;
        fade_back = false;
        
        #endif // PWM

        #if GPIO
        ws2812_set_led(&ws2812_gpio, (uint8_t)atoi(tokenizedInput[1]), 0, 0, 0);
        fade_flag = 0;
        #endif
    }
    else if (strcmp(tokenizedInput[0], "STOP_FADE_SINGLE_FRONT" ) == 0 && front_initialized) {
        if (!valid_command_size(commandSize, 2)) {
            return;
        }
        #if PWM
        ws2812_set_led(&ws2812_pwm_front, (uint8_t)atoi(tokenizedInput[1]), 0, 0, 0);
        fade_front = false;
        #endif
    }
    else if (strcmp(tokenizedInput[0], "STOP_FADE_SINGLE_BACK" ) == 0 && back_initialized) {
        if (!valid_command_size(commandSize, 2)) {
            return;
        }
        #if PWM
        ws2812_set_led(&ws2812_pwm_back, (uint8_t)atoi(tokenizedInput[1]), 0, 0, 0);
        fade_back = false;
        #endif
    }


    token = NULL;
    memset(rxBuff, 0, RX_BUFF_SIZE);
}
