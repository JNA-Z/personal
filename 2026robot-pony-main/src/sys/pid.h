/**
 * @file pid.h
 * @brief PID控制器模块 (纯C实现)
 * @note 重构自C++的pid.hpp, pid_incremental.hpp, pid_positional.hpp
 *       使用UTF-8编码
 */

#ifndef __pid_H
#define __pid_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * PID类型枚举
 * ============================================================================ */
typedef enum {
    pid_TYPE_INCREMENTAL = 0,   /* 增量式PID */
    pid_TYPE_POSITIONAL  = 1    /* 位置式PID */
} pid_type_t;

/* ============================================================================
 * 基础PID结构体 (PD控制器)
 * ============================================================================ */
typedef struct {
    /* PID参数 */
    float Kp;               /* 比例系数 */
    float Kd;               /* 微分系数 */
    
    /* 输出值 */
    float p_out;            /* 比例输出 */
    float d_out;            /* 微分输出 */
    float total_out;        /* 总输出 */
    
    /* 误差值 */
    float last_error;       /* 最后的误差值 */
    float prev_error;       /* 上次的误差值 */
    
    /* 限制 */
    float max_output;       /* PID最大输出限制 */
} pid_base_t;

/* ============================================================================
 * 增量式PID结构体
 * ============================================================================ */
typedef struct {
    /* 基础PD参数 (普通误差计算) */
    float Kp;               /* 比例系数 */
    float Kd;               /* 微分系数 */
    float p_out;            /* 比例输出 */
    float d_out;            /* 微分输出 */
    float total_out;        /* 总输出 */
    float last_error;       /* 最后的误差值 */
    float prev_error;       /* 上次的误差值 */
    float max_output;       /* PID最大输出限制 */
    
    /* yaw方向参数 (偏航角误差计算) */
    float Kp_yaw;           /* yaw方向比例系数 */
    float Kd_yaw;           /* yaw方向微分系数 */
    float p_out_yaw;        /* yaw方向比例输出 */
    float d_out_yaw;        /* yaw方向微分输出 */
    float total_out_yaw;    /* yaw方向总输出 */
    float last_error_yaw;   /* yaw方向最后误差值 */
    float prev_error_yaw;   /* yaw方向上次误差值 */
    float max_output_yaw;   /* yaw方向最大输出限制 */
} pid_incremental_t;

/* ============================================================================
 * 位置式PID结构体
 * ============================================================================ */
typedef struct {
    /* PID参数 */
    float Kp;               /* 比例系数 */
    float Ki;               /* 积分系数 */
    float Kd;               /* 微分系数 */
    
    /* 输出值 */
    float p_out;            /* 比例输出 */
    float i_out;            /* 积分输出 */
    float d_out;            /* 微分输出 */
    float total_out;        /* 总输出 */
    
    /* 误差值 */
    float last_error;       /* 最后的误差值 */
    float prev_error;       /* 上次的误差值 */
    
    /* 目标与当前值 */
    float target;           /* 目标值 */
    float present;          /* 当前值 */
    
    /* 限制参数 */
    float max_output;       /* PID最大输出限制 */
    float integral_limit;   /* 积分限幅 (最大积分补偿) */
    float i_band;           /* 积分分离 (离目标距离多少开始积分补偿) */
} pid_positional_t;

/* ============================================================================
 * 基础PID (PD控制器) 接口函数
 * ============================================================================ */

/**
 * @brief 初始化基础PID控制器
 * @param self PID控制器指针
 */
void pid_base_init(pid_base_t *self);

/**
 * @brief 设置最大输出值
 * @param self PID控制器指针
 * @param max_output 最大输出值
 */
void pid_base_set_max_output(pid_base_t *self, float max_output);

/**
 * @brief 设置Kp值
 * @param self PID控制器指针
 * @param kp 比例系数
 */
void pid_base_set_kp(pid_base_t *self, float kp);

/**
 * @brief 设置Kd值
 * @param self PID控制器指针
 * @param kd 微分系数
 */
void pid_base_set_kd(pid_base_t *self, float kd);

/**
 * @brief 设置Kp和Kd值
 * @param self PID控制器指针
 * @param kp 比例系数
 * @param kd 微分系数
 */
void pid_base_set_kp_kd(pid_base_t *self, float kp, float kd);

/**
 * @brief 获取Kp值
 * @param self PID控制器指针
 * @return 当前Kp值
 */
float pid_base_get_kp(pid_base_t *self);

/**
 * @brief 获取Kd值
 * @param self PID控制器指针
 * @return 当前Kd值
 */
float pid_base_get_kd(pid_base_t *self);

/**
 * @brief 获取上次计算的输出值
 * @param self PID控制器指针
 * @return 上次总输出值
 */
float pid_base_get_total_out(pid_base_t *self);

/* ============================================================================
 * 增量式PID接口函数
 * ============================================================================ */

/**
 * @brief 初始化增量式PID控制器
 * @param self PID控制器指针
 */
void pid_incremental_init(pid_incremental_t *self);

/**
 * @brief 计算增量式PID输出 (普通误差)
 * @param self PID控制器指针
 * @param new_error 新的误差值
 * @return 计算后的输出值
 */
float pid_incremental_calculate(pid_incremental_t *self, float new_error);

/**
 * @brief 计算增量式PID输出 (yaw偏航角误差)
 * @param self PID控制器指针
 * @param new_error 新的yaw误差值
 * @return 计算后的输出值
 */
float pid_incremental_calculate_yaw(pid_incremental_t *self, float new_error);

/**
 * @brief 设置最大输出值
 * @param self PID控制器指针
 * @param max_output 最大输出值
 */
void pid_incremental_set_max_output(pid_incremental_t *self, float max_output);

/**
 * @brief 设置yaw方向最大输出值
 * @param self PID控制器指针
 * @param max_output yaw方向最大输出值
 */
void pid_incremental_set_max_output_yaw(pid_incremental_t *self, float max_output);

/**
 * @brief 设置Kp值
 * @param self PID控制器指针
 * @param kp 比例系数
 */
void pid_incremental_set_kp(pid_incremental_t *self, float kp);

/**
 * @brief 设置Kd值
 * @param self PID控制器指针
 * @param kd 微分系数
 */
void pid_incremental_set_kd(pid_incremental_t *self, float kd);

/**
 * @brief 设置Kp和Kd值
 * @param self PID控制器指针
 * @param kp 比例系数
 * @param kd 微分系数
 */
void pid_incremental_set_kp_kd(pid_incremental_t *self, float kp, float kd);

/**
 * @brief 设置yaw方向Kp值
 * @param self PID控制器指针
 * @param kp_yaw yaw方向比例系数
 */
void pid_incremental_set_kp_yaw(pid_incremental_t *self, float kp_yaw);

/**
 * @brief 设置yaw方向Kd值
 * @param self PID控制器指针
 * @param kd_yaw yaw方向微分系数
 */
void pid_incremental_set_kd_yaw(pid_incremental_t *self, float kd_yaw);

/**
 * @brief 设置yaw方向Kp和Kd值
 * @param self PID控制器指针
 * @param kp_yaw yaw方向比例系数
 * @param kd_yaw yaw方向微分系数
 */
void pid_incremental_set_kp_kd_yaw(pid_incremental_t *self, float kp_yaw, float kd_yaw);

/**
 * @brief 获取Kp值
 * @param self PID控制器指针
 * @return 当前Kp值
 */
float pid_incremental_get_kp(pid_incremental_t *self);

/**
 * @brief 获取Kd值
 * @param self PID控制器指针
 * @return 当前Kd值
 */
float pid_incremental_get_kd(pid_incremental_t *self);

/**
 * @brief 获取yaw方向Kp值
 * @param self PID控制器指针
 * @return 当前yaw方向Kp值
 */
float pid_incremental_get_kp_yaw(pid_incremental_t *self);

/**
 * @brief 获取yaw方向Kd值
 * @param self PID控制器指针
 * @return 当前yaw方向Kd值
 */
float pid_incremental_get_kd_yaw(pid_incremental_t *self);

/**
 * @brief 获取上次计算的总输出值
 * @param self PID控制器指针
 * @return 上次总输出值
 */
float pid_incremental_get_total_out(pid_incremental_t *self);

/**
 * @brief 获取上次yaw方向计算的总输出值
 * @param self PID控制器指针
 * @return 上次yaw方向总输出值
 */
float pid_incremental_get_total_out_yaw(pid_incremental_t *self);

/* ============================================================================
 * 位置式PID接口函数
 * ============================================================================ */

/**
 * @brief 初始化位置式PID控制器
 * @param self PID控制器指针
 */
void pid_positional_init(pid_positional_t *self);

/**
 * @brief 设置目标值和积分参数
 * @param self PID控制器指针
 * @param target 目标值
 * @param integral_limit 积分限幅值
 * @param i_band 积分分离值
 */
void pid_positional_init_target(pid_positional_t *self, float target, 
                                     float integral_limit, float i_band);

/**
 * @brief 计算位置式PID输出
 * @param self PID控制器指针
 * @param present 当前值
 * @return 计算后的输出值
 */
float pid_positional_calculate(pid_positional_t *self, float present);

/**
 * @brief 设置最大输出值
 * @param self PID控制器指针
 * @param max_output 最大输出值
 */
void pid_positional_set_max_output(pid_positional_t *self, float max_output);

/**
 * @brief 设置目标值
 * @param self PID控制器指针
 * @param target 目标值
 */
void pid_positional_set_target(pid_positional_t *self, float target);

/**
 * @brief 设置Kp值
 * @param self PID控制器指针
 * @param kp 比例系数
 */
void pid_positional_set_kp(pid_positional_t *self, float kp);

/**
 * @brief 设置Ki值
 * @param self PID控制器指针
 * @param ki 积分系数
 */
void pid_positional_set_ki(pid_positional_t *self, float ki);

/**
 * @brief 设置Kd值
 * @param self PID控制器指针
 * @param kd 微分系数
 */
void pid_positional_set_kd(pid_positional_t *self, float kd);

/**
 * @brief 设置Kp、Ki、Kd值
 * @param self PID控制器指针
 * @param kp 比例系数
 * @param ki 积分系数
 * @param kd 微分系数
 */
void pid_positional_set_kp_ki_kd(pid_positional_t *self, float kp, float ki, float kd);

/**
 * @brief 获取Kp值
 * @param self PID控制器指针
 * @return 当前Kp值
 */
float pid_positional_get_kp(pid_positional_t *self);

/**
 * @brief 获取Ki值
 * @param self PID控制器指针
 * @return 当前Ki值
 */
float pid_positional_get_ki(pid_positional_t *self);

/**
 * @brief 获取Kd值
 * @param self PID控制器指针
 * @return 当前Kd值
 */
float pid_positional_get_kd(pid_positional_t *self);

/**
 * @brief 获取目标值
 * @param self PID控制器指针
 * @return 当前目标值
 */
float pid_positional_get_target(pid_positional_t *self);

/**
 * @brief 获取上次计算的总输出值
 * @param self PID控制器指针
 * @return 上次总输出值
 */
float pid_positional_get_total_out(pid_positional_t *self);

/**
 * @brief 清除积分输出
 * @param self PID控制器指针
 */
void pid_positional_clear_i_out(pid_positional_t *self);

#ifdef __cplusplus
}
#endif

#endif /* __pid_H */
