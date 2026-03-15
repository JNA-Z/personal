/**
 * @file chassis.c
 * @brief 底盘控制器实现 (单例模式)
 */

#include "chassis.h"
#include "model.h"

void chassis_init(void)
{
    bsp_motor_init(&motor_left, 1000);
    bsp_motor_init(&motor_right, 1000);
}

void chassis_enable(void)
{
    bsp_motor_enable(&motor_left);
    bsp_motor_enable(&motor_right);
}

void chassis_disable(void)
{
    bsp_motor_disable(&motor_left);
    bsp_motor_disable(&motor_right);
}

void chassis_set_speed(int16_t speed)
{
    bsp_motor_set_speed(&motor_left, speed);
    bsp_motor_set_speed(&motor_right, speed);
}

void chassis_set_speed_lr(int16_t left_speed, int16_t right_speed)
{
    bsp_motor_set_speed(&motor_left, left_speed);
    bsp_motor_set_speed(&motor_right, right_speed);
}

int16_t chassis_get_left_speed(void)
{
    return bsp_motor_get_speed(&motor_left);
}

int16_t chassis_get_right_speed(void)
{
    return bsp_motor_get_speed(&motor_right);
}

/* ========== 高级运动控制函数 ========== */

void chassis_run_straight(int16_t speed)
{
    chassis_set_speed(speed);
}

void chassis_run_back(int16_t speed)
{
    chassis_set_speed_lr(-speed, -speed);
}

void chassis_run_stop(void)
{
    chassis_set_speed(0);
}

void chassis_turn_right(int16_t speed)
{
    chassis_set_speed_lr(speed, -speed);
}

void chassis_turn_left(int16_t speed)
{
    chassis_set_speed_lr(-speed, speed);
}

void chassis_turn_right_ratio(int16_t speed, float proportion)
{
    chassis_set_speed_lr(speed, (int16_t)(proportion * speed));
}

void chassis_turn_left_ratio(int16_t speed, float proportion)
{
    chassis_set_speed_lr((int16_t)(proportion * speed), speed);
}

/* ========== 巡线控制函数 ========== */

void chassis_patrol_white(int16_t speed, int32_t compensation)
{
    chassis_set_speed_lr((int16_t)(speed - compensation), 
                         (int16_t)(speed + compensation));
}

void chassis_patrol_back(int16_t speed, int32_t compensation)
{
    chassis_set_speed_lr((int16_t)(-speed - compensation), 
                         (int16_t)(-speed + compensation));
}

void chassis_patrol_distance(int16_t speed, int32_t distance, int32_t compensation)
{
    chassis_set_speed_lr((int16_t)(speed + distance + compensation), 
                         (int16_t)(speed + distance - compensation));
}

void chassis_patrol_distance_protect(int16_t speed, int32_t distance, 
                                      int32_t compensation, int32_t protect)
{
    chassis_set_speed_lr((int16_t)(speed + distance + compensation - protect), 
                         (int16_t)(speed + distance - compensation + protect));
}

void chassis_patrol_distance_protect2(int16_t speed, int32_t distance, 
                                       int32_t compensation, int32_t protect)
{
    chassis_set_speed_lr((int16_t)(speed - distance + compensation - protect), 
                         (int16_t)(speed - distance - compensation + protect));
}

void chassis_patrol_distance_protect3(int16_t speed, int32_t distance, 
                                       int32_t compensation, int32_t protect)
{
    chassis_set_speed_lr((int16_t)(speed - distance - compensation + protect), 
                         (int16_t)(speed - distance + compensation - protect));
}
