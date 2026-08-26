/**
 * @file stepper_motor.h
 * @brief X42S 闭环步进电机驱动头文件 - UART 通信控制 (Emm_V5.0 全协议)
 * @version 4.0
 * @date 2026-07-30
 *
 * @details
 * ZDT_X42S 步进电机 Emm_V5.0 协议完整驱动。
 * 支持: 速度模式、位置模式、回零、参数读写、同步运动等全功能。
 *
 * 通信参数: 115200bps, 8N1
 * 帧格式: [Addr] [Cmd...] [Data...] [0x6B]
 * 多字节值: 大端序 (Big-Endian)
 * 方向: 0=CW, 非0=CCW
 *
 * 平台: STM32F103 (HAL库), 本工程挂载在 USART2
 */

#ifndef __STEPPER_MOTOR_H
#define __STEPPER_MOTOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "main.h"

/*============================================================================*/
/*                          宏定义区                                          */
/*============================================================================*/

#define STEPPER_UART_HANDLE             huart2  /**< UART 句柄 (main.c 中定义) */
extern UART_HandleTypeDef huart2;

/* ======================== 基础常量 ======================== */

#define STEPPER_TAIL                    0x6B    /**< 帧尾固定字节 */
#define STEPPER_MOTOR_ID_DEFAULT        0x01    /**< 默认电机地址 */

/* ======================== 方向常量 ======================== */

#define STEPPER_DIR_CW                  0x00    /**< 正转 (CW) */
#define STEPPER_DIR_CCW                 0x01    /**< 反转 (CCW) */

/* ======================== 帧长度常量 ======================== */

#define STEPPER_FRAME_3B                3       /**< 最短帧: Addr Cmd 6B */
#define STEPPER_FRAME_4B                4       /**< 短帧: Addr Cmd1 Cmd2 6B */
#define STEPPER_FRAME_5B                5       /**< 停止帧 */
#define STEPPER_FRAME_6B                6       /**< 使能/回零帧 */
#define STEPPER_FRAME_F6_LEN            8       /**< F6 速度模式帧 */
#define STEPPER_FRAME_FD_LEN            13      /**< FD 位置模式帧 */
#define STEPPER_FRAME_RX_LEN            8       /**< 接收缓冲区长度 */

/* ======================== 运动模式 (raF) ======================== */

#define STEPPER_RA_ABSOLUTE             0       /**< 绝对定位 */
#define STEPPER_RA_RELATIVE             1       /**< 相对定位 */
#define STEPPER_RA_REALTIME             2       /**< 实时相对偏移 */

/* ======================== 回零模式 ======================== */

#define STEPPER_HOME_NEAR               0       /**< 近模式找零点 */
#define STEPPER_HOME_LIMIT_Z            1       /**< 限位+Z信号回零 */
#define STEPPER_HOME_COLLISION          2       /**< 碰撞找零点 */
#define STEPPER_HOME_Z_LIMIT            3       /**< Z信号+限位回零 */

/* ======================== 参数码 (System Params) ======================== */

#define STEPPER_PARAM_VBUS      0x24    /**< 总线电压 */
#define STEPPER_PARAM_CBUS      0x26    /**< 总线电流 */
#define STEPPER_PARAM_CPHA      0x27    /**< 相电流 */
#define STEPPER_PARAM_ENCO      0x29    /**< 编码器值 */
#define STEPPER_PARAM_CLKC      0x30    /**< 指令脉冲计数 */
#define STEPPER_PARAM_ENCL      0x31    /**< 编码器圈数 */
#define STEPPER_PARAM_CLKI      0x32    /**< 指令脉冲频率 */
#define STEPPER_PARAM_TPOS      0x33    /**< 理论位置 */
#define STEPPER_PARAM_SPOS      0x34    /**< 实时位置 */
#define STEPPER_PARAM_VEL       0x35    /**< 实时速度 */
#define STEPPER_PARAM_CPOS      0x36    /**< 当前位置(闭环) */
#define STEPPER_PARAM_PERR      0x37    /**< 位置误差 */
#define STEPPER_PARAM_VBAT      0x38    /**< 电池电压 */
#define STEPPER_PARAM_TEMP      0x39    /**< 温度 */
#define STEPPER_PARAM_FLAG      0x3A    /**< 状态标志 */
#define STEPPER_PARAM_OFLAG     0x3B    /**< 输出标志 */
#define STEPPER_PARAM_OAF       0x3C    /**< 过载系数 */
#define STEPPER_PARAM_PIN       0x3D    /**< IO引脚状态 */

/* ======================== 默认运行参数 ======================== */

#define STEPPER_DEFAULT_RPM             100     /**< 默认速度 (RPM) */
#define STEPPER_DEFAULT_ACC             5       /**< 默认加速度 */

/* ======================== 机械参数 (实测) ======================== */

#define STEPPER_PULSES_PER_REV          3200    /**< 每圈脉冲 (16细分) */
#define STEPPER_LIMIT_CCW_DEG           45.0f   /**< 累计逆时针极限 (端子向下) 45° */
#define STEPPER_LIMIT_CW_DEG            45.0f   /**< 累计顺时针极限 (端子向上) 45° */

/** 角度→脉冲换算 */
#define STEPPER_DEG_TO_PULSE(deg) \
    ((uint32_t)((float)(deg) * STEPPER_PULSES_PER_REV / 360.0f))

/* ======================== 调试测试步骤 ======================== */

#define STEPPER_TEST_INIT               0
#define STEPPER_TEST_RUN_F6             1
#define STEPPER_TEST_STOP1              2
#define STEPPER_TEST_RUN_FD             3
#define STEPPER_TEST_STOP2              4
#define STEPPER_TEST_DONE               5

/*============================================================================*/
/*                          类型定义区                                        */
/*============================================================================*/

typedef enum {
    STEPPER_0 = 0,
    STEPPER_NUM
} Stepper_ID_t;

typedef enum {
    STEPPER_STATE_IDLE = 0,
    STEPPER_STATE_RUNNING,
    STEPPER_STATE_ERROR
} Stepper_State_t;

typedef struct {
    UART_HandleTypeDef* huart;
    uint8_t motor_id;
    uint16_t default_rpm;
    uint8_t default_acc;
} Stepper_Config_t;

typedef struct {
    Stepper_State_t state;
    bool is_fault;
    uint8_t rx_buf[STEPPER_FRAME_RX_LEN];
    uint8_t rx_len;
    bool rx_ready;
    uint8_t test_step;
    bool test_running;
    uint16_t test_error;
} Stepper_Data_t;

typedef struct {
    Stepper_Config_t config;
    Stepper_Data_t data;
} Stepper_Handle_t;

/*============================================================================*/
/*                          函数声明                                          */
/*============================================================================*/

/* ---------- 初始化和基本控制 ---------- */

bool Stepper_Init(Stepper_ID_t id);
void Stepper_Deinit(Stepper_ID_t id);
void Stepper_SetSpeed(Stepper_ID_t id, uint16_t rpm, uint8_t acc);

/* ---------- 运动控制 ---------- */

void Stepper_RunForward(Stepper_ID_t id);
void Stepper_RunBackward(Stepper_ID_t id);
void Stepper_Stop(Stepper_ID_t id);

/**
 * @brief F6 速度模式 — 连续旋转
 * @param id   电机ID
 * @param dir  方向: STEPPER_DIR_CW / STEPPER_DIR_CCW
 * @param rpm  速度 0~5000 (RPM)
 * @param acc  加速度 0~255 (0=直接启动)
 */
void Stepper_RunF6(Stepper_ID_t id, uint8_t dir, uint16_t rpm, uint8_t acc);

/**
 * @brief FD 位置模式 — 定位控制
 * @param id     电机ID
 * @param dir    方向
 * @param rpm    速度 0~5000 (RPM)
 * @param acc    加速度 0~255
 * @param pulses 脉冲数 (16细分下 3200=1圈)
 * @param raF    模式: STEPPER_RA_ABSOLUTE/RELATIVE/REALTIME
 */
void Stepper_RunFD(Stepper_ID_t id, uint8_t dir, uint16_t rpm, uint8_t acc,
                   uint32_t pulses, uint8_t raF);

/* ---------- 使能和状态 ---------- */

void Stepper_Enable(Stepper_ID_t id);
void Stepper_Disable(Stepper_ID_t id);

/* ---------- 回零 ---------- */

/**
 * @brief 回零 (使用默认模式 HOME_LIMIT_Z)
 */
void Stepper_Home(Stepper_ID_t id);

/**
 * @brief 回零 (带模式选择)
 * @param id   电机ID
 * @param mode 模式: STEPPER_HOME_NEAR / _LIMIT_Z / _COLLISION / _Z_LIMIT
 */
void Stepper_HomeWithMode(Stepper_ID_t id, uint8_t mode);

/**
 * @brief 强制中断回零
 */
void Stepper_HomeAbort(Stepper_ID_t id);

/* ---------- 同步运动 ---------- */

/**
 * @brief 同步触发 — 启动所有 snF=1 的缓存命令
 * @param id 电机ID (通常用 0x00 广播)
 */
void Stepper_SyncTrigger(Stepper_ID_t id);

/* ---------- 系统命令 ---------- */

void Stepper_SetCurrentPosZero(Stepper_ID_t id);
void Stepper_EncoderCalibrate(Stepper_ID_t id);
void Stepper_ResetMotor(Stepper_ID_t id);
void Stepper_SaveParams(Stepper_ID_t id);

/* ---------- 参数读写 ---------- */

/**
 * @brief 读取单个系统参数
 * @param id   电机ID
 * @param code 参数码 (STEPPER_PARAM_xxx)
 * @return uint8_t 读取到的值, 或 0 失败
 * @note 发送后阻塞等待响应, 超时约100ms
 */
uint8_t Stepper_ReadParam(Stepper_ID_t id, uint8_t code);

/**
 * @brief 写入参数 (双字节命令码)
 * @param id    电机ID
 * @param cmd1  命令码1
 * @param cmd2  命令码2
 * @param data  参数数据
 * @param len   数据长度 (不含snF和6B)
 */
void Stepper_WriteParam(Stepper_ID_t id, uint8_t cmd1, uint8_t cmd2,
                        const uint8_t *data, uint8_t len);

/* ---------- 状态查询 ---------- */

Stepper_State_t Stepper_GetState(Stepper_ID_t id);
const uint8_t* Stepper_GetResponse(Stepper_ID_t id);
uint8_t Stepper_GetResponseLen(Stepper_ID_t id);
bool Stepper_IsResponseReady(Stepper_ID_t id);
Stepper_Handle_t* Stepper_GetHandle(Stepper_ID_t id);

/* ---------- 响应读取(非阻塞, 周期性调用) ---------- */

void Stepper_UpdateResponse(Stepper_ID_t id);

/* 调试全局变量: 电机句柄数组 (状态/响应缓冲, 调试器实时监视) */
extern Stepper_Handle_t g_stepper_handles[STEPPER_NUM];

/* ---------- 调试测试 ---------- */

void Stepper_RecipTest(Stepper_ID_t id, uint32_t pulses, uint16_t rpm, uint16_t cycles);

void Stepper_RunTest(Stepper_ID_t id);
uint8_t Stepper_GetTestStep(Stepper_ID_t id);
bool Stepper_IsTestRunning(Stepper_ID_t id);
uint16_t Stepper_GetTestError(Stepper_ID_t id);

#ifdef __cplusplus
}
#endif

#endif /* __STEPPER_MOTOR_H */
