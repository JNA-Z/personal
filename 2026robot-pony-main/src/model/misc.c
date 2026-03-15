#include "misc.h"
#include "../driver/led.h"
#include "../board/led_port.h"
#include "../driver/st7735s.h"
#include "../board/lcd_port.h"
#include "main.h"

led_t g_led = {0};

// 根据cubemx配置一下宏
st7735s_t g_lcd = {
    0
    // .cs_gpio  = GPIOA,
    // .cs_pin   = GPIO_PIN_4,
    // .res_gpio = GPIOA,
    // .res_pin  = GPIO_PIN_5,
    // .a0_gpio  = GPIOA,
    // .a0_pin   = GPIO_PIN_6,
    // .sda_gpio = GPIOA,
    // .sda_pin  = GPIO_PIN_7,
    // .scl_gpio = GPIOB,
    // .scl_pin  = GPIO_PIN_6,
};

void init_model(void)
{
    led_port_init();

    // led_init(&g_led, GPIOB, GPIO_PIN_0, 1);

    
    lcd_port_init();
    st7735s_init(&g_lcd);
}

void open_led(void)
{
    led_on(&g_led);
}

void close_led(void)
{
    led_off(&g_led);
}
