/**
 * @file distance_patrol.h
 * @brief 距离巡逻模块 (纯C实现)
 * @note 使用encoder对象和chassis单例
 */

#ifndef __DISTANCE_PATROL_H
#define __DISTANCE_PATROL_H

#include <stdint.h>
#include "pid.h"
#include "bsp_encoder.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * 距离巡逻模块结构体
 * ============================================================================ */
typedef struct {
    /* 位置式PID控制器 */
    pid_positional_t pid;
    
    /* 控制编码器指针 */
    bsp_encoder_t *encoder;
} distance_patrol_t;

/* ============================================================================
 * 距离巡逻模块接口函数
 * ============================================================================ */

/**
 * @brief 初始化距离巡逻模块
 * @param self 距离巡逻模块指针
 * @param encoder 编码器指针
 */
void distance_patrol_init(distance_patrol_t *self, bsp_encoder_t *encoder);

/**
 * @brief 位置巡线初始化
 * @param self 距离巡逻模块指针
 * @param target 目标值 (编码值)
 * @param integral_limit 积分限幅值
 * @param i_band 积分分离值
 */
void distance_patrol_init_target(distance_patrol_t *self, 
                                 float target, 
                                 float integral_limit, 
                                 float i_band);

/**
 * @brief 编码器硬件初始化
 * @param self 距离巡逻模块指针
 */
void distance_patrol_encoder_init(distance_patrol_t *self);

/**
 * @brief 设置位置巡线PID参数
 * @param self 距离巡逻模块指针
 * @param kp 比例系数
 * @param ki 积分系数
 * @param kd 微分系数
 */
void distance_patrol_set_kp_ki_kd(distance_patrol_t *self, float kp, float ki, float kd);

/**
 * @brief 设置电机运动位置参数并使能编码器
 * @param self 距离巡逻模块指针
 * @param distance 目标距离 (编码值)
 * @param integral_limit 积分限幅值
 * @param i_band 积分分离值
 */
void distance_patrol_move_distance(distance_patrol_t *self, 
                                   int32_t distance, 
                                   float integral_limit, 
                                   float i_band);

/**
 * @brief 得到加速补偿速度
 * @param self 距离巡逻模块指针
 * @return 补偿速度值
 */
int16_t distance_patrol_get_calculate_speed(distance_patrol_t *self);

/**
 * @brief 获得Kp值
 */
float distance_patrol_get_kp(distance_patrol_t *self);

/**
 * @brief 获得Ki值
 */
float distance_patrol_get_ki(distance_patrol_t *self);

/**
 * @brief 获得Kd值
 */
float distance_patrol_get_kd(distance_patrol_t *self);

/**
 * @brief 获得当前编码值
 * @param self 距离巡逻模块指针
 * @return 编码值
 */
int32_t distance_patrol_get_count(distance_patrol_t *self);

/**
 * @brief 获得当前右轮编码值 (兼容接口)
 * @param self 距离巡逻模块指针
 * @return 右轮编码值
 */
int32_t distance_patrol_get_capture_right_count(distance_patrol_t *self);

/**
 * @brief 获得当前左轮编码值 (兼容接口)
 * @param self 距离巡逻模块指针
 * @return 左轮编码值
 */
int32_t distance_patrol_get_capture_left_count(distance_patrol_t *self);

/**
 * @brief 行驶固定编码值停止
 * @param self 距离巡逻模块指针
 * @param encoder_value 目标编码值
 * @return 1: 已停止, 0: 工作中
 */
uint8_t distance_patrol_run_distance(distance_patrol_t *self, int32_t encoder_value);

/**
 * @brief 行驶固定编码值停止 (右轮兼容接口)
 * @param self 距离巡逻模块指针
 * @param encoder_value 目标编码值
 * @return 1: 已停止, 0: 工作中
 * @note 兼容旧API
 */
uint8_t distance_patrol_run_right_distance(distance_patrol_t *self, int32_t encoder_value);

/**
 * @brief 行驶固定编码值停止 (左轮兼容接口)
 * @param self 距离巡逻模块指针
 * @param encoder_value 目标编码值
 * @return 1: 已停止, 0: 工作中
 * @note 兼容旧API
 */
uint8_t distance_patrol_run_left_distance(distance_patrol_t *self, int32_t encoder_value);

/**
 * @brief 设置最大补偿
 * @param self 距离巡逻模块指针
 * @param max_output 最大输出值
 */
void distance_patrol_set_max_output(distance_patrol_t *self, float max_output);

#ifdef __cplusplus
}
#endif

#endif /* __DISTANCE_PATROL_H */
