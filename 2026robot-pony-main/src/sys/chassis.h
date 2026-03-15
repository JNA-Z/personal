/**
 * @file chassis.h
 * @brief 底盘控制器 (单例模式)
 * @note 封装双电机组合，提供高层运动控制接口
 *       无需传递对象指针，直接使用单例
 */

#ifndef __CHASSIS_H
#define __CHASSIS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * 初始化与使能
 * ============================================================================ */

/**
 * @brief 初始化底盘
 */
void chassis_init(void);

/**
 * @brief 使能底盘电机
 */
void chassis_enable(void);

/**
 * @brief 失能底盘电机
 */
void chassis_disable(void);

/* ============================================================================
 * 速度控制
 * ============================================================================ */

/**
 * @brief 设置双轮相同速度
 * @param speed 速度值
 */
void chassis_set_speed(int16_t speed);

/**
 * @brief 设置左右轮不同速度
 * @param left_speed 左轮速度
 * @param right_speed 右轮速度
 */
void chassis_set_speed_lr(int16_t left_speed, int16_t right_speed);

/**
 * @brief 获取左轮实时速度
 * @return 左轮速度
 */
int16_t chassis_get_left_speed(void);

/**
 * @brief 获取右轮实时速度
 * @return 右轮速度
 */
int16_t chassis_get_right_speed(void);

/* ============================================================================
 * 高级运动控制
 * ============================================================================ */

/**
 * @brief 直线前进
 * @param speed 速度
 */
void chassis_run_straight(int16_t speed);

/**
 * @brief 后退
 * @param speed 速度
 */
void chassis_run_back(int16_t speed);

/**
 * @brief 停止
 */
void chassis_run_stop(void);

/**
 * @brief 右转
 * @param speed 速度
 */
void chassis_turn_right(int16_t speed);

/**
 * @brief 左转
 * @param speed 速度
 */
void chassis_turn_left(int16_t speed);

/**
 * @brief 右转带比例
 * @param speed 速度
 * @param proportion 左右轮速度比 (-1.0 ~ 1.0)
 */
void chassis_turn_right_ratio(int16_t speed, float proportion);

/**
 * @brief 左转带比例
 * @param speed 速度
 * @param proportion 左右轮速度比 (-1.0 ~ 1.0)
 */
void chassis_turn_left_ratio(int16_t speed, float proportion);

/* ============================================================================
 * 巡线控制
 * ============================================================================ */

/**
 * @brief 普通巡线(巡白线)
 * @param speed 基础速度
 * @param compensation 补偿值
 */
void chassis_patrol_white(int16_t speed, int32_t compensation);

/**
 * @brief 后退巡线
 * @param speed 基础速度
 * @param compensation 补偿值
 */
void chassis_patrol_back(int16_t speed, int32_t compensation);

/**
 * @brief 距离巡线
 * @param speed 基础速度
 * @param distance 距离速度补偿
 * @param compensation 补偿值
 */
void chassis_patrol_distance(int16_t speed, int32_t distance, int32_t compensation);

/**
 * @brief 距离巡线(带光电保护)
 * @param speed 基础速度
 * @param distance 距离速度补偿
 * @param compensation 补偿值
 * @param protect 光电补偿量
 */
void chassis_patrol_distance_protect(int16_t speed, int32_t distance, 
                                      int32_t compensation, int32_t protect);

/**
 * @brief 距离巡线2(带光电保护)
 * @param speed 基础速度
 * @param distance 距离速度补偿
 * @param compensation 补偿值
 * @param protect 光电补偿量
 */
void chassis_patrol_distance_protect2(int16_t speed, int32_t distance, 
                                       int32_t compensation, int32_t protect);

/**
 * @brief 距离巡线3
 * @param speed 基础速度
 * @param distance 距离速度补偿
 * @param compensation 补偿值
 * @param protect 光电补偿量
 */
void chassis_patrol_distance_protect3(int16_t speed, int32_t distance, 
                                       int32_t compensation, int32_t protect);

#ifdef __cplusplus
}
#endif

#endif /* __CHASSIS_H */
