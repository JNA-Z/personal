#include "main.h"
#include "tim.h"
#include "usart.h"
#include "pack.h"
#include "lcd.h"
#include "model.h"
#include "door_manager.h"

typedef enum
{
    WAIT_HEAD1,
    WAIT_HEAD2,
    WAIT_DATA,
    DATA_OK
} UART_RxFlag;

extern struct Sflag     flag;
extern struct Slocation loc;

static UART_RxFlag g_gyroscope_rx_flag = WAIT_HEAD1;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{
    if (huart == &huart1) {
        switch (g_gyroscope_rx_flag) {
        case WAIT_HEAD1:
        {
            if (GyrRxHeadBuffer[0] == 0x55) {
                g_gyroscope_rx_flag = WAIT_HEAD2;
                HAL_UART_Receive_IT(&huart1, GyrRxHeadBuffer, 1);
            }
            else {
                HAL_UART_Receive_IT(&huart1, GyrRxHeadBuffer, 1);
            }
            break;
        }
        case WAIT_HEAD2:
        {
            if (GyrRxHeadBuffer[0] == 0x53) {
                g_gyroscope_rx_flag = WAIT_DATA;
                HAL_UART_Receive_IT(&huart1, GyrRxDataBuffer, 9);
            }
            else {
                g_gyroscope_rx_flag = WAIT_HEAD1;
                HAL_UART_Receive_IT(&huart1, GyrRxHeadBuffer, 1);
            }
            break;
        }
        case WAIT_DATA:
        {
            g_gyroscope_rx_flag = WAIT_HEAD1;
            HAL_UART_Receive_IT(&huart1, GyrRxHeadBuffer, 1);
            break;
        }
        default:
        {
            HAL_UART_Receive_IT(&huart1, GyrRxHeadBuffer, 1);
            break;
        }
        }
    }

    if (huart == &huart2) {
        if (CameraRxBuffer[0] == '1') {
            door_dir_t dir = (flag.Direction == 0) ? DOOR_DIR_GO : DOOR_DIR_BACK;
            door_set_state((door_id_t)loc.door, dir, DOOR_STATE_CLOSED);
        }
        else {
            char str[63] = {0};
            sprintf(str, "Camera init success");
            LCD_ShowString(8, 32, (u8*)str, RED, WHITE, 12, 0);
        }

        HAL_UART_Receive_IT(&huart2, CameraRxBuffer, 1);
        HAL_UART_Receive_IT(&huart1, GyrRxHeadBuffer, 1);
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
    if (htim->Instance == TIM1) {
        HAL_IncTick();
    }

    if (htim == &htim3) {
        bsp_encoder_overflow_handler(&encoder_left);
    }

    if (htim == &htim4) {
        bsp_encoder_overflow_handler(&encoder_right);
    }
}
