/**
 * @file bsp_led.c
 * @brief LED驱动模块实现 (纯粹化版本)
 */

#include "bsp_led.h"

void bsp_led_init(bsp_led_t *self)
{
    /* 硬件配置已在静态初始化时完成，此函数可用于运行时状态初始化 */
    (void)self;
}

void bsp_led_on(bsp_led_t *self)
{
    HAL_GPIO_WritePin(self->gpiox, self->gpio_pin, GPIO_PIN_SET);
}

void bsp_led_off(bsp_led_t *self)
{
    HAL_GPIO_WritePin(self->gpiox, self->gpio_pin, GPIO_PIN_RESET);
}

void bsp_led_toggle(bsp_led_t *self)
{
    HAL_GPIO_TogglePin(self->gpiox, self->gpio_pin);
}
