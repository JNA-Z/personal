#include "motor_port.h"
#include "../driver/motor.h"
#include "main.h"



static motor_port_t g_motor_port = {0};

static void motor_write_pin(void* gpio, u16 pin, u8 value)
{
    HAL_GPIO_WritePin((GPIO_TypeDef*)gpio, pin, value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void motor_set_ccr(void* htim, u32 channel, u32 ccr){
	__HAL_TIM_SET_COMPARE((TIM_HandleTypeDef*) htim, channel, ccr);
}

static void motor_start(void* htim, u32 channel){
	HAL_TIM_PWM_Start((TIM_HandleTypeDef*) htim, channel);
}

static void motor_stop(void* htim, u32 channel){
	HAL_TIM_PWM_Stop((TIM_HandleTypeDef*) htim, channel);
}

void motor_port_init(void){
	g_motor_port.write_pin = motor_write_pin;
	g_motor_port.set_speed = motor_set_ccr;
	g_motor_port.start = motor_start;
	g_motor_port.stop = motor_stop;
	motor_bind_port(&g_motor_port);
}
