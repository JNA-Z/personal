/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    target_protocol.h
  * @brief   USART3 任务启动命令接收模块 (协议见 启动命令协议.md)
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __TARGET_PROTOCOL_H
#define __TARGET_PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*
 * 命令帧格式 (4 字节):
 *   [0xAA] [Byte1] [Byte2] [0xFF]
 *     Byte1: 任务码/符号位
 *     Byte2: 参数 (位置/cm 绝对值)
 *
 *   题1 Ball Pos : AA 02 05 FF  -> 启动序列任务 (+05 → 稳定0.5s → -05 → PB0高)
 *   题4 Ball Dist: AA 00 XX FF  -> 目标 +XX cm
 *                  AA 01 XX FF  -> 目标 -XX cm
 */

/**
  * @brief 启动 USART3 中断接收 (需在 MX_USART3_UART_Init 之后调用一次)
  * @note  USART3 中断已在 CubeMX 生成代码中使能, 这里启动 HAL 单字节接收
  */
void Target_Protocol_Init(void);

/* ---- 调试全局变量: USART3 接收实时数据 ---- */
extern volatile uint8_t g_uart3_rx_byte;      /* 当前收到的字节 */
extern volatile uint8_t g_uart3_rx_buf[4];    /* 正在组装的帧: AA B1 B2 FF */
extern volatile uint8_t g_uart3_rx_frame[4];  /* 最近一帧合法数据 */

/**
  * @brief 由共享 HAL_UART_RxCpltCallback 调用: 解析已收字节并重新使能
  */
void Target_RxTick(void);

/**
  * @brief 主循环周期调用 (调试): 若收到新合法帧, 经 USART3 TX 原帧回显
  * @note  用于确认 USART3 目标命令是否真正被收到/解析
  */
void Target_Poll(void);

#ifdef __cplusplus
}
#endif

#endif /* __TARGET_PROTOCOL_H */
