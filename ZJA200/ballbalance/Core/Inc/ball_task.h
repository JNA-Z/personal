/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ball_task.h
  * @brief   小球定位任务状态机
  ******************************************************************************
  * 收到 AA 02 05 FF 后执行:
  *   目标 +5 → 稳定 → 保持 0.5s → 目标 -5 → 稳定 → PB0 置高
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __BALL_TASK_H
#define __BALL_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ======================== 任务时序参数 ======================== */

#define BALL_TASK_HOLD_P5_MS   500u   /**< 固定在 +P 后再保持的时间 (0.5s) */

/* ======================== 任务状态 ======================== */

typedef enum
{
  BALL_TASK_IDLE = 0,   /* 等待启动帧 */
  BALL_TASK_MOVE_P,     /* 目标 +P, 球运动中 */
  BALL_TASK_HOLD_P,     /* 已固定在 +P, 计时 0.5s */
  BALL_TASK_MOVE_N,     /* 目标 -P, 球运动中 */
  BALL_TASK_DONE,       /* 已固定在 -P, PB0 置高 */
} BallTask_State_t;

typedef struct
{
  BallTask_State_t state;        /* 当前状态 */
  float   first_target;          /* 第一个目标 +P (cm) */
  float   target_cm;             /* 当前目标 */
  uint32_t t_state_enter;        /* 进入当前状态时刻 */
  uint8_t  task_active;          /* 任务是否激活 */
} BallTask_t;

/* 全局任务句柄 */
extern BallTask_t g_task;

/* ======================== 函数声明 ======================== */

/** 初始化 (清状态, PB0 置低) */
void BallTask_Init(void);

/**
  * @brief 启动定位任务 (由 USART3 帧触发)
  * @param first_target_cm 第一个目标 +P (cm)
  * @note  任务序列: +P → 稳定 → 0.5s → -P → 稳定 → PB0 高
  */
void BallTask_Start(float first_target_cm);

/** 周期调用: 推进任务状态机 (需在 BallCtrl_Update 之后调用) */
void BallTask_Update(void);

#ifdef __cplusplus
}
#endif

#endif /* __BALL_TASK_H */
