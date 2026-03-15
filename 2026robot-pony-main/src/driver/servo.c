#include "servo.h"

static servo_port_t* g_servo_port = NULL;

void servo_bind_port(const servo_port_t* port)
{
    g_servo_port = (servo_port_t*)port;
}

bool servo_port_is_registered(void)
{
    return (g_servo_port != NULL);
}

void servo_motor_enable(servo_t* servo)
{
    g_servo_port->pwm_start(servo->htim, servo->channel);
}

void servo_motor_disable(servo_t* servo)
{
    g_servo_port->pwm_stop(servo->htim, servo->channel);
}

void servo_motor_set_angle(servo_t* servo, f32 angle)
{
    if (angle < 0) { angle = 0; }
    if (angle > 180) { angle = 180; }

    g_servo_port->pwm_set_compare(servo->htim, servo->channel, (u32)(500 + (angle / 180.0f) * 2000));

    servo->angle = angle;
}

void servo_motor_set_output(servo_t* servo, u32 compare)
{
    g_servo_port->pwm_set_compare(servo->htim, servo->channel, compare);
}
