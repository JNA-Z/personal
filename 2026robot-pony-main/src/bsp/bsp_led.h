/**
 * @file bsp_led.h
 * @brief LED驱动模块 (纯粹化版本)
 * @note 硬件配置在model层静态初始化，此模块只关注逻辑
 */

#ifndef __BSP_LED_H
#define __BSP_LED_H

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief LED对象结构体
 * @note 硬件配置(gpiox, gpio_pin)应在定义时静态初始化
 */
typedef struct {
    GPIO_TypeDef *gpiox;    /* GPIO端口 (静态初始化) */
    uint16_t gpio_pin;      /* GPIO引脚 (静态初始化) */
} bsp_led_t;

/**
 * @brief 初始化LED对象运行时状态
 * @param self LED对象指针
 * @note 此函数只初始化运行时需要的非硬件状态，硬件配置应静态初始化
 */
void bsp_led_init(bsp_led_t *self);

/**
 * @brief 打开LED
 * @param self LED对象指针
 */
void bsp_led_on(bsp_led_t *self);

/**
 * @brief 关闭LED
 * @param self LED对象指针
 */
void bsp_led_off(bsp_led_t *self);

/**
 * @brief 翻转LED状态
 * @param self LED对象指针
 */
void bsp_led_toggle(bsp_led_t *self);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_LED_H */
