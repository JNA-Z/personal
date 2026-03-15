/**
 * @file pid.c
 * @brief PID控制器模块实现 (纯C实现)
 * @note 重构自C++的pid.cpp, pid_incremental.cpp, pid_positional.cpp
 *       使用UTF-8编码
 */

#include "pid.h"
#include <math.h>

/* ============================================================================
 * 内部辅助函数
 * ============================================================================ */

/**
 * @brief 限幅函数 - 将值限制在指定范围内
 * @param a1 要限制的值指针
 * @param a2 限制的绝对值范围
 */
static void pid_limit(float *a1, float a2)
{
    if (*a1 < 0) {
        if (fabsf(*a1) > a2) {
            *a1 = -a2;
        }
    } else {
        if (fabsf(*a1) > a2) {
            *a1 = a2;
        }
    }
}

/* ============================================================================
 * 基础PID (PD控制器) 实现
 * ============================================================================ */

void pid_base_init(pid_base_t *self)
{
    self->Kp = 0.0f;
    self->Kd = 0.0f;
    self->last_error = 0.0f;
    self->prev_error = 0.0f;
    self->p_out = 0.0f;
    self->d_out = 0.0f;
    self->total_out = 0.0f;
    self->max_output = 1000.0f;
}

void pid_base_set_max_output(pid_base_t *self, float max_output)
{
    self->max_output = max_output;
}

void pid_base_set_kp(pid_base_t *self, float kp)
{
    self->Kp = kp;
}

void pid_base_set_kd(pid_base_t *self, float kd)
{
    self->Kd = kd;
}

void pid_base_set_kp_kd(pid_base_t *self, float kp, float kd)
{
    self->Kp = kp;
    self->Kd = kd;
}

float pid_base_get_kp(pid_base_t *self)
{
    return self->Kp;
}

float pid_base_get_kd(pid_base_t *self)
{
    return self->Kd;
}

float pid_base_get_total_out(pid_base_t *self)
{
    return self->total_out;
}

/* ============================================================================
 * 增量式PID实现
 * ============================================================================ */

void pid_incremental_init(pid_incremental_t *self)
{
    /* 普通误差参数初始化 */
    self->Kp = 0.0f;
    self->Kd = 0.0f;
    self->last_error = 0.0f;
    self->prev_error = 0.0f;
    self->p_out = 0.0f;
    self->d_out = 0.0f;
    self->total_out = 0.0f;
    self->max_output = 90.0f;
    
    /* yaw方向参数初始化 */
    self->Kp_yaw = 0.0f;
    self->Kd_yaw = 0.0f;
    self->last_error_yaw = 0.0f;
    self->prev_error_yaw = 0.0f;
    self->p_out_yaw = 0.0f;
    self->d_out_yaw = 0.0f;
    self->total_out_yaw = 0.0f;
    self->max_output_yaw = 200.0f;
}

float pid_incremental_calculate(pid_incremental_t *self, float new_error)
{
    /* 传参赋值 */
    self->last_error = new_error;
    
    /* 比例项 */
    self->p_out = self->Kp * self->last_error;
    
    /* 微分项 */
    self->d_out = self->Kd * (self->last_error - self->prev_error);
    
    /* 总和 */
    self->total_out = self->p_out + self->d_out;
    
    /* 总输出限幅 */
    pid_limit(&(self->total_out), self->max_output);
    
    /* 新旧值变更 */
    self->prev_error = self->last_error;
    
    return self->total_out;
}

float pid_incremental_calculate_yaw(pid_incremental_t *self, float new_error)
{
    /* 传参赋值 */
    self->last_error_yaw = new_error;
    
    /* 比例项 */
    self->p_out_yaw = self->Kp_yaw * self->last_error_yaw;
    
    /* 微分项 */
    self->d_out_yaw = self->Kd_yaw * (self->last_error_yaw - self->prev_error_yaw);
    
    /* 总和 */
    self->total_out_yaw = self->p_out_yaw + self->d_out_yaw;
    
    /* 总输出限幅 */
    pid_limit(&(self->total_out_yaw), self->max_output_yaw);
    
    /* 新旧值变更 */
    self->prev_error_yaw = self->last_error_yaw;
    
    return self->total_out_yaw;
}

void pid_incremental_set_max_output(pid_incremental_t *self, float max_output)
{
    self->max_output = max_output;
}

void pid_incremental_set_max_output_yaw(pid_incremental_t *self, float max_output)
{
    self->max_output_yaw = max_output;
}

void pid_incremental_set_kp(pid_incremental_t *self, float kp)
{
    self->Kp = kp;
}

void pid_incremental_set_kd(pid_incremental_t *self, float kd)
{
    self->Kd = kd;
}

void pid_incremental_set_kp_kd(pid_incremental_t *self, float kp, float kd)
{
    self->Kp = kp;
    self->Kd = kd;
}

void pid_incremental_set_kp_yaw(pid_incremental_t *self, float kp_yaw)
{
    self->Kp_yaw = kp_yaw;
}

void pid_incremental_set_kd_yaw(pid_incremental_t *self, float kd_yaw)
{
    self->Kd_yaw = kd_yaw;
}

void pid_incremental_set_kp_kd_yaw(pid_incremental_t *self, float kp_yaw, float kd_yaw)
{
    self->Kp_yaw = kp_yaw;
    self->Kd_yaw = kd_yaw;
}

float pid_incremental_get_kp(pid_incremental_t *self)
{
    return self->Kp;
}

float pid_incremental_get_kd(pid_incremental_t *self)
{
    return self->Kd;
}

float pid_incremental_get_kp_yaw(pid_incremental_t *self)
{
    return self->Kp_yaw;
}

float pid_incremental_get_kd_yaw(pid_incremental_t *self)
{
    return self->Kd_yaw;
}

float pid_incremental_get_total_out(pid_incremental_t *self)
{
    return self->total_out;
}

float pid_incremental_get_total_out_yaw(pid_incremental_t *self)
{
    return self->total_out_yaw;
}

/* ============================================================================
 * 位置式PID实现
 * ============================================================================ */

void pid_positional_init(pid_positional_t *self)
{
    self->Kp = 0.0f;
    self->Kd = 0.0f;
    self->Ki = 0.0f;
    self->last_error = 0.0f;
    self->prev_error = 0.0f;
    self->p_out = 0.0f;
    self->d_out = 0.0f;
    self->i_out = 0.0f;
    self->total_out = 0.0f;
    self->max_output = 1000.0f;
    self->target = 0.0f;
    self->present = 0.0f;
    self->integral_limit = 0.0f;
    self->i_band = 0.0f;
}

void pid_positional_init_target(pid_positional_t *self, float target, 
                                     float integral_limit, float i_band)
{
    self->target = target;
    self->integral_limit = integral_limit;
    self->i_band = i_band;
    pid_positional_clear_i_out(self);
}

float pid_positional_calculate(pid_positional_t *self, float present)
{
    self->last_error = self->target - present;
    
    /* 比例项 */
    self->p_out = self->Kp * self->last_error;
    
    /* 积分项 */
    if (self->Ki != 0.0f) {
        if (fabsf(present) < self->i_band) {
            /* 在积分分离范围内, 正常积分 */
            self->i_out = self->Ki * present;
            pid_limit(&(self->i_out), self->integral_limit);
        } else {
            /* 超出积分分离范围, 使用衰减积分 */
            self->i_out = self->Ki * self->i_band * (self->last_error / self->i_band);
        }
    }
    
    /* 微分项 */
    self->d_out = self->Kd * (self->last_error - self->prev_error);
    
    /* 总和 */
    self->total_out = self->p_out + self->i_out + self->d_out;
    
    /* 总输出限幅 */
    pid_limit(&(self->total_out), self->max_output);
    
    /* 新旧值变更 */
    self->prev_error = self->last_error;
    
    /* 防止输出错误: 当目标值小于等于当前值时返回0 */
    if (self->target <= present) {
        return 0.0f;
    }
    
    return self->total_out;
}

void pid_positional_set_max_output(pid_positional_t *self, float max_output)
{
    self->max_output = max_output;
}

void pid_positional_set_target(pid_positional_t *self, float target)
{
    self->target = target;
}

void pid_positional_set_kp(pid_positional_t *self, float kp)
{
    self->Kp = kp;
}

void pid_positional_set_ki(pid_positional_t *self, float ki)
{
    self->Ki = ki;
}

void pid_positional_set_kd(pid_positional_t *self, float kd)
{
    self->Kd = kd;
}

void pid_positional_set_kp_ki_kd(pid_positional_t *self, float kp, float ki, float kd)
{
    self->Kp = kp;
    self->Ki = ki;
    self->Kd = kd;
}

float pid_positional_get_kp(pid_positional_t *self)
{
    return self->Kp;
}

float pid_positional_get_ki(pid_positional_t *self)
{
    return self->Ki;
}

float pid_positional_get_kd(pid_positional_t *self)
{
    return self->Kd;
}

float pid_positional_get_target(pid_positional_t *self)
{
    return self->target;
}

float pid_positional_get_total_out(pid_positional_t *self)
{
    return self->total_out;
}

void pid_positional_clear_i_out(pid_positional_t *self)
{
    self->i_out = 0.0f;
}
