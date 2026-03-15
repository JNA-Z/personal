/*
 *陀螺仪驱动函数
 *由串口USART1接收，接收中断回调在主函数
 *GTM+8 2025-05-10 01:30:48 GTM+8
 */
#include "WT901C.h"
#include <stdio.h>
#include "usart.h"
#include "dma.h"
#include "delay.h"
#include <string.h>
#include "main.h"

uint16_t gyro_start_state = 0;   // 0:还未开始接收；1：开始接收
uint8_t  GyrRxHeadBuffer[10];    // 陀螺仪串口帧头接收缓存
uint8_t  GyrRxDataBuffer[50];    // 陀螺仪串口数据接收缓存

/**********陀螺仪初始化配置包**********/

uint8_t Gyr_Init_Config[20] = {
    0xFF, 0xAA, 0x69, 0x88, 0xB5,   // 解锁包头
    0xFF, 0xAA, 0x01, 0x08, 0x00,   // 设置角度参考（x,y轴）
    0xFF, 0xAA, 0x01, 0x04, 0x00,   // 设置Z轴参考（六轴模式下）
    0xFF, 0xAA, 0x00, 0x00, 0x00    // 保存包尾
};
#if Debug   // 调试包
uint8_t Gyr_Debug_Packet[30] = {
    0xFF, 0xAA, 0x69, 0x88, 0xB5,   // 解锁包头
    0xFF, 0xAA, 0x01, 0x01, 0x00,   // 加速度校准
    0xFF, 0xAA, 0x01, 0x00, 0x00,   // 结束校准
    0xFF, 0xAA, 0x03, 0x09, 0x00,   // 设置输出速率（100HZ）
    0xFF, 0xAA, 0x02, 0x3E, 0x00,   // 设置输出内容
    0xFF, 0xAA, 0x00, 0x00, 0x00    // 保存包尾
};
#endif

/**********陀螺仪初始化**********/
void Gyroscope_Init(void)
{
#if Debug
    HAL_UART_Transmit(&huart1, Gyr_Debug_Packet, 5, 100);
    HAL_Delay(100);
    HAL_UART_Transmit(&huart1, Gyr_Debug_Packet + 5, 5, 100);
    HAL_Delay(2000);
    HAL_UART_Transmit(&huart1, Gyr_Debug_Packet + 10, 5, 100);
    HAL_Delay(100);
    HAL_UART_Transmit(&huart1, Gyr_Debug_Packet + 15, 5, 100);
    HAL_Delay(100);
    HAL_UART_Transmit(&huart1, Gyr_Debug_Packet + 20, 5, 100);
    HAL_Delay(100);
    HAL_UART_Transmit(&huart1, Gyr_Debug_Packet + 25, 5, 100);
#endif
    // 发送校准命令
    HAL_UART_Transmit(&huart1, Gyr_Init_Config, 5, 100);
    HAL_Delay(100);
    HAL_UART_Transmit(&huart1, Gyr_Init_Config + 5, 5, 100);
    HAL_Delay(500);
    HAL_UART_Transmit(&huart1, Gyr_Init_Config + 10, 5, 100);
    HAL_Delay(500);
    HAL_UART_Transmit(&huart1, Gyr_Init_Config + 15, 5, 100);
}

/**********陀螺仪启动**********/
void Gyroscope_Start(void)
{
    if (gyro_start_state == 0 && GyrRxHeadBuffer[0] == 0) {
        HAL_UART_Receive_IT(&huart1, GyrRxHeadBuffer, 1);
    }
    else {
        gyro_start_state = 1;
    }
}
