/**
 * @file wit.h
 * @author Easy (1609010708@qq.com)
 * @brief 维特陀螺仪驱动，port是否初始化由适配层保证，否则会有空指针风险
 * @version 0.1
 * @date 2026-01-11
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef DRIVER_WIT_H
#define DRIVER_WIT_H

#include "../util/datatype.h"
#include <stdint.h>

// 缓冲区大小
#define BUFF_SIZE 128


/**
 * @brief wit实例结构体
 * 包含uart句柄，dma句柄，和接收缓冲区
 * @param uart uart句柄指针
 * @param dma dma句柄指针
 * @param rx_buf 接收缓冲区
 */
typedef struct wit
{
    void* uart;  
    void* dma;
    uint8_t rx_buf[BUFF_SIZE];
}wit_t;


/**
 * @brief 端口结构体
 * 内部存放两个数据函数指针，用于数据接收和获取DMA写入索引
 * @param data_receive 数据接收函数指针，
 * @param get_dma_wrindex 获取DMA写入索引函数指针，用于获取当前DMA接收位置，还有数据长度
 */
typedef struct wit_port{
    void (*data_receive)(void* uart, void* dma, uint8_t* pData, uint32_t size);
    void (*data_send)(void* uart, uint8_t* pData, uint32_t size);
    uint32_t (*get_dma_wrindex)(void* dma, uint32_t buff_size);
    void (*delay_ms)(u32 ms);
} wit_port_t;

// 数据枚举
typedef enum wit_data_type{
    WIT_TYPE_TIME            = 0x50, // 时间
    WIT_TYPE_ACC             = 0x51, // 加速度
    WIT_TYPE_GYRO            = 0x52, // 角速度
    WIT_TYPE_ANGLE           = 0x53, // 角度
    WIT_TYPE_MAG             = 0x54, // 磁场
    WIT_TYPE_PORT_STATUS     = 0x55, // 端口状态
    WIT_TYPE_PRESS_ALT       = 0x56, // 气压高度
    WIT_TYPE_LON_LAT         = 0x57, // 经纬度
    WIT_TYPE_GPS_SPEED       = 0x58, // 地速
    WIT_TYPE_QUAT            = 0x59, // 四元数
    WIT_TYPE_GPS_ACCURACY    = 0x5A, // GPS定位精度
    WIT_TYPE_READ            = 0x5F  // 读取
} wit_data_type_t;

// 绑定port
void wit_bind_port(const wit_port_t *port);

// 陀螺仪初始化
void wit_init(wit_t *wit);

// 数据解析
void wit_data_frame(wit_t *wit, wit_data_type_t data_type, float *pdata);

#endif // !DRIVER_WIT_H
