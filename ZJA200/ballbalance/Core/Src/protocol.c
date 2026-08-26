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
  PROTOCOL_STATE_HEADER,    /* 已收帧头, 等待长度字段 (0x04 旧 / 0x08 新) */
  PROTOCOL_STATE_POS_SIGN,  /* 等待位置符号位 */
  PROTOCOL_STATE_POS_VALUE, /* 等待位置数值 */
  PROTOCOL_STATE_ACC_SIGN,  /* 等待加速度符号位 (仅新帧) */
  PROTOCOL_STATE_ACC_VALUE, /* 等待加速度数值 (仅新帧) */
  PROTOCOL_STATE_CHECKSUM,  /* 等待校验和 */
} Protocol_State;

static volatile Protocol_State rx_state = PROTOCOL_STATE_IDLE;
static volatile uint8_t rx_new_frame = 0;   /* 0=旧5字节帧, 1=新7字节帧 */

/* ---- 全局调试变量 (调试器实时监视) ---- */
volatile uint8_t g_uart1_rx_buf[PROTOCOL_FRAME_LEN]; /* 正在组装的视觉帧 */
static uint8_t rx_byte;                       /* HAL_UART_Receive_IT 接收缓冲 */

volatile Protocol_Frame ball_pos;             /* 全局坐标数据 (中断中更新) */
static volatile uint8_t  rx_frame_pending = 0;

/* 校验和 = 除校验字节外所有字节求和取低 8 位 (旧帧4字节, 新帧6字节) */
static uint8_t Protocol_CheckSum(void)
{
  uint8_t sum = 0u;
  uint8_t n   = rx_new_frame ? 6u : 4u;
  for (uint8_t i = 0u; i < n; i++)
  {
    sum = (uint8_t)(sum + g_uart1_rx_buf[i]);
  }
  return sum;
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
    if (byte == PROTOCOL_LEN_FIELD)          /* 旧帧 5 字节: 仅位置 */
    {
      rx_new_frame = 0;
      g_uart1_rx_buf[1] = byte;
      rx_state = PROTOCOL_STATE_POS_SIGN;
    }
    else if (byte == PROTOCOL_LEN_FIELD_NEW) /* 新帧 7 字节: 位置+加速度 */
    {
      rx_new_frame = 1;
      g_uart1_rx_buf[1] = byte;
      rx_state = PROTOCOL_STATE_POS_SIGN;
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

  case PROTOCOL_STATE_POS_SIGN:
    g_uart1_rx_buf[2] = byte;
    rx_state = PROTOCOL_STATE_POS_VALUE;
    break;

  case PROTOCOL_STATE_POS_VALUE:
    g_uart1_rx_buf[3] = byte;
    rx_state = rx_new_frame ? PROTOCOL_STATE_ACC_SIGN : PROTOCOL_STATE_CHECKSUM;
    break;

  case PROTOCOL_STATE_ACC_SIGN:
    g_uart1_rx_buf[4] = byte;
    rx_state = PROTOCOL_STATE_ACC_VALUE;
    break;

  case PROTOCOL_STATE_ACC_VALUE:
    g_uart1_rx_buf[5] = byte;
    rx_state = PROTOCOL_STATE_CHECKSUM;
    break;

  case PROTOCOL_STATE_CHECKSUM:
    if (rx_new_frame) { g_uart1_rx_buf[6] = byte; }
    else              { g_uart1_rx_buf[4] = byte; }

    if (Protocol_CheckSum() == byte)
    {
      /* 校验通过, 更新全局坐标数据 */
      ball_pos.sign      = g_uart1_rx_buf[2];
      ball_pos.value     = g_uart1_rx_buf[3];
      ball_pos.x_scaled  = (g_uart1_rx_buf[2] == 0u) ? (int16_t)g_uart1_rx_buf[3]
                                                     : -(int16_t)g_uart1_rx_buf[3];
      ball_pos.x_cm      = (float)ball_pos.x_scaled / 10.0f;

      if (rx_new_frame)
      {
        ball_pos.accel_sign   = g_uart1_rx_buf[4];
        ball_pos.accel_value  = g_uart1_rx_buf[5];
        ball_pos.accel_scaled = (g_uart1_rx_buf[4] == 0u) ? (int16_t)g_uart1_rx_buf[5]
                                                          : -(int16_t)g_uart1_rx_buf[5];
        ball_pos.accel_cm_s2  = (float)ball_pos.accel_scaled * 10.0f;
      }
      else
      {
        ball_pos.accel_sign   = 0u;
        ball_pos.accel_value  = 0u;
        ball_pos.accel_scaled = 0;
        ball_pos.accel_cm_s2  = 0.0f;
      }
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
  rx_new_frame    = 0;
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
