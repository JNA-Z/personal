/**
 * @file bsp_adc_photo.h
 * @brief 光敏二极管ADC采样驱动
 * @note 负责ADC DMA采样、数据存储和基本滤波
 *       与业务逻辑解耦，可独立调试
 */

#ifndef __BSP_ADC_PHOTO_H
#define __BSP_ADC_PHOTO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ADC通道数量 */
#define BSP_ADC_PHOTO_CHANNELS    10

/* ============================================================================
 * 光敏二极管ADC对象结构体
 * ============================================================================ */
typedef struct {
    /* ADC原始数据 */
    uint16_t ad_raw[BSP_ADC_PHOTO_CHANNELS];    /* DMA采样原始值 */
    uint16_t ad_max[BSP_ADC_PHOTO_CHANNELS];    /* 历史最大值 */
    uint16_t ad_min[BSP_ADC_PHOTO_CHANNELS];    /* 历史最小值 */
    uint16_t ad_avr[BSP_ADC_PHOTO_CHANNELS];    /* 平均值 */
    
    /* 状态 */
    uint8_t initialized;                         /* 初始化标志 */
} bsp_adc_photo_t;

/* ============================================================================
 * 初始化函数
 * ============================================================================ */

/**
 * @brief 初始化光敏二极管ADC
 * @param self ADC对象指针
 */
void bsp_adc_photo_init(bsp_adc_photo_t *self);

/**
 * @brief 启动ADC DMA采样
 * @param self ADC对象指针
 */
void bsp_adc_photo_start(bsp_adc_photo_t *self);

/**
 * @brief 停止ADC DMA采样
 * @param self ADC对象指针
 */
void bsp_adc_photo_stop(bsp_adc_photo_t *self);

/* ============================================================================
 * 数据处理函数
 * ============================================================================ */

/**
 * @brief 更新最大最小平均值
 * @param self ADC对象指针
 * @note 应在主循环中周期调用
 */
void bsp_adc_photo_update(bsp_adc_photo_t *self);

/**
 * @brief 重置最大最小值
 * @param self ADC对象指针
 */
void bsp_adc_photo_reset_calibration(bsp_adc_photo_t *self);

/* ============================================================================
 * 数据获取函数
 * ============================================================================ */

/**
 * @brief 获取指定通道的原始ADC值
 * @param self ADC对象指针
 * @param channel 通道号 (0-9)
 * @return ADC原始值
 */
uint16_t bsp_adc_photo_get_raw(bsp_adc_photo_t *self, uint8_t channel);

/**
 * @brief 获取指定通道的最大值
 * @param self ADC对象指针
 * @param channel 通道号 (0-9)
 * @return 最大值
 */
uint16_t bsp_adc_photo_get_max(bsp_adc_photo_t *self, uint8_t channel);

/**
 * @brief 获取指定通道的最小值
 * @param self ADC对象指针
 * @param channel 通道号 (0-9)
 * @return 最小值
 */
uint16_t bsp_adc_photo_get_min(bsp_adc_photo_t *self, uint8_t channel);

/**
 * @brief 获取指定通道的平均值
 * @param self ADC对象指针
 * @param channel 通道号 (0-9)
 * @return 平均值
 */
uint16_t bsp_adc_photo_get_avr(bsp_adc_photo_t *self, uint8_t channel);

/**
 * @brief 获取原始数据数组指针（用于批量访问）
 * @param self ADC对象指针
 * @return 原始数据数组指针
 */
const uint16_t* bsp_adc_photo_get_raw_array(bsp_adc_photo_t *self);

/* ============================================================================
 * 判断函数
 * ============================================================================ */

/**
 * @brief 判断指定通道是否在白线上
 * @param self ADC对象指针
 * @param channel 通道号 (0-9)
 * @param threshold 白线阈值
 * @return 1: 在白线上, 0: 不在白线上
 */
uint8_t bsp_adc_photo_is_white(bsp_adc_photo_t *self, uint8_t channel, uint16_t threshold);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_ADC_PHOTO_H */
