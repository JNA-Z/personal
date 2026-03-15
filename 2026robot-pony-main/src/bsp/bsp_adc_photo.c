/**
 * @file bsp_adc_photo.c
 * @brief 光敏二极管ADC采样驱动实现
 */

#include "bsp_adc_photo.h"
#include "adc.h"

/* ============================================================================
 * 初始化函数
 * ============================================================================ */

void bsp_adc_photo_init(bsp_adc_photo_t *self)
{
    uint8_t i;
    
    for (i = 0; i < BSP_ADC_PHOTO_CHANNELS; i++) {
        self->ad_raw[i] = 0;
        self->ad_max[i] = 0;
        self->ad_min[i] = 4095;  /* 12位ADC最大值 */
        self->ad_avr[i] = 2048;  /* 中间值 */
    }
    
    self->initialized = 1;
}

void bsp_adc_photo_start(bsp_adc_photo_t *self)
{
    /* 启动ADC DMA采样，数据直接存入ad_raw数组 */
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)self->ad_raw, BSP_ADC_PHOTO_CHANNELS);
}

void bsp_adc_photo_stop(bsp_adc_photo_t *self)
{
    HAL_ADC_Stop_DMA(&hadc1);
}

/* ============================================================================
 * 数据处理函数
 * ============================================================================ */

void bsp_adc_photo_update(bsp_adc_photo_t *self)
{
    uint8_t i;
    
    for (i = 0; i < BSP_ADC_PHOTO_CHANNELS; i++) {
        /* 更新最小值（忽略0值） */
        if (self->ad_raw[i] < self->ad_min[i] && self->ad_raw[i] != 0) {
            self->ad_min[i] = self->ad_raw[i];
        }
        
        /* 更新最大值 */
        if (self->ad_raw[i] > self->ad_max[i]) {
            self->ad_max[i] = self->ad_raw[i];
        }
        
        /* 计算平均值 */
        self->ad_avr[i] = (self->ad_max[i] + self->ad_min[i]) / 2;
    }
}

void bsp_adc_photo_reset_calibration(bsp_adc_photo_t *self)
{
    uint8_t i;
    
    for (i = 0; i < BSP_ADC_PHOTO_CHANNELS; i++) {
        self->ad_max[i] = 0;
        self->ad_min[i] = 4095;
        self->ad_avr[i] = 2048;
    }
}

/* ============================================================================
 * 数据获取函数
 * ============================================================================ */

uint16_t bsp_adc_photo_get_raw(bsp_adc_photo_t *self, uint8_t channel)
{
    if (channel >= BSP_ADC_PHOTO_CHANNELS) {
        return 0;
    }
    return self->ad_raw[channel];
}

uint16_t bsp_adc_photo_get_max(bsp_adc_photo_t *self, uint8_t channel)
{
    if (channel >= BSP_ADC_PHOTO_CHANNELS) {
        return 0;
    }
    return self->ad_max[channel];
}

uint16_t bsp_adc_photo_get_min(bsp_adc_photo_t *self, uint8_t channel)
{
    if (channel >= BSP_ADC_PHOTO_CHANNELS) {
        return 0;
    }
    return self->ad_min[channel];
}

uint16_t bsp_adc_photo_get_avr(bsp_adc_photo_t *self, uint8_t channel)
{
    if (channel >= BSP_ADC_PHOTO_CHANNELS) {
        return 0;
    }
    return self->ad_avr[channel];
}

const uint16_t* bsp_adc_photo_get_raw_array(bsp_adc_photo_t *self)
{
    return self->ad_raw;
}

/* ============================================================================
 * 判断函数
 * ============================================================================ */

uint8_t bsp_adc_photo_is_white(bsp_adc_photo_t *self, uint8_t channel, uint16_t threshold)
{
    if (channel >= BSP_ADC_PHOTO_CHANNELS) {
        return 0;
    }
    return self->ad_raw[channel] > threshold ? 1 : 0;
}
