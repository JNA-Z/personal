/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ball_ctrl.h
  * @brief   小球位置控制模块 (视觉坐标 → 步进电机倾角)
  ******************************************************************************
  * 被控对象: 电机倾角 θ 决定小球运动速度 (θ>0 逆时针 → 球向正方向)
  * 控制律:   θ_cmd = Kp·e + Ki·∫e·dt − Kd·v_ball, e = 目标 − 当前位置
  * 补充:     Ki 消除稳态误差(带抗饱和); 蠕动机制克服静摩擦;
  *           Kd 抑制振荡
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __BALL_CTRL_H
#define __BALL_CTRL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ======================== 控制参数默认值 ======================== */

#define BALL_CTRL_KP_DEFAULT         10.0f//20.0f//1.2f   /**< 比例增益 (deg/cm, 实测调参) */
#define BALL_CTRL_KI_DEFAULT         9.0f//18.0f//1.0f    /**< 积分增益 (消除稳态误差) */
#define BALL_CTRL_KD_DEFAULT         6.0f//10.0f//0.01f   /**< 微分增益 (阻尼, 抑制振荡) */
#define BALL_CTRL_II_MAX_DEG         8.0f  /**< 积分项输出上限 (防积分饱和) */
#define BALL_CTRL_DEADBAND_CM        0.3f   /**< 位置死区 (cm) */
#define BALL_CTRL_MIN_ANGLE_DEG      0.5f   /**< 最小动作角, 防止零碎抖动 */
#define BALL_CTRL_MAX_ANGLE_DEG      25.0f  /**< 倾角限幅 (原 45°, 改小减小动作幅度) */
#define BALL_CTRL_PERIOD_MS          20u    /**< 控制周期 */
#define BALL_CTRL_VISION_TIMEOUT_MS  200u   /**< 视觉超时判定 (无新帧) */
#define BALL_CTRL_STABLE_CONFIRM_MS  200u   /**< "到位稳定"确认时间 */

/* 加速度前馈 (配合电机摆动, 视觉端回传小球加速度):
 *   theta_ff = ACCEL_FF * a_cm_s2
 * 球向 +x 加速 (a>0) → 提前左倾 (θ<0) 抑制 → 输出减去 theta_ff。
 * 若实测振荡加剧 → 减小或置 0; 若方向反 → 把下面的减号改加号。
 * 注意: 视觉加速度由位置二阶差分得到, 噪声大, 必须用 MAX_DEG 限幅,
 *       否则到位前前馈扰动会让球进不了死区 (调不到想要的位置)。
 */
#define BALL_CTRL_ACCEL_FF           0.005f  /**< 加速度前馈系数 (deg per cm/s²) */
#define BALL_CTRL_ACCEL_FF_MAX_DEG   2.0f    /**< 加速度前馈输出限幅 (±2°, 防噪声扰动) */

/* 静摩擦补偿 (蠕动机制):
 * 球已停住但未到位时, 强制施加最小蠕动角克服静摩擦.
 * 要求: 蠕动角必须 > 机构静摩擦阈值, 否则推不动球.
 * 若球仍停在偏差处 → 加大该值; 若到位后抖动 → 减小该值.
 */
#define BALL_CTRL_CREEP_ANGLE_DEG    4.0f   /**< 最小蠕动角 (需 > 机构静摩擦阈值, 否则球停在半路调不到位) */
#define BALL_CTRL_STUCK_V_CM_S       0.3f   /**< 判定"球已停住"的速度阈值 */

/* ======================== 电机运动参数 ======================== */

#define BALL_MOTOR_DIR_POS           STEPPER_DIR_CCW  /**< 逆时针 → 球向正 (极性可反) */
#define BALL_MOTOR_DIR_NEG           STEPPER_DIR_CW   /**< 顺时针 → 球向负 */
#define BALL_MOTOR_MIN_DELTA_DEG     0.2f             /**< 最小角度增量 */
#define BALL_MOTOR_RPM               150u             /**< 电机转速 (原 200, 降低动作更柔和) */
#define BALL_MOTOR_ACC               8u               /**< 电机加速度 (原 10) */

/* ======================== 结构体定义 ======================== */

typedef struct
{
  float    kp;                /* 比例增益 */
  float    ki;                /* 积分增益 */
  float    kd;                /* 微分增益 */
  float    accel_ff;          /* 加速度前馈系数 */
  float    deadband_cm;       /* 位置死区 */
  float    min_angle_deg;     /* 最小动作角 */
  float    max_angle_deg;     /* 倾角限幅 45° */
  uint32_t period_ms;         /* 控制周期 */
  uint32_t vision_timeout_ms; /* 视觉超时 */
} BallCtrlParam_t;

typedef struct
{
  float    x_ball_cm;         /* 当前球位 (视觉) */
  float    x_prev_cm;         /* 上一球位 */
  float    x_vel_cm_s;        /* 球速估算 (低通) */
  float    accel_cm_s2;       /* 视觉回传加速度 (cm/s², 前馈用) */
  float    x_target_cm;       /* 目标位置 */
  float    e_cm;              /* PID 误差 e = 目标 − 当前 (监视用) */
  float    e_integral;        /* 误差积分 (cm·s, 抗饱和) */
  float    theta_cmd_deg;     /* 输出倾角 */
  float    theta_p_deg;       /* PID 比例项输出 (监视用) */
  float    theta_i_deg;       /* PID 积分项输出 (监视用) */
  float    theta_d_deg;       /* PID 微分项输出 (监视用) */
  uint8_t  vision_ok;         /* 视觉是否有效 */
  uint32_t t_last_frame;      /* 最近有效帧时刻 */
  uint32_t t_last_ctrl;       /* 上次控制时刻 */
  uint8_t  at_target;         /* 是否处于死区内 */
  uint32_t at_target_since;   /* 进入死区时刻 */
} BallCtrlState_t;

typedef struct
{
  float    angle_deg;         /* 当前跟踪的电机倾角 (相对起始点) */
  float    limit_deg;         /* 累计极限 45° */
  uint8_t  homed;             /* 预留: 是否回零 */
  uint8_t  enabled;
} MotorState_t;

typedef struct
{
  BallCtrlParam_t  param;
  BallCtrlState_t  state;
  MotorState_t     motor;
} BallCtrl_t;



/* 全局控制句柄: 中断写入球位, 主循环读取计算 */
extern BallCtrl_t g_ball;

/* ======================== 函数声明 ======================== */

/** 初始化 (置默认参数, 清状态) */
void BallCtrl_Init(void);

/** 设置目标位置 (cm) */
void BallCtrl_SetTarget(float target_cm);

/** 读取当前目标 */
float BallCtrl_GetTarget(void);

/**
  * @brief 周期调用控制 (内部按 period_ms 限速)
  *        读球位 → 估算球速 → 算倾角 → 发电机指令
  */
void BallCtrl_Update(void);

/**
  * @brief 是否已稳定到位 (|球−目标| < 死区 且 持续 STABLE_CONFIRM_MS)
  * @retval 1: 已到位; 0: 未到位
  */
uint8_t BallCtrl_IsAtTarget(void);

#ifdef __cplusplus
}
#endif

#endif /* __BALL_CTRL_H */
