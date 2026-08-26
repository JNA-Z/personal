/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    target_protocol.c
  * @brief   USART3 任务启动命令接收 + 4 字节帧解析 (启动命令协议.md)
  ******************************************************************************
  * 接收方式: HAL_UART_Receive_IT 单字节中断接收, 由 stm32f1xx_it.c 生成的
  *           USART3_IRQHandler → HAL_UART_IRQHandler → HAL_UART_RxCpltCallback
  *           在共享回调 (protocol.c 中) 里调用 Target_RxTick() 喂给状态机。
  *
  * 调试: 接收数据全部暴露为全局变量, 可在调试器实时监视。
  ******************************************************************************
  */
/* USER CODE END Header */

#include "main.h"
#include "target_protocol.h"
#include "ball_ctrl.h"
#include "ball_task.h"

extern UART_HandleTypeDef huart3;

/* 帧格式: [0xAA][Byte1][Byte2][0xFF] */
#define TARGET_HEADER       0xAAu
#define TARGET_TAIL         0xFFu
#define TARGET_TASK_BALLPOS 0x02u   /* 题1: Ball Pos 序列任务 */
#define TARGET_SIGN_POS     0x00u   /* 题4: 正 */
#define TARGET_SIGN_NEG     0x01u   /* 题4: 负 */
#define TARGET_PARAM_MAX    25u     /* 目标 cm 上限 (轨道量程 ~±25.5cm) */

/* 帧接收状态机 */
typedef enum
{
  TARGET_STATE_WAIT_HEADER,
  TARGET_STATE_BYTE1,
  TARGET_STATE_BYTE2,
  TARGET_STATE_TAIL,
} Target_RxState_t;

static Target_RxState_t rx_state = TARGET_STATE_WAIT_HEADER;

/* ---- 全局调试变量 (调试器实时监视) ---- */
volatile uint8_t g_uart3_rx_byte;      /* 当前收到的字节 */
volatile uint8_t g_uart3_rx_buf[4];    /* 正在组装的帧: AA B1 B2 FF */
volatile uint8_t g_uart3_rx_frame[4];  /* 最近一帧合法数据 */

/* 回显标志 (内部) */
static volatile uint8_t g_rx_echo_pending = 0;

/* ------------------------------------------------------------------ */
/*  帧分发: 按 Byte1 (任务码) 派发                                    */
/* ------------------------------------------------------------------ */
static void Target_Dispatch(uint8_t task_code, uint8_t param)
{
  /* 记录最近一帧合法数据 (调试用) */
  g_uart3_rx_frame[0] = TARGET_HEADER;
  g_uart3_rx_frame[1] = task_code;
  g_uart3_rx_frame[2] = param;
  g_uart3_rx_frame[3] = TARGET_TAIL;
  g_rx_echo_pending = 1;

  switch (task_code)
  {
  case TARGET_TASK_BALLPOS:
    /* 题1: 启动序列任务, 目标先 +param 再 -param */
    if (param > TARGET_PARAM_MAX) param = TARGET_PARAM_MAX;
    BallTask_Start((float)param);
    break;

  case TARGET_SIGN_POS:
  case TARGET_SIGN_NEG:
    /* 题4: 直接指定目标位置 */
    if (param > TARGET_PARAM_MAX) param = TARGET_PARAM_MAX;
    BallCtrl_SetTarget((task_code == TARGET_SIGN_POS) ? (float)param
                                                       : -(float)param);
    break;

  default:
    break; /* 未知任务码忽略 */
  }
}

/* ------------------------------------------------------------------ */
/*  逐字节解析                                                         */
/* ------------------------------------------------------------------ */
static void Target_ParseByte(uint8_t byte)
{
  switch (rx_state)
  {
  case TARGET_STATE_WAIT_HEADER:
    if (byte == TARGET_HEADER)
    {
      g_uart3_rx_buf[0] = byte;
      rx_state = TARGET_STATE_BYTE1;
    }
    break;

  case TARGET_STATE_BYTE1:
    g_uart3_rx_buf[1] = byte;
    rx_state = TARGET_STATE_BYTE2;
    break;

  case TARGET_STATE_BYTE2:
    g_uart3_rx_buf[2] = byte;
    rx_state = TARGET_STATE_TAIL;
    break;

  case TARGET_STATE_TAIL:
    rx_state = TARGET_STATE_WAIT_HEADER;
    if (byte == TARGET_TAIL)
    {
      Target_Dispatch(g_uart3_rx_buf[1], g_uart3_rx_buf[2]);
    }
    else
    {
      /* 尾帧不符, 重新同步: 该字节可能是新帧头 */
      if (byte == TARGET_HEADER)
      {
        g_uart3_rx_buf[0] = byte;
        rx_state = TARGET_STATE_BYTE1;
      }
    }
    break;
  }
}

/**
  * @brief 启动 USART3 中断接收 (USART3 中断已在 CubeMX 生成代码中使能)
  */
void Target_Protocol_Init(void)
{
  rx_state = TARGET_STATE_WAIT_HEADER;

  if (HAL_UART_Receive_IT(&huart3, (uint8_t *)&g_uart3_rx_byte, 1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief 由共享 HAL_UART_RxCpltCallback 调用: 解析已收字节并重新使能接收
  */
void Target_RxTick(void)
{
  Target_ParseByte(g_uart3_rx_byte);

  if (HAL_UART_Receive_IT(&huart3, (uint8_t *)&g_uart3_rx_byte, 1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief 主循环周期调用: 若收到新合法帧, 经 USART3 TX 原帧回显 (调试)
  * @note  串口监视器接 USART3 TX (PB10) 或上位机 RX, 能看到回显即链路通
  */
void Target_Poll(void)
{
  if (g_rx_echo_pending)
  {
    g_rx_echo_pending = 0;
    HAL_UART_Transmit(&huart3, (uint8_t *)g_uart3_rx_frame, 4, 100);
  }
}
