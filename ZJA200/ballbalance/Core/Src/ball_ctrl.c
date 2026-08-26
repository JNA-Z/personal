/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ball_ctrl.c
  * @brief   小球位置控制: 视觉球位 + 目标 → 步进电机倾角
  ******************************************************************************
  */
/* USER CODE END Header */

#include "ball_ctrl.h"
#include "protocol.h"
#include "stepper_motor.h"
#include <math.h>

BallCtrl_t g_ball;

 
/* ------------------------------------------------------------------ */
/*  电机指令: 把期望倾角换算成 FD 相对定位脉冲                         */
/*  采用"相对定位 + 内部跟踪角"方式: 跟踪角从起始位置起算,            */
/*  与"顺逆累计转动 ±45°"约束一致, 且无需回零/限位开关。              */
/* ------------------------------------------------------------------ */
static void Motor_SetAngle(float angle_deg)
{
  /* 累计角限幅 (机械硬约束, 最大最大不能超 45°) */
  if (angle_deg >  g_ball.motor.limit_deg) angle_deg =  g_ball.motor.limit_deg;
  if (angle_deg < -g_ball.motor.limit_deg) angle_deg = -g_ball.motor.limit_deg;

  float delta = angle_deg - g_ball.motor.angle_deg;
  if (fabsf(delta) < BALL_MOTOR_MIN_DELTA_DEG)
  {
    g_ball.motor.angle_deg = angle_deg;   /* 同步跟踪值 */
    return;                               /* 角度变化过小, 不命令 */
  }

  /* 角度差 → 脉冲数 (16细分 3200 脉冲/圈) */
  int32_t pulses = (int32_t)(fabsf(delta) * STEPPER_PULSES_PER_REV / 360.0f);
  if (pulses < 1) pulses = 1;

  uint8_t dir = (delta > 0.0f) ? BALL_MOTOR_DIR_POS : BALL_MOTOR_DIR_NEG;

  Stepper_RunFD(STEPPER_0, dir, BALL_MOTOR_RPM, BALL_MOTOR_ACC,
                (uint32_t)pulses, STEPPER_RA_RELATIVE);

  g_ball.motor.angle_deg = angle_deg;   /* 更新跟踪角 */
}

/* ------------------------------------------------------------------ */
/*  初始化                                                             */
/* ------------------------------------------------------------------ */
void BallCtrl_Init(void)
{
  g_ball.param.kp              = BALL_CTRL_KP_DEFAULT;
  g_ball.param.ki              = BALL_CTRL_KI_DEFAULT;
  g_ball.param.kd              = BALL_CTRL_KD_DEFAULT;
  g_ball.param.deadband_cm     = BALL_CTRL_DEADBAND_CM;
  g_ball.param.min_angle_deg   = BALL_CTRL_MIN_ANGLE_DEG;
  g_ball.param.max_angle_deg   = BALL_CTRL_MAX_ANGLE_DEG;
  g_ball.param.period_ms       = BALL_CTRL_PERIOD_MS;
  g_ball.param.vision_timeout_ms = BALL_CTRL_VISION_TIMEOUT_MS;

  g_ball.state.x_ball_cm    = 0.0f;
  g_ball.state.x_prev_cm    = 0.0f;
  g_ball.state.x_vel_cm_s   = 0.0f;
  g_ball.state.x_target_cm  = 0.0f;   /* 默认目标 0: 开机先稳球在中心 */
  g_ball.state.e_cm         = 0.0f;
  g_ball.state.e_integral   = 0.0f;
  g_ball.state.theta_cmd_deg = 0.0f;
  g_ball.state.theta_p_deg  = 0.0f;
  g_ball.state.theta_i_deg  = 0.0f;
  g_ball.state.theta_d_deg  = 0.0f;
  g_ball.state.vision_ok    = 0;
  g_ball.state.t_last_frame = 0;
  g_ball.state.t_last_ctrl  = 0;
  g_ball.state.at_target    = 0;
  g_ball.state.at_target_since = 0;

  g_ball.motor.angle_deg    = 0.0f;   /* 起始位置即跟踪零点 */
  g_ball.motor.limit_deg    = BALL_CTRL_MAX_ANGLE_DEG;
  g_ball.motor.homed        = 0;
  g_ball.motor.enabled      = 1;
}

/* ------------------------------------------------------------------ */
/*  目标设置                                                           */
/* ------------------------------------------------------------------ */
void BallCtrl_SetTarget(float target_cm)
{
  g_ball.state.x_target_cm = target_cm;
  g_ball.state.e_integral = 0.0f;   /* 目标切换: 清积分, 防残留 */
}

float BallCtrl_GetTarget(void)
{
  return g_ball.state.x_target_cm;
}

/* ------------------------------------------------------------------ */
/*  周期控制                                                           */
/* ------------------------------------------------------------------ */
void BallCtrl_Update(void)
{
  uint32_t now = HAL_GetTick();

  /* 按周期限速 */
  if (now - g_ball.state.t_last_ctrl < g_ball.param.period_ms)
    return;
  g_ball.state.t_last_ctrl = now;

  /* 读取最新球位 (视觉中断实时更新) */
  Protocol_Frame frame;
  if (Protocol_GetFrame(&frame))
  {
    g_ball.state.x_prev_cm   = g_ball.state.x_ball_cm;
    g_ball.state.x_ball_cm   = frame.x_cm;
    g_ball.state.t_last_frame = now;
  }

  /* 视觉有效性 (超时保护, 防止丢帧失控) */
  g_ball.state.vision_ok =
    (now - g_ball.state.t_last_frame < g_ball.param.vision_timeout_ms) ? 1u : 0u;

  /* 球速估算 (一阶低通) */
  float dt = (float)g_ball.param.period_ms / 1000.0f;
  float v_raw = (g_ball.state.x_ball_cm - g_ball.state.x_prev_cm) / dt;
  g_ball.state.x_vel_cm_s = 0.7f * g_ball.state.x_vel_cm_s + 0.3f * v_raw;

  /* 误差 */
  float e = g_ball.state.x_target_cm - g_ball.state.x_ball_cm;
  g_ball.state.e_cm = e;   /* 全局保存, 调试器实时监视 */
  pide  = e;

  /* ==================== PID 计算 ==================== */

  float theta_p = g_ball.param.kp * e;                          /* 比例项 */
  float theta_d = g_ball.param.kd * g_ball.state.x_vel_cm_s;    /* 微分项 */
  float theta_i_prev = g_ball.param.ki * g_ball.state.e_integral;

  /* 积分: 条件积分抗饱和 — 输出未饱和才累积, 防止到位前积分风车过冲 */
  if (g_ball.state.vision_ok && (g_ball.param.ki > 0.0f))
  {
    float theta_raw = theta_p + theta_i_prev - theta_d;
    if (fabsf(theta_raw) < g_ball.param.max_angle_deg)
    {
      g_ball.state.e_integral += e * dt;   /* dt = period_ms/1000 = 0.02s */
      float i_max = BALL_CTRL_II_MAX_DEG / g_ball.param.ki; /* |Ki·∫e| ≤ II_MAX */
      if (g_ball.state.e_integral >  i_max) g_ball.state.e_integral =  i_max;
      if (g_ball.state.e_integral < -i_max) g_ball.state.e_integral = -i_max;
    }
    /* 输出饱和: 停止积分 (防风车) */
  }
  else
  {
    g_ball.state.e_integral = 0.0f;   /* 视觉丢失或 Ki=0: 清积分 */
  }

  float theta_i = g_ball.param.ki * g_ball.state.e_integral;  /* 积分项 */
  float theta   = theta_p + theta_i - theta_d;

  /* 全局保存 PID 三项, 调试器实时监视 */
  g_ball.state.theta_p_deg = theta_p;
  g_ball.state.theta_i_deg = theta_i;
  g_ball.state.theta_d_deg = theta_d;

  /* ==================== 输出整形 ==================== */

  if (!g_ball.state.vision_ok)
  {
    /* 视觉丢失: 电机回零保持;
     * 同时清零速度估算, 防止视觉恢复瞬间微分项跳变 */
    g_ball.state.x_vel_cm_s = 0.0f;
    theta = 0.0f;
  }
  else if (fabsf(e) < g_ball.param.deadband_cm)
  {
    /* 死区内: 去掉 P 项(防微抖), 保留积分把球顶到精确目标 + 微分阻尼 */
    theta = theta_i - theta_d;
  }
  else if (fabsf(g_ball.state.x_vel_cm_s) < BALL_CTRL_STUCK_V_CM_S)
  {
    /* 球停住未到位: 保证最小蠕动角 (PID 输出不足时才补) */
    if ((e > 0.0f) && (theta <  BALL_CTRL_CREEP_ANGLE_DEG))
      theta =  BALL_CTRL_CREEP_ANGLE_DEG;
    if ((e < 0.0f) && (theta > -BALL_CTRL_CREEP_ANGLE_DEG))
      theta = -BALL_CTRL_CREEP_ANGLE_DEG;
  }

  /* 最小动作角 */
  if (fabsf(theta) < g_ball.param.min_angle_deg) theta = 0.0f;

  /* 倾角限幅 ±45° */
  if (theta >  g_ball.param.max_angle_deg) theta =  g_ball.param.max_angle_deg;
  if (theta < -g_ball.param.max_angle_deg) theta = -g_ball.param.max_angle_deg;

  g_ball.state.theta_cmd_deg = theta;
  Motor_SetAngle(theta);

  /* 到位检测: |误差| < 死区 */
  if (fabsf(e) <= g_ball.param.deadband_cm)
  {
    if (!g_ball.state.at_target)
    {
      g_ball.state.at_target = 1;
      g_ball.state.at_target_since = now;
    }
  }
  else
  {
    g_ball.state.at_target = 0;
  }
}

/* ------------------------------------------------------------------ */
/*  是否稳定到位                                                       */
/* ------------------------------------------------------------------ */
uint8_t BallCtrl_IsAtTarget(void)
{
  if (g_ball.state.at_target)
  {
    if (HAL_GetTick() - g_ball.state.at_target_since >= BALL_CTRL_STABLE_CONFIRM_MS)
      return 1u;
  }
  return 0u;
}
