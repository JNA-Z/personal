#include "wit_port.h"
#include "../driver/wit.h"
#include "main.h"


static wit_port_t g_wit_port = {0};




// wit数据接收的串口DMA接收函数实现
/**
 * @brief WIT传感器数据接收适配函数 - STM32 HAL库实现
 * 
 * 该函数是硬件适配层的一部分，负责启动UART的DMA接收功能。
 * 通过函数指针机制实现跨平台兼容，当前使用STM32 HAL库实现。
 * 
 * @param uart UART外设句柄指针，需要转换为具体的HAL句柄类型
 * @param dma DMA句柄指针（在当前STM32实现中未直接使用）
 * @param pData 接收数据缓冲区指针，用于存储传感器原始数据
 * @param size 接收数据缓冲区大小，确保DMA传输不会溢出
 * 
 * @note 该函数通过适配层设计支持多种MCU平台：
 * - STM32平台：使用HAL_UART_Receive_DMA函数
 * - TI平台：使用uDMA相关函数（已注释，可启用）
 * 
 * @warning 参数类型转换需要确保正确性，错误的转换可能导致硬件异常
 * 
 * @see wit_port_init() - 适配层初始化函数
 * @see HAL_UART_Receive_DMA() - STM32 HAL库DMA接收函数
 */
static void wit_data_receive(void* uart, void* dma, uint8_t* pData, uint32_t size)
{
    // STM32平台实现 - 使用HAL库启动UART DMA接收
    // 将void*类型的uart参数转换为具体的UART_HandleTypeDef*类型
    // 启动DMA接收后，数据会自动填充到pData指向的缓冲区
    HAL_UART_Receive_DMA((UART_HandleTypeDef*)uart, pData, (uint16_t)size);
    
    // DMA接收启动后：
    // 1. 传感器数据通过DMA自动传输到指定缓冲区
    // 2. 接收过程不占用CPU资源，实现高效数据采集
    // 3. 当缓冲区满或达到指定长度时触发DMA完成中断

    // =========================================================================
    // TI平台实现（已注释，供参考和未来移植使用）
    // =========================================================================
    // uDMAChannelTransferSet((uint32_t)dma | UDMA_PRI_SELECT,  // DMA通道配置（主通道）
    //                        UDMA_MODE_BASIC,                  // 基本传输模式
    //                        (void *)((uint32_t)uart + UART_O_DR),  // 源地址：UART数据寄存器
    //                        pData,                            // 目标地址：数据缓冲区
    //                        size);                            // 传输数据大小
    //                        
    // uDMAChannelEnable((uint32_t)dma);                        // 使能DMA通道
    // UARTDMAEnable((uint32_t)uart, UART_DMA_RX);              // 使能UART的DMA接收功能
}


/**
 * @brief 获取DMA写入索引
 * 
 * 该函数用于获取DMA写入索引，用于判断DMA缓冲区的剩余空间。
 * 具体实现取决于DMA控制器的寄存器和操作方式。
 * 
 * @param dma DMA句柄指针 
 * @return DMA写入索引
 * @note 该函数需要根据具体的DMA控制器和寄存器进行实现。
 * 通常，DMA写入索引是通过DMA计数器寄存器获取的，
 * 计数器的值表示DMA已经写入的数据量。https://github.com/Lab-IEC/stm32_template_project/pull/15/conflict?name=stm32f4xx_template_project%252Fsrc%252Fboard%252Fwit_port.h&ancestor_oid=e8bfd7e75ce91cb8f175bccfdde978f73c38d865&base_oid=addb23ce0a4ea1e5cf95f4c7a8db4ac061a331f2&head_oid=ee3de92ce863351341484bf5bfae8ad0299a8d0d
 * 
 */
static void wit_data_send(void* uart, uint8_t* pData, uint32_t size)
{
    HAL_UART_Transmit((UART_HandleTypeDef*)uart, pData, (uint16_t)size, HAL_MAX_DELAY);
}

static uint32_t wit_get_dma_wrindex(void* dma, uint32_t buff_size)
{
    return buff_size - __HAL_DMA_GET_COUNTER((DMA_HandleTypeDef*)dma);
}


/**
 * @brief 初始化WIT传感器端口
 * 
 * 该函数用于初始化WIT传感器端口，包括设置数据接收函数和DMA写入索引获取函数。
 * 具体实现取决于具体的MCU平台和DMA控制器。
 *
 * @note 该函数需要根据具体的MCU平台和DMA控制器进行实现。

 */
static void delay_ms(u32 ms)
{
    HAL_Delay(ms);
}

void wit_port_init(void)
{
    g_wit_port.data_receive = wit_data_receive;
    g_wit_port.get_dma_wrindex = wit_get_dma_wrindex;
    g_wit_port.data_send = wit_data_send;
    g_wit_port.delay_ms = delay_ms;
    wit_bind_port(&g_wit_port);
}
