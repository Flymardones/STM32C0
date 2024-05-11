#include "ws2812_gpio.h"



static inline void ws2812_write_high() {
    GPIOA->ODR = GPIO_PIN_9;
    __asm__ volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
    __asm__ volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"); 
    

    GPIOA->ODR = ~GPIO_PIN_9;
    __asm__ volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"); 
}

static inline void ws2812_write_low() {
    GPIOA->ODR = GPIO_PIN_9;
    __asm__ volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\n");

    GPIOA->ODR = ~GPIO_PIN_9;
    __asm__ volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
    __asm__ volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
}


void ws2812_gpio_fade(ws2812_configuration* ws2812_conf, uint16_t fade_time_ms) {

  uint16_t fade_delay = 0;
  
  /* Do nothing */
  if (fade_time_ms < ws2812_conf->brightness) {
	return;
  }
  
  fade_delay = (fade_time_ms / ws2812_conf->brightness);
  
  for (ws2812_conf->fade = ws2812_conf->brightness; ws2812_conf->fade >= 0; ws2812_conf->fade -= 2) {
    ws2812_gpio_send(ws2812_conf);
    HAL_Delay(fade_delay);
  }
  
  for (ws2812_conf->fade = 0; ws2812_conf->fade < ws2812_conf->brightness; ws2812_conf->fade += 2) {
    ws2812_gpio_send(ws2812_conf);
	HAL_Delay(fade_delay);

  }
}

void ws2812_gpio_send(ws2812_configuration* ws2812_conf) {
    for (uint8_t i = 0; i < ws2812_conf->led_num; i++) {
        ws2812_gpio_data(ws2812_conf, ws2812_conf->led_data[i][GREEN], ws2812_conf->led_data[i][RED], ws2812_conf->led_data[i][BLUE]);
    }
    ws2812_delay_us(280);
}

void ws2812_gpio_data(ws2812_configuration* ws2812_conf, uint8_t green, uint8_t red, uint8_t blue) {
    uint8_t send_data[24];

    if (fade_flag) {
        green = (green * ws2812_conf->fade / 100);
        red = (red * ws2812_conf->fade / 100);
        blue = (blue * ws2812_conf->fade / 100);
    } 
    else {
        green = (green * ws2812_conf->brightness / 100);
        red = (red * ws2812_conf->brightness / 100);
        blue = (blue * ws2812_conf->brightness / 100);
    }

    for (uint8_t i = 0; i < 8; i++) {
        uint8_t mask = 1 << (7 - i);
        send_data[i] = (green & mask) ? 1 : 0;
        send_data[i + 8] = (red & mask) ? 1 : 0;
        send_data[i + 16] = (blue & mask) ? 1 : 0;
    }

    for (uint8_t i = 0; i < 24; i++) {
        if (send_data[i]) {
            ws2812_write_high();
        }
        else {
            ws2812_write_low();
        }
    }

}

void ws2812_gpio_send_burst(ws2812_configuration* ws2812_conf) {
    uint8_t send_data[ws2812_conf->led_num * 24];
    uint8_t green, red, blue;

   	for (uint8_t i = 0; i < ws2812_conf->led_num; i++) {
        green = ws2812_conf->led_data[i][GREEN] * ws2812_conf->brightness / 100;
        red = ws2812_conf->led_data[i][RED] * ws2812_conf->brightness / 100;
        blue = ws2812_conf->led_data[i][BLUE] * ws2812_conf->brightness / 100;

        for (uint8_t j = 0; j < 8; j++) {
            int index = i * 24 + j;
            uint8_t mask = 1 << (7 - j);
            send_data[index] = (green & mask) ? 1 : 0;
            send_data[index + 8] = (red & mask) ? 1 : 0;
            send_data[index + 16] = (blue & mask) ? 1 : 0;
        }
    }    
    
    for (uint16_t i = 0; i < ws2812_conf->led_num * 24; i++) {
        if (send_data[i]) {
            ws2812_write_high();
        }
        else {
            ws2812_write_low();
        }
    }
    ws2812_delay_us(280);
}

void ws2812_gpio_clear(ws2812_configuration* ws2812_conf) {
    memset(ws2812_conf->led_data, 0, ws2812_conf->led_num * sizeof(*ws2812_conf->led_data));
    ws2812_gpio_send(ws2812_conf);
}

bool ws2812_gpio_init(ws2812_configuration* ws2812_conf) {

    ws2812_conf->led_data = malloc(sizeof(uint8_t[3]) * ws2812_conf->led_num);
    
    if (ws2812_conf->led_data == NULL) {
        return false;
    }
    
    memset(ws2812_conf->led_data, 0, ws2812_conf->led_num * sizeof(*ws2812_conf->led_data));

    ws2812_gpio_send(ws2812_conf);

    return true;
}

void ws2812_gpio_deinit(ws2812_configuration* ws2812_conf) {

    free(ws2812_conf->led_data);
    ws2812_conf->led_data = NULL;
	ws2812_conf->handle = NULL;
	ws2812_conf->led_num = 0;
	ws2812_conf->brightness = 0;
	ws2812_conf->dma = 0;
}