#include "led_port.h"
#include "../driver/led.h"
#include "main.h"


static void led_write_pin(void* gpio, u16 pin, u8 value)
{
    HAL_GPIO_WritePin((GPIO_TypeDef*)gpio, pin, value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static const led_port_t g_led_port = {
    .write_pin = led_write_pin,
};

void led_port_init(void)
{
    led_bind_port(&g_led_port);
}
