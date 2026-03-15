/**
 * @file patrol_line.h
 * @brief 巡线补偿控制模块
 * @note 重构说明：
 *       - ADC采样已分离到 bsp_adc_photo 模块
 *       - 本模块专注于误差计算和PID补偿
 *       - 通过指针接收ADC数据，解耦硬件依赖
 */

#ifndef __PATROL_LINE_H
#define __PATROL_LINE_H

#include <stdint.h>
#include "pid.h"
#include "bsp_adc_photo.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ADC通道数量（与bsp_adc_photo保持一致） */
#define PATROL_LINE_ADC_CHANNELS    BSP_ADC_PHOTO_CHANNELS

/* ============================================================================
 * 巡线控制器结构体
 * ============================================================================ */
typedef struct {
    /* PID控制器 (增量式) */
    pid_incremental_t pid;
    
    /* ADC数据指针（由外部提供） */
    bsp_adc_photo_t *adc;                    /* ADC采样对象指针 */
    
    /* 比较用的ADC通道 */
    uint8_t compare_ad_left;                 /* 左侧比较通道 */
    uint8_t compare_ad_right;                /* 右侧比较通道 */
    
    /* 误差平均滤波 */
    int32_t error_avr[PATROL_LINE_ADC_CHANNELS]; /* 误差平均值数组 */
    
    /* 补偿输出 */
    int32_t compensation;                    /* 补偿值 */
    
    /* yaw补偿 */
    float yaw_compensation;                  /* yaw补偿基准值 */
} patrol_line_t;

/* ============================================================================
 * 初始化函数
 * ============================================================================ */

/**
 * @brief 初始化巡线控制器
 * @param self 巡线控制器指针
 * @param adc ADC采样对象指针
 */
void patrol_line_init(patrol_line_t *self, bsp_adc_photo_t *adc);

/**
 * @brief 设置比较ADC通道
 * @param self 巡线控制器指针
 * @param left 左侧通道号
 * @param right 右侧通道号
 */
void patrol_line_set_compare_ad(patrol_line_t *self, uint8_t left, uint8_t right);

/* ============================================================================
 * PID参数设置函数
 * ============================================================================ */

/**
 * @brief 设置PID最大输出
 * @param self 巡线控制器指针
 * @param max_output 最大输出值
 */
void patrol_line_set_pid_max_output(patrol_line_t *self, float max_output);

/**
 * @brief 设置光敏二极管PID参数
 * @param self 巡线控制器指针
 * @param kp 比例系数
 * @param kd 微分系数
 */
void patrol_line_set_pid_kp_kd(patrol_line_t *self, float kp, float kd);

/**
 * @brief 设置陀螺仪PID参数
 * @param self 巡线控制器指针
 * @param kp yaw方向比例系数
 * @param kd yaw方向微分系数
 */
void patrol_line_set_pid_kp_kd_yaw(patrol_line_t *self, float kp, float kd);

/* ============================================================================
 * yaw补偿函数
 * ============================================================================ */

/**
 * @brief 设置yaw补偿基准值
 * @param self 巡线控制器指针
 * @param yaw_angle 当前yaw角度
 */
void patrol_line_set_yaw_compensation(patrol_line_t *self, float yaw_angle);

/**
 * @brief 获取yaw补偿基准值
 * @param self 巡线控制器指针
 * @return yaw补偿基准值
 */
float patrol_line_get_yaw_compensation(patrol_line_t *self);

/* ============================================================================
 * 误差计算函数
 * ============================================================================ */

/**
 * @brief 获取ADC动态误差 (带平均滤波)
 * @param self 巡线控制器指针
 * @return 误差平均值
 */
int32_t patrol_line_get_error(patrol_line_t *self);

/**
 * @brief 获取yaw误差
 * @param self 巡线控制器指针
 * @param yaw_angle 当前yaw角度
 * @return yaw误差值
 */
float patrol_line_get_error_yaw(patrol_line_t *self, float yaw_angle);

/**
 * @brief 获取ADC静态误差 (最小值差值)
 * @param self 巡线控制器指针
 * @return ADC最小值差值
 */
int32_t patrol_line_get_ad_min_error(patrol_line_t *self);

/**
 * @brief 获取补偿后的误差
 * @param self 巡线控制器指针
 * @return 补偿后误差 = 动态误差 - 静态误差
 */
int32_t patrol_line_get_compensation_error(patrol_line_t *self);

/* ============================================================================
 * 补偿计算函数
 * ============================================================================ */

/**
 * @brief 计算光敏二极管误差并返回补偿
 * @param self 巡线控制器指针
 * @return PID补偿值
 */
int32_t patrol_line_calcu_compensation(patrol_line_t *self);

/**
 * @brief 计算陀螺仪误差并返回补偿
 * @param self 巡线控制器指针
 * @param yaw_angle 当前yaw角度
 * @return PID补偿值
 */
int32_t patrol_line_yaw_compensation(patrol_line_t *self, float yaw_angle);

/**
 * @brief 获取AD补偿值
 * @param self 巡线控制器指针
 * @return 根据ADC值返回对应的补偿值
 */
int32_t patrol_line_ad_compensation(patrol_line_t *self);

/**
 * @brief 读取当前补偿值
 * @param self 巡线控制器指针
 * @return 当前补偿值
 */
int32_t patrol_line_read_compensation(patrol_line_t *self);

/**
 * @brief 获取补偿值的绝对值（用于LCD显示）
 * @param self 巡线控制器指针
 * @return 补偿值的绝对值
 */
int32_t patrol_line_lcd_show_compensation(patrol_line_t *self);

/* ============================================================================
 * 判断函数
 * ============================================================================ */

/**
 * @brief 判断ADC左右大小
 * @param self 巡线控制器指针
 * @return 0: 左边高, 1: 右边高
 */
uint8_t patrol_line_compare_ad(patrol_line_t *self);

/**
 * @brief 判断指定通道是否在白线上
 * @param self 巡线控制器指针
 * @param channel ADC通道号
 * @return 1: 在白线上, 0: 不在白线上
 */
uint8_t patrol_line_ad_if_white(patrol_line_t *self, uint8_t channel);

/**
 * @brief 获取曲线偏心AD补偿值
 * @param self 巡线控制器指针
 * @return 曲线补偿值
 */
int32_t patrol_line_ad_circle_compensation(patrol_line_t *self);

/* ============================================================================
 * 参数获取函数
 * ============================================================================ */

/**
 * @brief 获取PID的Kp值
 * @param self 巡线控制器指针
 * @return Kp值
 */
float patrol_line_get_pid_kp(patrol_line_t *self);

/**
 * @brief 获取PID的Kd值
 * @param self 巡线控制器指针
 * @return Kd值
 */
float patrol_line_get_pid_kd(patrol_line_t *self);

#ifdef __cplusplus
}
#endif

#endif /* __PATROL_LINE_H */
