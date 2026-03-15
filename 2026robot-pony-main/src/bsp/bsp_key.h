/**
 * @file bsp_key.h
 * @brief 按键驱动模块 (纯粹化版本)
 * @note 硬件配置在model层静态初始化，此模块只关注逻辑
 */

#ifndef __BSP_KEY_H
#define __BSP_KEY_H

#include <stdint.h>
#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 按键对象结构体
 * @note 硬件配置(gpiox, gpio_pin)应在定义时静态初始化
 */
typedef struct {
    GPIO_TypeDef *gpiox;    /* GPIO端口 (静态初始化) */
    uint16_t gpio_pin;      /* GPIO引脚 (静态初始化) */
    uint8_t pressed;        /* 按键状态: 0-未按下, 1-按下 */
} bsp_key_t;

/**
 * @brief 初始化按键对象运行时状态
 * @param self 按键对象指针
 */
void bsp_key_init(bsp_key_t *self);

/**
 * @brief 扫描单个按键 (带消抖)
 * @param self 按键对象指针
 * @return 1: 按下, 0: 未按下
 */
uint8_t bsp_key_scan(bsp_key_t *self);

/**
 * @brief 检测按键按下事件 (上升沿)
 * @param self 按键对象指针
 * @return 1: 按下事件, 0: 无事件
 */
uint8_t bsp_key_pressed(bsp_key_t *self);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_KEY_H */
