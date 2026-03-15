/**
 * @file servo.h
 * @author canrad (1517807724@qq.com)
 * @brief pwm舵机驱动，支持MG996R等舵机
 * @version 0.1
 * @date 2026-01-10
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef DRIVER_SERVO_H
#define DRIVER_SERVO_H

#include "../util/datatype.h"

typedef struct servo_port{
    void (*pwm_start)(void* htim, u32 channel);
    void (*pwm_stop)(void* htim, u32 channel);
    void (*pwm_set_compare)(void* htim, u32 channel, u32 compare);
}servo_port_t;

void servo_bind_port(const servo_port_t* port);
bool servo_port_is_registered(void);

typedef struct servo
{
    void* htim;
    u32   channel;
    f32   angle;
} servo_t;

void servo_motor_enable(servo_t* servo);

void servo_motor_disable(servo_t* servo);

void servo_motor_set_angle(servo_t* servo, f32 angle);

void servo_motor_set_output(servo_t* servo, u32 compare);

#endif   // !DRIVER_SERVO_H
