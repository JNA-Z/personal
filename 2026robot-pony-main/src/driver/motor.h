/**
 * @file motor.h
 * @author Easy (1609010708@qq.com)
 * @brief motor驱动，port是否初始化由适配层保证，否则会有空指针风险
 * @version 0.1
 * @date 2026-01-11
 * 
 * @copyright Copyright (c) 2026
 * 
 */
 
#ifndef DRIVER_MOTOR_H
#define DRIVER_MOTOR_H

#include "../util/datatype.h"
#include "encoder.h"
#include "debug.h"

// gear ratio = 30:1
#define MOTOR_GEAR_REDUCTION 35

// wheel diameter 65 mm
#define MOTOR_WHEEL_DIAM_MM 65

typedef struct motor{
	void* gpio1;// 电机方向控制引脚1
    u16 pin1;
	void* gpio2;// 电机方向控制引脚2
    u16 pin2;
	void* htim;
	u32 channel;	
	i32 polarity;// 电机极性（1/-1）用以匹配设置方向与实际方向

	encoder_t* encoder;
	f32 cnt_to_mmps_factor;
} motor_t;

//port
typedef struct motor_port{
	void (*write_pin)(void* gpio, u16 pin, u8 value);
	void (*set_speed)(void* htim, u32 channel, u32 speed);
	void (*start)(void* htim, u32 channel);
	void (*stop)(void* htim, u32 channel);
} motor_port_t;

// 绑定port
void motor_bind_port(const motor_port_t* port);

// api

// 初始化motor
void motor_init(motor_t* motor);
// 电机使能
void motor_enable(motor_t* motor);
// 电机失能
void motor_disable(motor_t* motor);
// 设置电机速度
void motor_set_speed(motor_t* motor, i32 speed);
// 获取电机速度
f32 motor_get_speed(motor_t* motor);



#endif // !DRIVER_MOTOR_H
