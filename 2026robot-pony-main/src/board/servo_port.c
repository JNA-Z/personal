#include "servo_port.h"
#include "../driver/servo.h"

void servo_pwm_start(void* htim, u32 channel)
{
    //     HAL_TIM_PWM_Start(servo->htim, servo->channel);
}
void servo_pwm_stop(void* htim, u32 channel)
{
    // HAL_TIM_PWM_Stop(servo->htim, servo->channel);
}
void servo_pwm_set_compare(void* htim, u32 channel, u32 compare)
{
    // __HAL_TIM_SET_COMPARE(servo->htim, servo->channel, compare);
}

static const servo_port_t g_servo_port = {
    .pwm_start       = servo_pwm_start,
    .pwm_stop        = servo_pwm_stop,
    .pwm_set_compare = servo_pwm_set_compare,
};

void servo_port_init(void)
{
    servo_bind_port(&g_servo_port);
}
