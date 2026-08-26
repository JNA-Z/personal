/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    protocol.h
  * @brief   USART1 中断接收协议模块 (协议见 串口通信协议.md)
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*
 * 数据包格式 (5 字节):
 *   Byte0: 0xAA  帧头 (固定)
 *   Byte1: 0x04  数据长度 (固定)
 *   Byte2: sign  符号位, 0 = 正值/零, 1 = 负值
 *   Byte3: value X 坐标绝对值 = round(|x_cm| * 10), 上限 255
 *   Byte4: checksum = (Byte0 + Byte1 + Byte2 + Byte3) & 0xFF
 *
 * 例: AA 04 00 20 CE -> x = +3.2 cm ; AA 04 01 0F BE -> x = -1.5 cm
 *
 * 丢球约定: 视觉模块未检测到球时停止发包 (不发 x=0.0)。
 *           STM32 侧靠 ball_ctrl.c 的 200ms 无新帧超时判定视觉失效。
 */
#define PROTOCOL_FRAME_LEN   5u
#define PROTOCOL_HEADER      0xAAu
#define PROTOCOL_LEN_FIELD   0x04u
#define PROTOCOL_VALUE_MAX   255u

typedef struct
{
  uint8_t sign;       /* 符号位: 0=正值/零, 1=负值 */
  uint8_t value;      /* 绝对值刻度: |x_cm| * 10, 0~255 */
  int16_t x_scaled;   /* 带符号刻度: sign==0 ? +value : -value, -255~+255 */
  float   x_cm;       /* 换算成厘米: x_scaled / 10.0f */
} Protocol_Frame;

/* 全局坐标数据: 每收到一帧校验通过后由中断实时更新, 主循环可直接读取 */
extern volatile Protocol_Frame ball_pos;

/* 调试全局变量: 正在组装的视觉帧 (AA 04 sign value checksum) */
extern volatile uint8_t g_uart1_rx_buf[PROTOCOL_FRAME_LEN];

/**
  * @brief 启动 USART1 中断接收 (需在 MX_USART1_UART_Init 之后调用一次)
  */
void Protocol_Init(void);

/**
  * @brief  主循环轮询取帧
  * @retval 1: 取到一帧新数据(写入 *frame); 0: 暂无新数据
  */
uint8_t Protocol_GetFrame(Protocol_Frame *frame);

#ifdef __cplusplus
}
#endif

#endif /* __PROTOCOL_H */
