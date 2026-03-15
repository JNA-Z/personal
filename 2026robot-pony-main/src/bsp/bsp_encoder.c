/**
 * @file bsp_encoder.c
 * @brief 编码器驱动模块实现 (纯粹化版本)
 */

#include "bsp_encoder.h"

void bsp_encoder_init(bsp_encoder_t *self)
{
    self->capture_count = 0;
    self->last_count = 0;
    self->overflow_count = 0;
    self->shaft_speed = 0.0f;
    self->direction = ENCODER_DIR_FORWARD;
}

void bsp_encoder_start(bsp_encoder_t *self)
{
    /* 清零计数器和溢出计数 */
    self->capture_count = 0;
    self->last_count = 0;
    self->overflow_count = 0;
    __HAL_TIM_SET_COUNTER(self->htim, 0);
    
    /* 清零中断标志位 */
    __HAL_TIM_CLEAR_IT(self->htim, TIM_IT_UPDATE);
    /* 使能定时器的更新事件中断 */
    __HAL_TIM_ENABLE_IT(self->htim, TIM_IT_UPDATE);
    /* 使能编码器接口 */
    HAL_TIM_Encoder_Start_IT(self->htim, TIM_CHANNEL_ALL);
}

void bsp_encoder_stop(bsp_encoder_t *self)
{
    HAL_TIM_Encoder_Stop(self->htim, TIM_CHANNEL_ALL);
    (void)self;
}

void bsp_encoder_clear(bsp_encoder_t *self)
{
    self->capture_count = 0;
    self->last_count = 0;
    self->overflow_count = 0;
    __HAL_TIM_SET_COUNTER(self->htim, 0);
}

void bsp_encoder_overflow_handler(bsp_encoder_t *self)
{
    if (__HAL_TIM_IS_TIM_COUNTING_DOWN(self->htim)) {
        self->overflow_count--;
    } else {
        self->overflow_count++;
    }
}

int32_t bsp_encoder_get_count(bsp_encoder_t *self)
{
    uint32_t counter = __HAL_TIM_GET_COUNTER(self->htim);
    self->capture_count = (int32_t)counter + (self->overflow_count * 65535);
    return self->capture_count;
}

encoder_dir_t bsp_encoder_get_direction(bsp_encoder_t *self)
{
    self->direction = __HAL_TIM_IS_TIM_COUNTING_DOWN(self->htim) ? 
                       ENCODER_DIR_REVERSE : ENCODER_DIR_FORWARD;
    return self->direction;
}

float bsp_encoder_get_shaft_speed(bsp_encoder_t *self)
{
    bsp_encoder_get_count(self);
    self->shaft_speed = ((float)(self->capture_count - self->last_count) 
                         / ENCODER_READ_RESOLUTION * ENCODER_TIME_VALUE);
    self->last_count = self->capture_count;
    return self->shaft_speed;
}

float bsp_encoder_get_output_shaft_speed(bsp_encoder_t *self)
{
    return bsp_encoder_get_shaft_speed(self) / REDUCTION_RATIO;
}
