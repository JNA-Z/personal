/**
 * @file bsp_key.c
 * @brief 按键驱动模块实现 (纯粹化版本)
 */

#include "bsp_key.h"

void bsp_key_init(bsp_key_t *self)
{
    self->pressed = 0;
}

uint8_t bsp_key_scan(bsp_key_t *self)
{
    if (HAL_GPIO_ReadPin(self->gpiox, self->gpio_pin) == GPIO_PIN_RESET) {
        HAL_Delay(10);  /* 消抖 */
        if (HAL_GPIO_ReadPin(self->gpiox, self->gpio_pin) == GPIO_PIN_RESET) {
            /* 等待按键释放 */
            while (HAL_GPIO_ReadPin(self->gpiox, self->gpio_pin) == GPIO_PIN_RESET) {
                HAL_Delay(1);
            }
            return 1;
        }
    }
    return 0;
}

uint8_t bsp_key_pressed(bsp_key_t *self)
{
    uint8_t current = (HAL_GPIO_ReadPin(self->gpiox, self->gpio_pin) == GPIO_PIN_RESET) ? 1 : 0;
    uint8_t event = (current && !self->pressed);
    self->pressed = current;
    return event;
}
