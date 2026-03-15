/**
 * @file bsp_encoder.h
 * @brief 编码器驱动模块 (纯粹化版本)
 * @note 硬件配置在model层静态初始化，此模块只关注逻辑
 *       每个编码器是独立的对象
 */

#ifndef __BSP_ENCODER_H
#define __BSP_ENCODER_H

#include <stdint.h>
#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 使用main.h中定义的编码器参数 */
#ifndef ENCODER_READ_RESOLUTION
#define ENCODER_READ_RESOLUTION     1320.0f
#endif

#ifndef REDUCTION_RATIO
#define REDUCTION_RATIO             30.0f
#endif

/* 时间系数 */
#define ENCODER_TIME_VALUE          5

/**
 * @brief 编码器方向枚举
 */
typedef enum {
    ENCODER_DIR_REVERSE = 0,    /* 反转 (向下计数) */
    ENCODER_DIR_FORWARD = 1     /* 正转 (向上计数) */
} encoder_dir_t;

/**
 * @brief 编码器对象结构体
 * @note 硬件配置(htim)应在定义时静态初始化
 */
typedef struct {
    TIM_HandleTypeDef *htim;            /* 定时器句柄 (静态初始化) */
    volatile int32_t capture_count;     /* 当前捕获值 */
    volatile int32_t last_count;        /* 上次捕获值 */
    volatile int16_t overflow_count;    /* 溢出计数 */
    volatile float shaft_speed;         /* 转轴转速 (转/秒) */
    volatile encoder_dir_t direction;   /* 方向状态 */
} bsp_encoder_t;

/**
 * @brief 初始化编码器对象运行时状态
 * @param self 编码器对象指针
 */
void bsp_encoder_init(bsp_encoder_t *self);

/**
 * @brief 启动编码器硬件
 * @param self 编码器对象指针
 */
void bsp_encoder_start(bsp_encoder_t *self);

/**
 * @brief 停止编码器
 * @param self 编码器对象指针
 */
void bsp_encoder_stop(bsp_encoder_t *self);

/**
 * @brief 清零编码器数据
 * @param self 编码器对象指针
 */
void bsp_encoder_clear(bsp_encoder_t *self);

/**
 * @brief 处理编码器溢出中断
 * @param self 编码器对象指针
 * @note 在定时器中断中调用
 */
void bsp_encoder_overflow_handler(bsp_encoder_t *self);

/**
 * @brief 获取捕获值
 * @param self 编码器对象指针
 * @return 捕获值
 */
int32_t bsp_encoder_get_count(bsp_encoder_t *self);

/**
 * @brief 获取方向
 * @param self 编码器对象指针
 * @return 方向状态
 */
encoder_dir_t bsp_encoder_get_direction(bsp_encoder_t *self);

/**
 * @brief 获取转轴速度 (转/秒)
 * @param self 编码器对象指针
 * @return 转轴速度
 */
float bsp_encoder_get_shaft_speed(bsp_encoder_t *self);

/**
 * @brief 获取输出轴速度 (转/秒)
 * @param self 编码器对象指针
 * @return 输出轴速度
 */
float bsp_encoder_get_output_shaft_speed(bsp_encoder_t *self);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_ENCODER_H */
