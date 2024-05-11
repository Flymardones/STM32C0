#ifndef WS2812_GPIO_H
#define WS2812_GPIO_H

#include "ws2812.h"

extern ws2812_configuration ws2812_gpio;

bool ws2812_gpio_init();
void ws2812_gpio_deinit();
void ws2812_gpio_clear();
void ws2812_gpio_fade(ws2812_configuration* ws2812_conf, uint16_t fade_time_ms);
void ws2812_gpio_send(ws2812_configuration *ws2812_conf);
void ws2812_gpio_data(ws2812_configuration* ws2812_conf, uint8_t green, uint8_t red, uint8_t blue);
void ws2812_gpio_send_burst(ws2812_configuration* ws2812_conf);

#endif // WS2812_GPIO_H