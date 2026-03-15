/**
 * @file bsp_motor.h
 * @brief 单电机驱动模块 (纯粹化版本)
 * @note 硬件配置在model层静态初始化，此模块只关注逻辑
 *       每个电机是独立的对象，通过配置区分左右
 */

#ifndef __BSP_MOTOR_H
#define __BSP_MOTOR_H

#include <stdint.h>
#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 电机方向枚举
 */
typedef enum {
    MOTOR_DIR_REVERSE = -1,     /* 反转 */
    MOTOR_DIR_BRAKE   = 0,      /* 制动 */
    MOTOR_DIR_FORWARD = 1       /* 正转 */
} motor_dir_t;

/**
 * @brief 电机对象结构体
 * @note 硬件配置(htim, fwd_channel, rev_channel)应在定义时静态初始化
 */
typedef struct {
    TIM_HandleTypeDef *htim;        /* 定时器句柄 (静态初始化) */
    uint32_t fwd_channel;          /* 正转PWM通道 (静态初始化) */
    uint32_t rev_channel;          /* 反转PWM通道 (静态初始化) */
    volatile uint8_t enabled;      /* 使能标志 */
    volatile uint16_t speed;       /* 当前速度绝对值 */
    volatile uint16_t limit_spd;   /* 速度限幅 */
    volatile motor_dir_t dir;      /* 方向状态 */
} bsp_motor_t;

/**
 * @brief 初始化电机对象运行时状态
 * @param self 电机对象指针
 * @param limit_spd 速度限幅值
 */
void bsp_motor_init(bsp_motor_t *self, uint16_t limit_spd);

/**
 * @brief 使能电机
 * @param self 电机对象指针
 */
void bsp_motor_enable(bsp_motor_t *self);

/**
 * @brief 失能电机
 * @param self 电机对象指针
 */
void bsp_motor_disable(bsp_motor_t *self);

/**
 * @brief 设置电机使能状态
 * @param self 电机对象指针
 * @param en 1:使能, 0:失能
 */
void bsp_motor_set_enable(bsp_motor_t *self, uint8_t en);

/**
 * @brief 设置速度限幅
 * @param self 电机对象指针
 * @param limit_spd 速度限幅值
 */
void bsp_motor_set_limit_spd(bsp_motor_t *self, uint16_t limit_spd);

/**
 * @brief 设置电机速度
 * @param self 电机对象指针
 * @param speed 速度值 (-limit_spd ~ +limit_spd, 负值反转)
 */
void bsp_motor_set_speed(bsp_motor_t *self, int16_t speed);

/**
 * @brief 获取电机当前速度
 * @param self 电机对象指针
 * @return 当前速度 (带符号)
 */
int16_t bsp_motor_get_speed(bsp_motor_t *self);

/**
 * @brief 获取电机方向
 * @param self 电机对象指针
 * @return 方向状态
 */
motor_dir_t bsp_motor_get_direction(bsp_motor_t *self);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_MOTOR_H */
