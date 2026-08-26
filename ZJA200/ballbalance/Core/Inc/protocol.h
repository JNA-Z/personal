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
 * 数据包格式:
 *   (旧帧, 兼容) 5 字节:
 *     Byte0: 0xAA  帧头 (固定)
 *     Byte1: 0x04  长度字段 (固定)
 *     Byte2: sign  位置符号位, 0 = 正值/零, 1 = 负值
 *     Byte3: value X 坐标绝对值 = round(|x_cm| * 10), 上限 255
 *     Byte4: checksum = (Byte0 + Byte1 + Byte2 + Byte3) & 0xFF
 *
 *   (新帧, 位置+加速度) 7 字节:
 *     Byte0: 0xAA  帧头 (固定)
 *     Byte1: 0x08  长度字段 (固定)
 *     Byte2: sign  位置符号位
 *     Byte3: value 位置绝对值 = round(|x_cm| * 10), 上限 255
 *     Byte4: a_sign  加速度符号位
 *     Byte5: a_value 加速度绝对值 = round(|accel| / 10), 0~255 (±2550 cm/s²)
 *     Byte6: checksum = (Byte0..Byte5) & 0xFF
 *
 * 例: AA 04 00 20 CE -> x = +3.2 cm (旧帧)
 *     AA 08 00 20 01 0F 63 -> x = +3.2 cm, a = -150 cm/s² (新帧)
 *
 * 丢球约定: 视觉模块未检测到球时停止发包 (不发 x=0.0)。
 *           STM32 侧靠 ball_ctrl.c 的 200ms 无新帧超时判定视觉失效。
 */
#define PROTOCOL_FRAME_LEN     7u   /* 最大帧缓冲 (新帧 7 字节) */
#define PROTOCOL_FRAME_LEN_POS 5u   /* 旧帧长度 (仅位置) */
#define PROTOCOL_HEADER      0xAAu
#define PROTOCOL_LEN_FIELD   0x04u   /* 旧帧长度字段 */
#define PROTOCOL_LEN_FIELD_NEW 0x08u /* 新帧长度字段 (位置+加速度) */
#define PROTOCOL_VALUE_MAX   255u

typedef struct
{
  uint8_t sign;       /* 位置符号位: 0=正值/零, 1=负值 */
  uint8_t value;      /* 位置绝对值刻度: |x_cm| * 10, 0~255 */
  int16_t x_scaled;   /* 带符号位置刻度: sign==0 ? +value : -value, -255~+255 */
  float   x_cm;       /* 位置 (cm): x_scaled / 10.0f */
  uint8_t accel_sign;   /* 加速度符号位: 0=正/零, 1=负 */
  uint8_t accel_value;  /* 加速度绝对值刻度: |accel|/10, 0~255 */
  int16_t accel_scaled; /* 带符号加速度刻度 */
  float   accel_cm_s2;  /* 加速度 (cm/s²): accel_scaled * 10.0f */
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
