#include "wit.h"
#include <string.h>

static wit_port_t *g_wit_port = NULL;

void wit_bind_port(const wit_port_t *port) 
{ 
    g_wit_port = (wit_port_t *)port; 
}

/**
 * @brief 维特陀螺仪初始化
 * 
 * @param wit 
 */
void wit_init(wit_t *wit) {
    // 开启串口DMA接收
    g_wit_port->data_receive(wit->uart, wit->dma, wit->rx_buf, BUFF_SIZE);
}

// 数据解析函数
/**
 * @brief 从WIT传感器DMA接收缓冲区中解析指定类型的数据帧
 * 
 * 该函数从环形缓冲区中逆向搜索最新的有效数据包，解析并转换为浮点数格式。
 * WIT传感器数据包格式：0x55 + 数据类型 + 8字节数据 + 校验和
 * 
 * @param wit WIT传感器实例指针，包含DMA句柄和接收缓冲区
 * @param data_type 要解析的数据类型枚举值
 * @param pdata 输出参数，存储解析后的浮点数数据数组
/**
 * @brief 维特陀螺仪数据帧解析
 * 
 * @param wit 
 * @param data_type 
 * @param pdata 
 */
void wit_data_frame(wit_t *wit, wit_data_type_t data_type, float *pdata) {
    // 获取DMA写入索引（当前DMA接收位置）
    uint32_t index = g_wit_port->get_dma_wrindex(wit->dma, BUFF_SIZE);

    // 开始从缓冲区回退寻找所需的最新数据包
    // 采用逆向搜索策略，从最新数据向前查找，确保获取最新有效数据
    for (uint32_t i = 0; i < BUFF_SIZE; i++) {

        // 环形缓冲区索引回退处理：如果为0则回绕到缓冲区末尾
        index = index == 0 ? BUFF_SIZE - 1 : index - 1;

        // 包头及数据类型匹配：检查是否为WIT数据包（包头0x55 + 匹配的数据类型）
        if (wit->rx_buf[index] == 0x55 && wit->rx_buf[(index + 1) % BUFF_SIZE] == data_type) {
            // 提取数据并进行校验和验证
            uint8_t data[8];  // 存储8字节原始数据
            uint8_t sum = 0x55 + data_type;  // 初始化校验和（包头 + 数据类型）
            
            // 提取8字节数据并计算校验和
            for (int j = 0; j < 8; j++) {
                data[j] = wit->rx_buf[(index + j + 2) % BUFF_SIZE];  // 跳过包头和类型字节
                sum += data[j];   // 累加数据字节到校验和
            }
            
            // 获取数据包末尾的校验字节（第11个字节）
            uint8_t check_bit = wit->rx_buf[(index + 10) % BUFF_SIZE];

            // 校验通过：计算的和与校验字节匹配
            if (sum == check_bit) {
                // 根据数据类型进行相应的数据解析和转换
                switch (data_type)
                {
                // 时间数据解析
                case WIT_TYPE_TIME:
                    pdata[0] = (float)data[0] + 2000.0f;  // 年（数据+2000）
                    pdata[1] = (float)data[1];           // 月
                    pdata[2] = (float)data[2];           // 日
                    pdata[3] = (float)data[3];           // 时
                    pdata[4] = (float)data[4];           // 分
                    pdata[5] = (float)data[5];           // 秒
                    pdata[6] = (float)(int16_t)(data[7] << 8 | data[6]);  // 毫秒（2字节组合）
                    break;
                
                // 加速度数据解析（单位：g）
                case WIT_TYPE_ACC:
                    // X轴加速度：16位有符号数转换为浮点，范围±16g
                    pdata[0] = (float)(int16_t)(data[1] << 8 | data[0]) / 32768.0f * 16.0f;
                    // Y轴加速度
                    pdata[1] = (float)(int16_t)(data[3] << 8 | data[2]) / 32768.0f * 16.0f;
                    // Z轴加速度
                    pdata[2] = (float)(int16_t)(data[5] << 8 | data[4]) / 32768.0f * 16.0f;
                    // 温度数据
                    pdata[3] = (float)(int16_t)(data[7] << 8 | data[6]) / 100.0f;
                    break;
                
                // 角速度数据解析（单位：°/s）
                case WIT_TYPE_GYRO:
                    // X轴角速度：16位有符号数转换为浮点，范围±2000°/s
                    pdata[0] = (float)(int16_t)(data[1] << 8 | data[0]) / 32768.0f * 2000.0f;
                    // Y轴角速度
                    pdata[1] = (float)(int16_t)(data[3] << 8 | data[2]) / 32768.0f * 2000.0f;
                    // Z轴角速度
                    pdata[2] = (float)(int16_t)(data[5] << 8 | data[4]) / 32768.0f * 2000.0f;
                    // 电压数据（非蓝牙产品无效）
                    pdata[3] = (float)(int16_t)(data[7] << 8 | data[6]) / 100.0f;
                    break;
                
                // 欧拉角数据解析（单位：°）
                case WIT_TYPE_ANGLE:
                    // 横滚角roll（X轴）：16位有符号数转换为浮点，范围±180°
                    pdata[0] = (float)(int16_t)(data[1] << 8 | data[0]) / 32768.0f * 180.0f;
                    // 俯仰角pitch（Y轴）
                    pdata[1] = (float)(int16_t)(data[3] << 8 | data[2]) / 32768.0f * 180.0f;
                    // 航向角yaw（Z轴）
                    pdata[2] = (float)(int16_t)(data[5] << 8 | data[4]) / 32768.0f * 180.0f;
                    // 版本号
                    pdata[3] = (float)(int16_t)(data[7] << 8 | data[6]);
                    break;
                
                // 其他数据类型
                case WIT_TYPE_MAG:
                case WIT_TYPE_PORT_STATUS:
                case WIT_TYPE_PRESS_ALT:
                case WIT_TYPE_LON_LAT:
                case WIT_TYPE_GPS_SPEED:
                case WIT_TYPE_QUAT:
                case WIT_TYPE_GPS_ACCURACY:
                default:
                    break;  // 预留接口，后续可扩展
                }

                break; // 数据包已成功处理，退出搜索循环
            }
        }
    }
}
