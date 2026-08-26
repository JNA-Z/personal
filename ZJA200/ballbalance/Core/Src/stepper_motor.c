/**
 * @file stepper_motor.c
 * @brief X42S 闭环步进电机驱动 (Emm_V5.0 全协议实现)
 * @version 4.0
 * @date 2026-07-30
 *
 * @details
 * 完整实现 ZDT_X42S 步进电机 Emm_V5.0 协议:
 *   - F6 速度模式、FD 位置模式
 *   - 使能/关闭、回零(4种模式)
 *   - 同步触发、编码器校准、复位
 *   - 参数读写(19个系统参数 + 14个写入命令)
 *
 * 帧格式: [Addr(1)] [Cmd(1~2)] [Data...] [0x6B]
 * 字节序: 大端 (Big-Endian)
 * 方向:   0=CW, 非0=CCW
 * snF:    0=立即执行, 1=等待同步触发
 */

#include "stepper_motor.h"
#include <string.h>

/*============================================================================*/
/*                          私有变量                                         */
/*============================================================================*/

/* ---- 全局句柄 (调试器实时监视电机状态/响应) ---- */
Stepper_Handle_t g_stepper_handles[STEPPER_NUM] = {0};
static bool g_stepper_initialized[STEPPER_NUM] = {false};

/*============================================================================*/
/*                          私有函数                                         */
/*============================================================================*/

static bool Stepper_ValidateId(Stepper_ID_t id)
{
    return (id < STEPPER_NUM);
}

static void Stepper_Transmit(Stepper_ID_t id, const uint8_t *frame, uint8_t len)
{
    if (!Stepper_ValidateId(id) || !g_stepper_initialized[id])
        return;
    UART_HandleTypeDef *huart = g_stepper_handles[id].config.huart;
    if (huart == NULL) return;
    HAL_UART_Transmit(huart, (uint8_t *)frame, len, 100);
}

static void Stepper_Send3B(Stepper_ID_t id, uint8_t cmd)
{
    uint8_t frame[3] = {g_stepper_handles[id].config.motor_id, cmd, STEPPER_TAIL};
    Stepper_Transmit(id, frame, 3);
}

static void Stepper_Send4B(Stepper_ID_t id, uint8_t cmd1, uint8_t cmd2)
{
    uint8_t frame[4] = {g_stepper_handles[id].config.motor_id, cmd1, cmd2, STEPPER_TAIL};
    Stepper_Transmit(id, frame, 4);
}

/*============================================================================*/
/*                          公共函数实现                                      */
/*============================================================================*/

/* ---------- 初始化和基本控制 ---------- */

bool Stepper_Init(Stepper_ID_t id)
{
    if (!Stepper_ValidateId(id))
        return false;

    if (g_stepper_initialized[id])
        Stepper_Deinit(id);

    g_stepper_handles[id].config.huart = &STEPPER_UART_HANDLE;
    g_stepper_handles[id].config.motor_id = STEPPER_MOTOR_ID_DEFAULT;
    g_stepper_handles[id].config.default_rpm = STEPPER_DEFAULT_RPM;
    g_stepper_handles[id].config.default_acc = STEPPER_DEFAULT_ACC;

    g_stepper_handles[id].data.state = STEPPER_STATE_IDLE;
    g_stepper_handles[id].data.is_fault = false;
    g_stepper_handles[id].data.test_step = 0;
    g_stepper_handles[id].data.test_running = false;
    g_stepper_handles[id].data.test_error = 0;
    g_stepper_handles[id].data.rx_len = 0;
    g_stepper_handles[id].data.rx_ready = false;
    memset(g_stepper_handles[id].data.rx_buf, 0, STEPPER_FRAME_RX_LEN);

    g_stepper_initialized[id] = true;
    return true;
}

void Stepper_Deinit(Stepper_ID_t id)
{
    if (!Stepper_ValidateId(id) || !g_stepper_initialized[id]) return;
    Stepper_Stop(id);
    g_stepper_handles[id].config.huart = NULL;
    g_stepper_initialized[id] = false;
}

void Stepper_SetSpeed(Stepper_ID_t id, uint16_t rpm, uint8_t acc)
{
    if (!Stepper_ValidateId(id) || !g_stepper_initialized[id]) return;
    g_stepper_handles[id].config.default_rpm = rpm;
    g_stepper_handles[id].config.default_acc = acc;
}

/* ---------- 运动控制 ---------- */

/**
 * @frame F6 速度模式 (8字节)
 *   [Addr] [0xF6] [Dir] [Vel_H][Vel_L] [Acc] [snF] [0x6B]
 */
void Stepper_RunF6(Stepper_ID_t id, uint8_t dir, uint16_t rpm, uint8_t acc)
{
    if (!Stepper_ValidateId(id) || !g_stepper_initialized[id])
        return;

    uint8_t frame[STEPPER_FRAME_F6_LEN] = {
        g_stepper_handles[id].config.motor_id,  // [0] Addr
        0xF6,                                    // [1] Cmd: 速度模式
        dir,                                     // [2] Dir: 0=CW, 非0=CCW
        (uint8_t)((rpm >> 8) & 0xFF),            // [3] Vel 高字节
        (uint8_t)(rpm & 0xFF),                   // [4] Vel 低字节
        acc,                                     // [5] Acc
        0x00,                                    // [6] snF=0 (立即)
        STEPPER_TAIL                             // [7] 0x6B
    };
    Stepper_Transmit(id, frame, STEPPER_FRAME_F6_LEN);
    g_stepper_handles[id].data.state = STEPPER_STATE_RUNNING;
}

/**
 * @frame FD 位置模式 (13字节)
 *   [Addr] [0xFD] [Dir] [Vel_H][Vel_L] [Acc] [Pulse(4B大端)] [raF] [snF] [0x6B]
 */
void Stepper_RunFD(Stepper_ID_t id, uint8_t dir, uint16_t rpm, uint8_t acc,
                   uint32_t pulses, uint8_t raF)
{
    if (!Stepper_ValidateId(id) || !g_stepper_initialized[id])
        return;

    uint8_t frame[STEPPER_FRAME_FD_LEN] = {
        g_stepper_handles[id].config.motor_id,  // [0]  Addr
        0xFD,                                    // [1]  Cmd: 位置模式
        dir,                                     // [2]  Dir
        (uint8_t)((rpm >> 8) & 0xFF),            // [3]  Vel 高字节
        (uint8_t)(rpm & 0xFF),                   // [4]  Vel 低字节
        acc,                                     // [5]  Acc
        (uint8_t)((pulses >> 24) & 0xFF),        // [6]  Pulse[31:24]
        (uint8_t)((pulses >> 16) & 0xFF),        // [7]  Pulse[23:16]
        (uint8_t)((pulses >> 8) & 0xFF),         // [8]  Pulse[15:8]
        (uint8_t)(pulses & 0xFF),                // [9]  Pulse[7:0]
        raF,                                     // [10] raF
        0x00,                                    // [11] snF=0
        STEPPER_TAIL                             // [12] 0x6B
    };
    Stepper_Transmit(id, frame, STEPPER_FRAME_FD_LEN);
    g_stepper_handles[id].data.state = STEPPER_STATE_RUNNING;
}

void Stepper_RunForward(Stepper_ID_t id)
{
    if (!Stepper_ValidateId(id) || !g_stepper_initialized[id]) return;
    Stepper_RunF6(id, STEPPER_DIR_CW,
                  g_stepper_handles[id].config.default_rpm,
                  g_stepper_handles[id].config.default_acc);
}

void Stepper_RunBackward(Stepper_ID_t id)
{
    if (!Stepper_ValidateId(id) || !g_stepper_initialized[id]) return;
    Stepper_RunF6(id, STEPPER_DIR_CCW,
                  g_stepper_handles[id].config.default_rpm,
                  g_stepper_handles[id].config.default_acc);
}

/**
 * @frame 停止 (5字节)
 *   [Addr] [0xFE] [0x98] [snF] [0x6B]
 */
void Stepper_Stop(Stepper_ID_t id)
{
    if (!Stepper_ValidateId(id) || !g_stepper_initialized[id]) return;

    uint8_t frame[STEPPER_FRAME_5B] = {
        g_stepper_handles[id].config.motor_id,
        0xFE, 0x98,
        0x00,                    // snF=0
        STEPPER_TAIL
    };
    Stepper_Transmit(id, frame, STEPPER_FRAME_5B);
    g_stepper_handles[id].data.state = STEPPER_STATE_IDLE;
}

/* ---------- 使能和状态 ---------- */

/**
 * @frame 使能/关闭 (6字节)
 *   [Addr] [0xF3] [0xAB] [State] [snF] [0x6B]
 */
static void Stepper_SetEnable(Stepper_ID_t id, uint8_t state)
{
    if (!Stepper_ValidateId(id) || !g_stepper_initialized[id]) return;

    uint8_t frame[STEPPER_FRAME_6B] = {
        g_stepper_handles[id].config.motor_id,
        0xF3, 0xAB,
        state,
        0x00,                    // snF=0
        STEPPER_TAIL
    };
    Stepper_Transmit(id, frame, STEPPER_FRAME_6B);
}

void Stepper_Enable(Stepper_ID_t id)
{
    Stepper_SetEnable(id, 1);
}

void Stepper_Disable(Stepper_ID_t id)
{
    Stepper_SetEnable(id, 0);
}

/* ---------- 回零 ---------- */

/**
 * @frame 触发回零 (6字节)
 *   [Addr] [0xF3] [0xAB] [Mode] [snF] [0x6B]
 */
void Stepper_HomeWithMode(Stepper_ID_t id, uint8_t mode)
{
    if (!Stepper_ValidateId(id) || !g_stepper_initialized[id]) return;

    uint8_t frame[STEPPER_FRAME_6B] = {
        g_stepper_handles[id].config.motor_id,
        0xF3, 0xAB,
        mode,
        0x00,                    // snF=0
        STEPPER_TAIL
    };
    Stepper_Transmit(id, frame, STEPPER_FRAME_6B);
    g_stepper_handles[id].data.state = STEPPER_STATE_RUNNING;
}

void Stepper_Home(Stepper_ID_t id)
{
    Stepper_HomeWithMode(id, STEPPER_HOME_LIMIT_Z);
}

/**
 * @frame 强制中断回零 (4字节)
 *   [Addr] [0x9C] [0x48] [0x6B]
 */
void Stepper_HomeAbort(Stepper_ID_t id)
{
    Stepper_Send4B(id, 0x9C, 0x48);
    g_stepper_handles[id].data.state = STEPPER_STATE_IDLE;
}

/* ---------- 同步运动 ---------- */

/**
 * @frame 同步触发 (3字节)
 *   [Addr] [0xFF] [0x66] [0x6B]
 */
void Stepper_SyncTrigger(Stepper_ID_t id)
{
    uint8_t addr = g_stepper_initialized[id] ?
                   g_stepper_handles[id].config.motor_id : 0x00;

    uint8_t frame[4] = {addr, 0xFF, 0x66, STEPPER_TAIL};

    UART_HandleTypeDef *huart = g_stepper_handles[0].config.huart;
    if (huart) HAL_UART_Transmit(huart, frame, 4, 100);
}

/* ---------- 系统命令 ---------- */

/**
 * @frame 当前位置归零 (3字节)
 *   [Addr] [0x0A] [0x6D] [0x6B]
 */
void Stepper_SetCurrentPosZero(Stepper_ID_t id)
{
    Stepper_Send4B(id, 0x0A, 0x6D);
}

/**
 * @frame 编码器校准 (3字节)
 *   [Addr] [0x06] [0x45] [0x6B]
 */
void Stepper_EncoderCalibrate(Stepper_ID_t id)
{
    Stepper_Send4B(id, 0x06, 0x45);
}

/**
 * @frame 复位电机 (3字节)
 *   [Addr] [0x08] [0x97] [0x6B]
 */
void Stepper_ResetMotor(Stepper_ID_t id)
{
    Stepper_Send4B(id, 0x08, 0x97);
}

/**
 * @frame 保存参数到Flash (3字节)
 *   [Addr] [0x2E] [0x6B]
 */
void Stepper_SaveParams(Stepper_ID_t id)
{
    Stepper_Send3B(id, 0x2E);
}

/* ---------- 参数读写 ---------- */

/**
 * @frame 读取参数 (3字节)
 *   [Addr] [ParamCode] [0x6B]
 *   电机回复: [Addr] [0xE2] [Value] [0x6B]  (4字节)
 */
uint8_t Stepper_ReadParam(Stepper_ID_t id, uint8_t code)
{
    if (!Stepper_ValidateId(id) || !g_stepper_initialized[id])
        return 0;

    UART_HandleTypeDef *huart = g_stepper_handles[id].config.huart;
    if (huart == NULL) return 0;

    /* 发送读取命令 */
    uint8_t cmd[3] = {g_stepper_handles[id].config.motor_id, code, STEPPER_TAIL};
    HAL_UART_Transmit(huart, cmd, 3, 100);

    /* 等待响应: [Addr] [0xE2] [Value] [0x6B] */
    uint8_t resp[4] = {0};
    if (HAL_UART_Receive(huart, resp, 4, 100) == HAL_OK) {
        if (resp[0] == g_stepper_handles[id].config.motor_id &&
            resp[1] == 0xE2 && resp[3] == STEPPER_TAIL) {
            return resp[2];  // 返回值
        }
        /* 兼容其他响应格式 */
        if (resp[3] == STEPPER_TAIL) {
            return resp[2];
        }
    }
    return 0;
}

/**
 * @frame 写入参数
 *   [Addr] [Cmd1] [Cmd2] [Data...] [snF(1)] [0x6B]
 */
void Stepper_WriteParam(Stepper_ID_t id, uint8_t cmd1, uint8_t cmd2,
                        const uint8_t *data, uint8_t len)
{
    if (!Stepper_ValidateId(id) || !g_stepper_initialized[id]) return;
    if (len > 8) return;  // 安全限制

    uint8_t frame[16] = {0};
    uint8_t idx = 0;

    frame[idx++] = g_stepper_handles[id].config.motor_id;
    frame[idx++] = cmd1;
    frame[idx++] = cmd2;

    for (uint8_t i = 0; i < len; i++) {
        frame[idx++] = data[i];
    }

    frame[idx++] = 0x00;        // snF=0
    frame[idx++] = STEPPER_TAIL;

    Stepper_Transmit(id, frame, idx);
}

/* ---------- 状态查询 ---------- */

Stepper_State_t Stepper_GetState(Stepper_ID_t id)
{
    if (!Stepper_ValidateId(id) || !g_stepper_initialized[id])
        return STEPPER_STATE_ERROR;
    return g_stepper_handles[id].data.state;
}

const uint8_t* Stepper_GetResponse(Stepper_ID_t id)
{
    if (!Stepper_ValidateId(id) || !g_stepper_initialized[id])
        return NULL;
    return g_stepper_handles[id].data.rx_buf;
}

uint8_t Stepper_GetResponseLen(Stepper_ID_t id)
{
    if (!Stepper_ValidateId(id) || !g_stepper_initialized[id])
        return 0;
    return g_stepper_handles[id].data.rx_len;
}

bool Stepper_IsResponseReady(Stepper_ID_t id)
{
    if (!Stepper_ValidateId(id) || !g_stepper_initialized[id])
        return false;
    bool ready = g_stepper_handles[id].data.rx_ready;
    if (ready) g_stepper_handles[id].data.rx_ready = false;  // auto-clear
    return ready;
}

Stepper_Handle_t* Stepper_GetHandle(Stepper_ID_t id)
{
    if (!Stepper_ValidateId(id) || !g_stepper_initialized[id])
        return NULL;
    return &g_stepper_handles[id];
}

/* ---------- 响应读取 (非阻塞, 由外部循环调用) ---------- */

void Stepper_UpdateResponse(Stepper_ID_t id)
{
    if (!Stepper_ValidateId(id) || !g_stepper_initialized[id])
        return;

    UART_HandleTypeDef *huart = g_stepper_handles[id].config.huart;
    if (huart == NULL) return;

    while (__HAL_UART_GET_FLAG(huart, UART_FLAG_RXNE) != RESET)
    {
        uint8_t byte = (uint8_t)(huart->Instance->DR & 0xFF);

        if (g_stepper_handles[id].data.rx_len < STEPPER_FRAME_RX_LEN)
        {
            g_stepper_handles[id].data.rx_buf[g_stepper_handles[id].data.rx_len++] = byte;

            if (byte == STEPPER_TAIL)
            {
                g_stepper_handles[id].data.rx_ready = true;
            }
        }
        else
        {
            /* 缓冲区满, 重置 */
            g_stepper_handles[id].data.rx_len = 0;
        }
    }
}

/* ---------- 调试测试 ---------- */

void Stepper_RecipTest(Stepper_ID_t id, uint32_t pulses, uint16_t rpm, uint16_t cycles)
{
    if (!Stepper_ValidateId(id) || !g_stepper_initialized[id])
        return;

    /* 计算单程运动时间: 脉冲数 / 3200脉冲每圈 → 圈数 → 秒 */
    float revs = (float)pulses / 3200.0f;
    float secs = revs * 60.0f / (float)rpm;
    uint32_t wait_ms = (uint32_t)(secs * 1000.0f) + 300;  /* +300ms余量 */

    for (uint16_t i = 0; i < cycles; i++)
    {
        /* 正转 */
        Stepper_RunFD(id, STEPPER_DIR_CW, rpm, 5, pulses, STEPPER_RA_RELATIVE);
        HAL_Delay(wait_ms);

        /* 反转 */
        Stepper_RunFD(id, STEPPER_DIR_CCW, rpm, 5, pulses, STEPPER_RA_RELATIVE);
        HAL_Delay(wait_ms);
    }

    Stepper_Stop(id);
}

void Stepper_RunTest(Stepper_ID_t id)
{
    if (!Stepper_ValidateId(id) || !g_stepper_initialized[id])
        return;

    Stepper_Data_t *data = &g_stepper_handles[id].data;
    data->test_running = true;
    data->test_error = 0;

    /* 使能电机 */
    Stepper_Enable(id);
    HAL_Delay(100);

    /* F6 速度模式: 正转 2秒 */
    data->test_step = STEPPER_TEST_RUN_F6;
    Stepper_RunF6(id, STEPPER_DIR_CW, 200, 5);
    HAL_Delay(2000);

    /* 停止 */
    data->test_step = STEPPER_TEST_STOP1;
    Stepper_Stop(id);
    HAL_Delay(800);

    /* FD 位置模式: 相对定位 +3200 脉冲 (1圈@16细分) */
    data->test_step = STEPPER_TEST_RUN_FD;
    Stepper_RunFD(id, STEPPER_DIR_CW, 200, 5, 3200, STEPPER_RA_RELATIVE);
    HAL_Delay(2000);

    /* 停止 */
    data->test_step = STEPPER_TEST_STOP2;
    Stepper_Stop(id);
    HAL_Delay(500);

    data->test_step = STEPPER_TEST_DONE;
    data->test_running = false;
}

uint8_t Stepper_GetTestStep(Stepper_ID_t id)
{
    if (!Stepper_ValidateId(id) || !g_stepper_initialized[id])
        return STEPPER_TEST_DONE;
    return g_stepper_handles[id].data.test_step;
}

bool Stepper_IsTestRunning(Stepper_ID_t id)
{
    if (!Stepper_ValidateId(id) || !g_stepper_initialized[id])
        return false;
    return g_stepper_handles[id].data.test_running;
}

uint16_t Stepper_GetTestError(Stepper_ID_t id)
{
    if (!Stepper_ValidateId(id) || !g_stepper_initialized[id])
        return 1;
    return g_stepper_handles[id].data.test_error;
}
