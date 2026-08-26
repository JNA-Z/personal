/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ball_task.c
  * @brief   小球定位任务状态机实现
  ******************************************************************************
  */
/* USER CODE END Header */

#include "ball_task.h"
#include "ball_ctrl.h"
#include "main.h"

/* 任务完成信号引脚 (PB0, main.h 中定义 flag_Pin, 已配置为 GPIO 输出) */
#define BALL_TASK_DONE_PIN     flag_Pin
#define BALL_TASK_DONE_PORT    flag_GPIO_Port

BallTask_t g_task;

/* ------------------------------------------------------------------ */
/*  初始化                                                             */
/* ------------------------------------------------------------------ */
void BallTask_Init(void)
{
  g_task.state         = BALL_TASK_IDLE;
  g_task.first_target  = 0.0f;
  g_task.target_cm     = 0.0f;
  g_task.t_state_enter = 0;
  g_task.task_active   = 0;

  HAL_GPIO_WritePin(BALL_TASK_DONE_PORT, BALL_TASK_DONE_PIN, GPIO_PIN_RESET);
}

/* ------------------------------------------------------------------ */
/*  启动任务 (USART3 帧触发)                                           */
/* ------------------------------------------------------------------ */
void BallTask_Start(float first_target_cm)
{
  /* 重新触发: 复位完成信号, 重跑序列 */
  HAL_GPIO_WritePin(BALL_TASK_DONE_PORT, BALL_TASK_DONE_PIN, GPIO_PIN_RESET);

  g_task.first_target  = first_target_cm;
  g_task.target_cm     = first_target_cm;
  g_task.t_state_enter = HAL_GetTick();
  g_task.task_active   = 1;

  BallCtrl_SetTarget(first_target_cm);
  g_task.state = BALL_TASK_MOVE_P;
}

/* ------------------------------------------------------------------ */
/*  状态机推进 (每循环调用)                                            */
/* ------------------------------------------------------------------ */
void BallTask_Update(void)
{
  if (!g_task.task_active)
    return;

  uint32_t now = HAL_GetTick();

  switch (g_task.state)
  {
  case BALL_TASK_IDLE:
    break;

  case BALL_TASK_MOVE_P:
    /* 球固定在 +P 后, 进入保持计时 */
    if (BallCtrl_IsAtTarget())
    {
      g_task.state         = BALL_TASK_HOLD_P;
      g_task.t_state_enter = now;
    }
    break;

  case BALL_TASK_HOLD_P:
    /* 在 +P 保持 0.5s 后, 目标改为 -P */
    if (now - g_task.t_state_enter >= BALL_TASK_HOLD_P5_MS)
    {
      g_task.target_cm = -g_task.first_target;
      BallCtrl_SetTarget(g_task.target_cm);
      g_task.state         = BALL_TASK_MOVE_N;
      g_task.t_state_enter = now;
    }
    break;

  case BALL_TASK_MOVE_N:
    /* 球稳定在 -P 后, 完成: PB0 置高 */
    if (BallCtrl_IsAtTarget())
    {
      g_task.state         = BALL_TASK_DONE;
      g_task.t_state_enter = now;
      HAL_GPIO_WritePin(BALL_TASK_DONE_PORT, BALL_TASK_DONE_PIN, GPIO_PIN_SET);
    }
    break;

  case BALL_TASK_DONE:
    /* 保持完成状态, 等待下一次启动帧 */
    break;
  }
}
