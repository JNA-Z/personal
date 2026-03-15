#include "encoder_port.h"
#include "../driver/encoder.h"
#include "main.h"


/**
 * @brief 启动编码器相关定时器基础功能
 * 
 * @param tim 定时器句柄
 */
static void encoder_tim_base_start(void* tim)
{
    // 启动时基单元并启用中断
    HAL_TIM_Base_Start_IT((TIM_HandleTypeDef*)tim);
}

/**
 * @brief 启动编码器相关定时器特定功能
 * 
 * @param tim 定时器句柄
 * @param channel 通道
 */
static void encoder_tim_start(void* tim, u32 channel)
{
    // 若使用定时器的编码器模式，需启动定时器的编码器功能
    HAL_TIM_Encoder_Start((TIM_HandleTypeDef*)tim, channel);

    // 若使用定时器的外部时钟模式，需启动定时器的基础功能
    // HAL_TIM_Base_Start_IT((TIM_HandleTypeDef*)tim);

    // 若使用定时器的输入捕获模式，需启动定时器的输入捕获功能
    // HAL_TIM_IC_Start_IT((TIM_HandleTypeDef*)tim, channel);
}
/**
 * @brief 读取编码器相关定时器的计数值
 * 
 * @param tim 定时器句柄
 * @param channel 通道
 * @return u32 当前计数值
 */
static u32 encoder_read_tim_cnt(void* tim, u32 channel)
{
    // 若使用定时器编码器模式，定时器根据AB相自动计数到计数器，应获取计数器的值
    return __HAL_TIM_GetCounter((TIM_HandleTypeDef*)tim);

    // 若使用定时器外部时钟模式，定时器根据输入信号自动计数到计数器，应获取计数器的值
    // return __HAL_TIM_GetCounter((TIM_HandleTypeDef*)tim);

    // 若使用定时器输入捕获模式，定时器根据输入信号，捕获计数器的值到捕获比较寄存器，应获取捕获比较寄存器的值
    // return __HAL_TIM_GetCompare((TIM_HandleTypeDef*)tim, channel);
}

/**
 * @brief 清零编码器相关定时器的计数值
 * 
 * @param tim 定时器句柄
 * @param channel 通道
 */
static void encoder_clear_tim_cnt(void* tim, u32 channel)
{
    __HAL_TIM_SetCounter((TIM_HandleTypeDef*)tim, 0);
    // __HAL_TIM_SetCompare((TIM_HandleTypeDef*)tim, channel, 0);
}

static const encoder_port_t g_encoder_port={
    .tim_base_start = encoder_tim_base_start,    
    .tim_start = encoder_tim_start,
    .read_tim_cnt = encoder_read_tim_cnt,
    .clear_tim_cnt = encoder_clear_tim_cnt,
};

bool encoder_port_init(void)
{
    encoder_bind_port(&g_encoder_port);

    return encoder_port_is_registered();
}


