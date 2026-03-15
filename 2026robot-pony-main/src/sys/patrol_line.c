/**
 * @file patrol_line.c
 * @brief 巡线补偿控制模块实现
 * @note ADC采样由 bsp_adc_photo 模块提供
 */

#include "patrol_line.h"
#include <stdlib.h>

/* ============================================================================
 * 初始化函数
 * ============================================================================ */

void patrol_line_init(patrol_line_t *self, bsp_adc_photo_t *adc)
{
    uint8_t i;
    
    /* 绑定ADC采样对象 */
    self->adc = adc;
    
    /* 初始化PID控制器 */
    pid_incremental_init(&self->pid);
    
    /* 初始化误差数组 */
    for (i = 0; i < PATROL_LINE_ADC_CHANNELS; i++) {
        self->error_avr[i] = 0;
    }
    
    /* 默认比较通道 */
    self->compare_ad_left = 4;
    self->compare_ad_right = 5;
    
    /* 初始化补偿值 */
    self->compensation = 0;
    self->yaw_compensation = 0.0f;
}

void patrol_line_set_compare_ad(patrol_line_t *self, uint8_t left, uint8_t right)
{
    self->compare_ad_left = left;
    self->compare_ad_right = right;
}

/* ============================================================================
 * PID参数设置函数
 * ============================================================================ */

void patrol_line_set_pid_max_output(patrol_line_t *self, float max_output)
{
    pid_incremental_set_max_output(&self->pid, max_output);
}

void patrol_line_set_pid_kp_kd(patrol_line_t *self, float kp, float kd)
{
    pid_incremental_set_kp_kd(&self->pid, kp, kd);
}

void patrol_line_set_pid_kp_kd_yaw(patrol_line_t *self, float kp, float kd)
{
    pid_incremental_set_kp_kd_yaw(&self->pid, kp, kd);
}

/* ============================================================================
 * yaw补偿函数
 * ============================================================================ */

void patrol_line_set_yaw_compensation(patrol_line_t *self, float yaw_angle)
{
    self->yaw_compensation = yaw_angle;
}

float patrol_line_get_yaw_compensation(patrol_line_t *self)
{
    return self->yaw_compensation;
}

/* ============================================================================
 * 误差计算函数
 * ============================================================================ */

int32_t patrol_line_get_error(patrol_line_t *self)
{
    int32_t res_avr = 0;
    int32_t res_total = 0;
    uint8_t i;
    
    if (self->adc == NULL) {
        return 0;
    }
    
    /* 获取原始ADC数据 */
    const uint16_t *ad_raw = bsp_adc_photo_get_raw_array(self->adc);
    
    /* 计算当前误差 */
    int32_t res = (int32_t)(ad_raw[self->compare_ad_left] - 
                            ad_raw[self->compare_ad_right]);
    
    /* 更新误差数组 (移位滤波) */
    self->error_avr[PATROL_LINE_ADC_CHANNELS - 1] = res;
    
    for (i = 0; i < PATROL_LINE_ADC_CHANNELS - 1; i++) {
        self->error_avr[i] = self->error_avr[i + 1];
        res_total += self->error_avr[i];
    }
    
    /* 计算平均值 */
    if (self->error_avr[0] != 0) {
        res_avr = res_total / PATROL_LINE_ADC_CHANNELS;
    }
    
    return res_avr;
}

float patrol_line_get_error_yaw(patrol_line_t *self, float yaw_angle)
{
    float yaw_error;
    
    yaw_error = yaw_angle - self->yaw_compensation;
    
    /* 处理角度跨越360度的情况 */
    if (yaw_error >= 270.0f && yaw_error <= 360.0f) {
        yaw_error = -1.0f * (self->yaw_compensation - yaw_angle + 360.0f);
    } else if (yaw_error <= -270.0f && yaw_error >= -360.0f) {
        yaw_error = yaw_angle - self->yaw_compensation + 360.0f;
    }
    
    return yaw_error;
}

int32_t patrol_line_get_ad_min_error(patrol_line_t *self)
{
    int32_t ad_min_error;
    
    if (self->adc == NULL) {
        return 0;
    }
    
    ad_min_error = bsp_adc_photo_get_min(self->adc, self->compare_ad_left) - 
                   bsp_adc_photo_get_min(self->adc, self->compare_ad_right);
    
    /* 当两边都在白线上时, 误差清零 */
    const uint16_t *ad_raw = bsp_adc_photo_get_raw_array(self->adc);
    if (ad_raw[self->compare_ad_left] >= 4090 && 
        ad_raw[self->compare_ad_right] >= 4090) {
        ad_min_error = 0;
    }
    
    return ad_min_error;
}

int32_t patrol_line_get_compensation_error(patrol_line_t *self)
{
    return patrol_line_get_error(self) - patrol_line_get_ad_min_error(self);
}

/* ============================================================================
 * 补偿计算函数
 * ============================================================================ */

int32_t patrol_line_calcu_compensation(patrol_line_t *self)
{
    self->compensation = (int32_t)pid_incremental_calculate(
        &self->pid, 
        (float)patrol_line_get_compensation_error(self)
    );
    
    return self->compensation;
}

int32_t patrol_line_yaw_compensation(patrol_line_t *self, float yaw_angle)
{
    self->compensation = (int32_t)pid_incremental_calculate_yaw(
        &self->pid, 
        patrol_line_get_error_yaw(self, yaw_angle)
    );
    
    return self->compensation;
}

int32_t patrol_line_ad_compensation(patrol_line_t *self)
{
    if (self->adc == NULL) {
        return 0;
    }
    
    const uint16_t *ad_raw = bsp_adc_photo_get_raw_array(self->adc);
    
    /* 根据不同ADC通道的值返回不同补偿 */
    if (ad_raw[0] > 2000) {
        return 150;
    } else if (ad_raw[9] > 2000) {
        return -150;
    } else if (ad_raw[1] > 2000) {
        return 120;
    } else if (ad_raw[8] > 2000) {
        return -120;
    } else if (ad_raw[2] > 2000) {
        return 90;
    } else if (ad_raw[7] > 2000) {
        return -90;
    } else if (ad_raw[3] > 2000) {
        return 50;
    } else if (ad_raw[6] > 2000) {
        return -50;
    }
    
    return 0;
}

int32_t patrol_line_read_compensation(patrol_line_t *self)
{
    return self->compensation;
}

int32_t patrol_line_lcd_show_compensation(patrol_line_t *self)
{
    if (self->compensation < 0) {
        return abs((int)self->compensation);
    }
    return self->compensation;
}

/* ============================================================================
 * 判断函数
 * ============================================================================ */

uint8_t patrol_line_compare_ad(patrol_line_t *self)
{
    if (self->adc == NULL) {
        return 0;
    }
    
    const uint16_t *ad_raw = bsp_adc_photo_get_raw_array(self->adc);
    
    if (ad_raw[self->compare_ad_left] >= ad_raw[self->compare_ad_right]) {
        return 0;   /* 左边高 */
    }
    return 1;       /* 右边高 */
}

uint8_t patrol_line_ad_if_white(patrol_line_t *self, uint8_t channel)
{
    if (self->adc == NULL || channel >= PATROL_LINE_ADC_CHANNELS) {
        return 0;
    }
    
    /* 使用阈值2000判断白线 */
    return bsp_adc_photo_is_white(self->adc, channel, 2000);
}

int32_t patrol_line_ad_circle_compensation(patrol_line_t *self)
{
    if (self->adc == NULL) {
        return 0;
    }
    
    const uint16_t *ad_raw = bsp_adc_photo_get_raw_array(self->adc);
    
    /* 曲线偏心补偿 */
    if (ad_raw[0] > 2000) {
        return 100;
    } else if (ad_raw[1] > 2000) {
        return 50;
    } else if (ad_raw[4] > 2000) {
        return -50;
    } else if (ad_raw[5] > 2000) {
        return -100;
    } else if (ad_raw[6] > 2000) {
        return -100;
    } else if (ad_raw[7] > 2000) {
        return -100;
    } else if (ad_raw[8] > 2000) {
        return -100;
    } else if (ad_raw[9] > 2000) {
        return -100;
    }
    
    return -100;
}

/* ============================================================================
 * 参数获取函数
 * ============================================================================ */

float patrol_line_get_pid_kp(patrol_line_t *self)
{
    return pid_incremental_get_kp(&self->pid);
}

float patrol_line_get_pid_kd(patrol_line_t *self)
{
    return pid_incremental_get_kd(&self->pid);
}
