/**
 * @file distance_patrol.c
 * @brief 距离巡逻模块实现 (纯C实现)
 */

#include "distance_patrol.h"
#include "chassis.h"

/* ============================================================================
 * 初始化函数
 * ============================================================================ */

void distance_patrol_init(distance_patrol_t *self, bsp_encoder_t *encoder)
{
    /* 初始化PID控制器 */
    pid_positional_init(&self->pid);
    
    /* 设置编码器指针 */
    self->encoder = encoder;
}

void distance_patrol_init_target(distance_patrol_t *self, 
                                 float target, 
                                 float integral_limit, 
                                 float i_band)
{
    pid_positional_init_target(&self->pid, target, integral_limit, i_band);
}

void distance_patrol_encoder_init(distance_patrol_t *self)
{
    if (self->encoder != NULL) {
        bsp_encoder_start(self->encoder);
    }
}

/* ============================================================================
 * PID参数设置函数
 * ============================================================================ */

void distance_patrol_set_kp_ki_kd(distance_patrol_t *self, float kp, float ki, float kd)
{
    pid_positional_set_kp_ki_kd(&self->pid, kp, ki, kd);
}

/* ============================================================================
 * 运动控制函数
 * ============================================================================ */

void distance_patrol_move_distance(distance_patrol_t *self, 
                                   int32_t distance, 
                                   float integral_limit, 
                                   float i_band)
{
    /* 设置目标距离和积分参数 */
    distance_patrol_init_target(self, (float)distance, integral_limit, i_band);
    
    /* 初始化编码器 */
    if (self->encoder != NULL) {
        bsp_encoder_start(self->encoder);
    }
}

int16_t distance_patrol_get_calculate_speed(distance_patrol_t *self)
{
    int32_t count;
    float pid_out;
    
    if (self->encoder == NULL) {
        return 0;
    }
    
    /* 获取编码值 */
    count = bsp_encoder_get_count(self->encoder);
    
    /* 计算PID输出 */
    pid_out = pid_positional_calculate(&self->pid, (float)count);
    
    /* 当输出小于等于5时, 停止编码器 */
    if (pid_out <= 5.0f) {
        bsp_encoder_stop(self->encoder);
    }
    
    return (int16_t)pid_out;
}

/* ============================================================================
 * 参数获取函数
 * ============================================================================ */

float distance_patrol_get_kp(distance_patrol_t *self)
{
    return pid_positional_get_kp(&self->pid);
}

float distance_patrol_get_ki(distance_patrol_t *self)
{
    return pid_positional_get_ki(&self->pid);
}

float distance_patrol_get_kd(distance_patrol_t *self)
{
    return pid_positional_get_kd(&self->pid);
}

int32_t distance_patrol_get_count(distance_patrol_t *self)
{
    if (self->encoder != NULL) {
        return bsp_encoder_get_count(self->encoder);
    }
    return 0;
}

int32_t distance_patrol_get_capture_right_count(distance_patrol_t *self)
{
    return distance_patrol_get_count(self);
}

int32_t distance_patrol_get_capture_left_count(distance_patrol_t *self)
{
    return distance_patrol_get_count(self);
}

/* ============================================================================
 * 距离停止函数
 * ============================================================================ */

uint8_t distance_patrol_run_distance(distance_patrol_t *self, int32_t encoder_value)
{
    if (self->encoder == NULL) {
        return 1;
    }
    
    /* 检查编码值是否达到目标 */
    if (bsp_encoder_get_count(self->encoder) >= encoder_value) {
        /* 停止电机 */
        chassis_set_speed(0);
        chassis_disable();
        
        /* 停止编码器 */
        bsp_encoder_stop(self->encoder);
        
        return 1;   /* 已停止 */
    }
    
    return 0;       /* 工作中 */
}

uint8_t distance_patrol_run_right_distance(distance_patrol_t *self, int32_t encoder_value)
{
    return distance_patrol_run_distance(self, encoder_value);
}

uint8_t distance_patrol_run_left_distance(distance_patrol_t *self, int32_t encoder_value)
{
    return distance_patrol_run_distance(self, encoder_value);
}

/* ============================================================================
 * 输出限制函数
 * ============================================================================ */

void distance_patrol_set_max_output(distance_patrol_t *self, float max_output)
{
    pid_positional_set_max_output(&self->pid, max_output);
}
