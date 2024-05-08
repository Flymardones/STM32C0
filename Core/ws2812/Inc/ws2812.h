/*
 * ws2812.h
 *
 *  Created on: Feb 22, 2024
 *      Author: chris
 */

#ifndef WS2812_INC_WS2812_H_
#define WS2812_INC_WS2812_H_

#include "main.h"

typedef enum {
    ws2812_ok,
    ws2812_error,
    ws2812_dma_error,
} ws2812_status_t;

typedef enum {
    GREEN,
    RED,
    BLUE,
} ws2812_color_t;

typedef struct {

    /*
    * @member handle is the structure definition for the peripheral
    */
    void* handle;

    /*
    * @member led_num is the amount of leds
    */
    uint8_t led_num;

    /*
    * @member led_num is the amount of leds
    */
    uint8_t brightness;

    /*
    * @member fade is the fade for the leds
    */
    int8_t fade;

    /*
    * @member buffer is the buffer for storing the data for the leds
    */
    uint8_t(*led_data)[3];

    /*
    * @member dma is used for defining whether the dma should be used of not
    */
    uint8_t dma;

    /*
    * @member circBuffer is the buffer for sending data in circular mode
    */
    uint8_t circBuffer[48];

    /*
    * @member ping_pong is used for sending in a ping pong fashion
    */
    bool ping_pong;

} ws2812_configuration;


/*!
 * @brief Set the RGB value of a single LED
 *
 * @param ws2812_conf the ws2812 configuration
 * @param led the led number to set
 * @param red the red value (0-255)
 * @param green the green value (0-255)
 * @param blue the blue value (0-255)
 */
void ws2812_set_led(ws2812_configuration* ws2812_conf, uint8_t led, uint8_t red, uint8_t green, uint8_t blue);


/*!
 * @brief Microsecond delay
 *
 * @param us the amount of microseconds to delay
 */
void ws2812_delay_us(uint16_t us);



#endif /* WS2812_INC_WS2812_H_ */
