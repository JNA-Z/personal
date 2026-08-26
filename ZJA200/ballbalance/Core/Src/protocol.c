/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    protocol.c
  * @brief   USART1 中断接收 + 5 字节帧协议解析
  ******************************************************************************
  * 接收方式: 单字节 HAL_UART_Receive_IT, 每收一字节在 HAL_UART_RxCpltCallback
  *           中喂给状态机, 然后重新使能下一个字节的接收。
  *
  * 丢球约定: 视觉模块在未检测到球时会停止发包。
  *           本模块无需改动, 由 ball_ctrl.c 中的 200ms 无新帧超时判定
  *           视觉失效 (vision_ok=0) → 电机回零保持、积分清零。
  ******************************************************************************
  */
/* USER CODE END Header */

#include "main.h"
#include "protocol.h"
#include "target_protocol.h"

/* USART1 句柄 (定义于 main.c) */
extern UART_HandleTypeDef huart1;

/* 帧解析状态机 */
typedef enum
{
  PROTOCOL_STATE_IDLE,      /* 等待帧头 0xAA */
  PROTOCOL_STATE_HEADER,    /* 已收帧头, 等待长度 0x04 */
  PROTOCOL_STATE_SIGN,      /* 已收长度, 等待符号位 */
  PROTOCOL_STATE_VALUE,     /* 已收符号位, 等待数值 */
  PROTOCOL_STATE_CHECKSUM,  /* 已收数值, 等待校验和 */
} Protocol_State;

static volatile Protocol_State rx_state = PROTOCOL_STATE_IDLE;

/* ---- 全局调试变量 (调试器实时监视) ---- */
volatile uint8_t g_uart1_rx_buf[PROTOCOL_FRAME_LEN]; /* 正在组装的视觉帧 */
static uint8_t rx_byte;                       /* HAL_UART_Receive_IT 接收缓冲 */

volatile Protocol_Frame ball_pos;             /* 全局坐标数据 (中断中更新) */
static volatile uint8_t  rx_frame_pending = 0;

/* 校验和 = (Byte0 + Byte1 + Byte2 + Byte3) 取低 8 位 */
static uint8_t Protocol_CheckSum(void)
{
  return (uint8_t)(g_uart1_rx_buf[0] + g_uart1_rx_buf[1] + g_uart1_rx_buf[2] + g_uart1_rx_buf[3]);
}

/**
  * @brief 逐字节喂给协议状态机, 收满一帧并校验通过后写入全局 ball_pos
  * @param byte 从 UART 收到的一个字节
  */
static void Protocol_ParseByte(uint8_t byte)
{
  switch (rx_state)
  {
  case PROTOCOL_STATE_IDLE:
    if (byte == PROTOCOL_HEADER)
    {
      g_uart1_rx_buf[0] = byte;
      rx_state = PROTOCOL_STATE_HEADER;
    }
    break;

  case PROTOCOL_STATE_HEADER:
    if (byte == PROTOCOL_LEN_FIELD)
    {
      g_uart1_rx_buf[1] = byte;
      rx_state = PROTOCOL_STATE_SIGN;
    }
    else
    {
      /* 长度字段不符, 重新同步: 该字节本身可能就是下一帧帧头 */
      rx_state = PROTOCOL_STATE_IDLE;
      if (byte == PROTOCOL_HEADER)
      {
        g_uart1_rx_buf[0] = byte;
        rx_state = PROTOCOL_STATE_HEADER;
      }
    }
    break;

  case PROTOCOL_STATE_SIGN:
    g_uart1_rx_buf[2] = byte;
    rx_state = PROTOCOL_STATE_VALUE;
    break;

  case PROTOCOL_STATE_VALUE:
    g_uart1_rx_buf[3] = byte;
    rx_state = PROTOCOL_STATE_CHECKSUM;
    break;

  case PROTOCOL_STATE_CHECKSUM:
    g_uart1_rx_buf[4] = byte;
    if (Protocol_CheckSum() == byte)
    {
      /* 校验通过, 更新全局坐标数据 */
      ball_pos.sign  = g_uart1_rx_buf[2];
      ball_pos.value = g_uart1_rx_buf[3];
      ball_pos.x_scaled = (g_uart1_rx_buf[2] == 0u) ? (int16_t)g_uart1_rx_buf[3]
                                            : -(int16_t)g_uart1_rx_buf[3];
      ball_pos.x_cm = (float)ball_pos.x_scaled / 10.0f;
      rx_frame_pending = 1;
    }
    rx_state = PROTOCOL_STATE_IDLE; /* 下一字节即新帧帧头, 重新同步 */
    break;
  }
}

/**
  * @brief 启动接收, 应在 USART1 初始化完成后调用
  */
void Protocol_Init(void)
{
  rx_state        = PROTOCOL_STATE_IDLE;
  rx_frame_pending = 0;

  if (HAL_UART_Receive_IT(&huart1, &rx_byte, 1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief 主循环轮询取帧
  * @retval 1: 取到新帧(写入 *frame); 0: 无新帧
  */
uint8_t Protocol_GetFrame(Protocol_Frame *frame)
{
  if (rx_frame_pending != 0u)
  {
    *frame = ball_pos;
    rx_frame_pending = 0;
    return 1u;
  }
  return 0u;
}

/**
  * @brief HAL UART 接收完成弱回调, 在此覆盖
  *        仅处理 USART1; 后续如需给 USART2 加中断接收, 在此按实例分支即可
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    Protocol_ParseByte(rx_byte);
    /* 重新使能下一字节的中断接收 */
    if (HAL_UART_Receive_IT(huart, &rx_byte, 1) != HAL_OK)
    {
      Error_Handler();
    }
  }
  else if (huart->Instance == USART3)
  {
    /* 任务启动命令帧解析 (内部重新使能) */
    Target_RxTick();
  }
}
