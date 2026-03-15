#include "bsp.h"

#include "main.h"
#include "usart.h"
#include "camera.h"
#include "pack.h"
#include "delay.h"

/**
 * @brief 读取启动按键状态
 */
uint8_t bsp_start_key_is_pressed(void)
{
    return HAL_GPIO_ReadPin(KEY_START_GPIO_Port, KEY_START_Pin) == GPIO_PIN_RESET;
}

/**
 * @brief 读取挡板遮挡状态
 */
uint8_t bsp_barrier_is_blocked(void)
{
    return HAL_GPIO_ReadPin(Infrared_up_GPIO_Port, Infrared_up_Pin) == Have;
}

/**
 * @brief 读取挡板移开状态
 */
uint8_t bsp_barrier_is_cleared(void)
{
    return HAL_GPIO_ReadPin(Infrared_up_GPIO_Port, Infrared_up_Pin) == No;
}

/**
 * @brief 启动摄像头串口单字节中断接收
 */
void bsp_camera_start_rx(void)
{
    HAL_UART_Receive_IT(&huart2, CameraRxBuffer, 1);
}

/**
 * @brief 采样设备传感器状态
 */
void bsp_sample_device_state(struct Sflag_Device* state)
{
    if (state == NULL) {
        return;
    }

    state->Infrared_up = (HAL_GPIO_ReadPin(Infrared_up_GPIO_Port, Infrared_up_Pin) == Have) ? Online : Offline;
    delay_tick_noblock(1);

    state->Infrared_Hit = (HAL_GPIO_ReadPin(Infrared_Hit_GPIO_Port, Infrared_Hit_Pin) == Have) ? Online : Offline;
    delay_tick_noblock(1);

    state->Infrared_down_left = (HAL_GPIO_ReadPin(Infrared_down_left_GPIO_Port, Infrared_down_left_Pin) == Have) ? Online : Offline;
    delay_tick_noblock(1);

    state->Infrared_down_right = (HAL_GPIO_ReadPin(Infrared_down_right_GPIO_Port, Infrared_down_right_Pin) == Have) ? Online : Offline;
    delay_tick_noblock(1);

    state->Color_code_left = (HAL_GPIO_ReadPin(colorcode_left_GPIO_Port, colorcode_left_Pin) == Have) ? Online : Offline;
    delay_tick_noblock(1);

    state->Color_code_right = (HAL_GPIO_ReadPin(colorcode_right_GPIO_Port, colorcode_right_Pin) == Have) ? Online : Offline;
    delay_tick_noblock(1);

    state->Color_code_front_left = (HAL_GPIO_ReadPin(colorcode_front_left_GPIO_Port, colorcode_front_left_Pin) == Have) ? Online : Offline;
    delay_tick_noblock(1);

    state->Color_code_front_right = (HAL_GPIO_ReadPin(colorcode_front_right_GPIO_Port, colorcode_front_right_Pin) == Have) ? Online : Offline;
    delay_tick_noblock(1);

    delay_tick_noblock(1);
}
