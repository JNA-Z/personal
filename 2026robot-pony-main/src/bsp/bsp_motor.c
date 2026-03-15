/**
 * @file bsp_motor.c
 * @brief 单电机驱动模块实现 (纯粹化版本)
 */

#include "bsp_motor.h"

void bsp_motor_init(bsp_motor_t *self, uint16_t limit_spd)
{
    self->enabled = 0;
    self->speed = 0;
    self->limit_spd = limit_spd;
    self->dir = MOTOR_DIR_BRAKE;
    
    /* 设置初始状态为制动 */
    __HAL_TIM_SET_COMPARE(self->htim, self->fwd_channel, 0);
    __HAL_TIM_SET_COMPARE(self->htim, self->rev_channel, 0);
}

void bsp_motor_enable(bsp_motor_t *self)
{
    self->enabled = 1;
    HAL_TIM_PWM_Start(self->htim, self->fwd_channel);
    HAL_TIM_PWM_Start(self->htim, self->rev_channel);
}

void bsp_motor_disable(bsp_motor_t *self)
{
    self->enabled = 0;
    HAL_TIM_PWM_Stop(self->htim, self->fwd_channel);
    HAL_TIM_PWM_Stop(self->htim, self->rev_channel);
}

void bsp_motor_set_enable(bsp_motor_t *self, uint8_t en)
{
    if (en) {
        bsp_motor_enable(self);
    } else {
        bsp_motor_disable(self);
    }
}

void bsp_motor_set_limit_spd(bsp_motor_t *self, uint16_t limit_spd)
{
    self->limit_spd = limit_spd;
}

void bsp_motor_set_speed(bsp_motor_t *self, int16_t speed)
{
    int16_t abs_speed = (speed >= 0) ? speed : -speed;
    
    /* 速度限幅 */
    if (abs_speed > self->limit_spd) {
        abs_speed = self->limit_spd;
    }
    self->speed = (uint16_t)abs_speed;
    
    /* 确定方向 */
    if (speed > 0) {
        self->dir = MOTOR_DIR_FORWARD;
    } else if (speed < 0) {
        self->dir = MOTOR_DIR_REVERSE;
    } else {
        self->dir = MOTOR_DIR_BRAKE;
    }
    
    /* 设置PWM输出 */
    switch (self->dir) {
    case MOTOR_DIR_FORWARD:
        __HAL_TIM_SET_COMPARE(self->htim, self->fwd_channel, self->speed);
        __HAL_TIM_SET_COMPARE(self->htim, self->rev_channel, 0);
        break;
        
    case MOTOR_DIR_REVERSE:
        __HAL_TIM_SET_COMPARE(self->htim, self->fwd_channel, 0);
        __HAL_TIM_SET_COMPARE(self->htim, self->rev_channel, self->speed);
        break;
        
    case MOTOR_DIR_BRAKE:
    default:
        __HAL_TIM_SET_COMPARE(self->htim, self->fwd_channel, 0);
        __HAL_TIM_SET_COMPARE(self->htim, self->rev_channel, 0);
        break;
    }
}

int16_t bsp_motor_get_speed(bsp_motor_t *self)
{
    switch (self->dir) {
    case MOTOR_DIR_FORWARD:
        return (int16_t)self->speed;
    case MOTOR_DIR_REVERSE:
        return -(int16_t)self->speed;
    case MOTOR_DIR_BRAKE:
    default:
        return 0;
    }
}

motor_dir_t bsp_motor_get_direction(bsp_motor_t *self)
{
    return self->dir;
}
