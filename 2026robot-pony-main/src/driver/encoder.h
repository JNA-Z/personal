/**
 * @file encoder.h
 * @author Easy (1609010708@qq.com)
 * @brief Encoder driver: supports quadrature (timer encoder) and single-pulse (freq/period) modes
 * @version 0.3
 * @date 2026-01-24
 *
 * @copyright Copyright (c) 2026
 */
 
#ifndef DRIVER_ENCODER_H
#define DRIVER_ENCODER_H

#include "../util/datatype.h"
#include "debug.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ENCODER_PULSES_PER_REV 13  // Pulses Per Revolution
#define ENCODER_DECODE_FACTOR 4    // 1,2 or 4 (quadrature decoding)


typedef struct encoder_port_t {
    void (*tim_base_start)(void* tim);
    void (*tim_start)(void* tim, u32 channel);
    u32 (*read_tim_cnt)(void* tim, u32 channel);          
    void (*clear_tim_cnt)(void* tim, u32 channel);        
} encoder_port_t;

// bind port
void encoder_bind_port(const encoder_port_t* port);
bool encoder_port_is_registered(void);



typedef enum encoder_mode{
    // 正交编码器模式
    ENCODER_MODE_QUADRATURE = 0,      
    // 单脉冲计数模式，给高频信号使用
    ENCODER_MODE_SINGLE_M,        
    // 测时间，给低频信号使用
    ENCODER_MODE_SINGLE_T,         
} encoder_mode_t;

typedef struct encoder_t {
    // 编码器模式
    encoder_mode_t mode;
    // 编码器定时器句柄
    void* encoder_tim;      
    // 编码器使用的通道[*ENCODER_MODE_SINGLE_M模式下该参数无效]
    u32 channel;    
    // 门控定时器句柄[*ENCODER_MODE_SINGLE_T模式下非必须，该指针可为NULL]
    void* gate_tim;
    // 采样时间(ms)[*在ENCODER_MODE_SINGLE_T下该参数为encoder_tim的周期]
    f32 sample_ms;    
    // 编码器极性(推荐1或-1) 
    f32 polarity;

    i32 value;   
    i32 prev_value;
} encoder_t;


void encoder_init(encoder_t* self);
u32 encoder_get_count(encoder_t* self);
void encoder_irq(encoder_t* self);

#ifdef __cplusplus
}
#endif

#endif  // DRIVER_ENCODER_H
