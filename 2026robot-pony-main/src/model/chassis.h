/**
 * @file chassis.h
 * @author canrad (1517807724@qq.com)
 * @brief 小车运动控制模型
 * 通过driver和board实现对小车的控制
 * @version 0.1
 * @date 2026-01-11
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef MODEL_CHASSIS_H
#define MODEL_CHASSIS_H

#include "../util/datatype.h"

#define OPEN_CONTROL 1
#define CLOSE_CONTROL 0

typedef struct pid {
    i32 kp;
    i32 kd;
    i32 last_error;
    i32 error;
    i32 MaxOutput;
} pid_t;

void chassis_init(void);
void chassis_set_speed(i32 left_speed ,i32 right_speed);
void chassis_enable(void);
void chassis_disable(void);
void chassis_pid(i32 left_speed ,i32 right_speed ,u32 tick);
void chassis_stop(void);
void chassis_Open_Or_Close(u8 control_state,i32 left_speed ,i32 right_speed ,u32 tick);
u32 chassis_get_left_speed(u32 tick);
u32 chassis_get_right_speed(u32 tick);
void chassis_turn_left(i32 speed,u32 tick);
void chassis_turn_right(i32 speed,u32 tick);
void chassis_straight(i32 speed,u32 tick);

#endif // MODEL_CHASSIS_H
