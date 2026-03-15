#include "motor.h"

static motor_port_t* g_motor_port = NULL;

void motor_bind_port(const motor_port_t* port)
{
    g_motor_port = (motor_port_t*)port;
}

static f32 cul_counts_to_mps_factor(const motor_t* motor);

/**
 * @brief 电机初始化
 * 
 * @param motor 电机句柄
 */
void motor_init(motor_t* motor)
{
	if (!motor->encoder) {
        debug_print("[motor] warning: no encoder attached, speed can not be measured\n");
        return;
    }
	// 计算速度因子并缓存，方便后续读出速度时的计算
	motor->cnt_to_mmps_factor = cul_counts_to_mps_factor(motor);
	// 默认清零电机速度并启动
	g_motor_port->set_speed(motor->htim, motor->channel, 0);
	g_motor_port->start(motor->htim, motor->channel);
}

/**
 * @brief 电机使能
 * 
 * @param motor 
 */
void motor_enable(motor_t* motor)
{
	g_motor_port->start(motor->htim, motor->channel);
}

/**
 * @brief 电机失能
 * 
 * @param motor 
 */
void motor_disable(motor_t* motor)
{
	g_motor_port->stop(motor->htim, motor->channel);
}

/**
 * @brief 设置电机速度
 * 
 * @param motor 
 * @param speed 速度(speed>0正转，speed<0反转，若实际转向与设定不符，可通过调整polarity参数解决)
 */
void motor_set_speed(motor_t* motor, i32 speed)
{
	u32 abs_speed = speed >= 0 ? speed : -speed;
	if (speed * motor->polarity >= 0) {
		// 正转
		g_motor_port->write_pin(motor->gpio1, motor->pin1, 1);
		g_motor_port->write_pin(motor->gpio2, motor->pin2, 0);
	} else {
		// 反转
		g_motor_port->write_pin(motor->gpio1, motor->pin1, 0);
		g_motor_port->write_pin(motor->gpio2, motor->pin2, 1);
	}
	g_motor_port->set_speed(motor->htim, motor->channel, abs_speed);
}

/**
 * @brief 获取电机速度
 * 
 * @param motor 
 * @return f32 电机速度(mm/s)
 */
f32 motor_get_speed(motor_t* motor)
{
    if (!motor->encoder) {
        debug_print("[motor] error: no encoder attached\n");
        return 0;
    }

    switch (motor->encoder->mode) {
        case ENCODER_MODE_QUADRATURE:
        case ENCODER_MODE_SINGLE_M:
        {
            return motor->cnt_to_mmps_factor * motor->encoder->value;
        }
        case ENCODER_MODE_SINGLE_T:
        {
			return motor->cnt_to_mmps_factor / motor->encoder->value;
        }
        default:
            return 0.0f;
    }
}

/**
 * @brief [TOOL]计算速度转换因子（计数值转换为毫米每秒）
 * 
 * @param motor 电机句柄
 * @return f32 速度转换因子
 */
static f32 cul_counts_to_mps_factor(const motor_t* motor)
{
	f32 counts_to_mps_factor;
	counts_to_mps_factor = ( 1000 * 3.1415926 * (f32)MOTOR_WHEEL_DIAM_MM ) / ( (f32)ENCODER_PULSES_PER_REV * (f32)ENCODER_DECODE_FACTOR * (f32)MOTOR_GEAR_REDUCTION * (f32)motor->encoder->sample_ms);
	return counts_to_mps_factor;
}
