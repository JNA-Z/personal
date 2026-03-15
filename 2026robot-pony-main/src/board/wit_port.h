#ifndef BOARD_WIT_PORT_H
#define BOARD_WIT_PORT_H
/*
 * @brief 初始化维特陀螺仪端口
 * 初始化串口和DMA，绑定数据接收函数和获取DMA写入索引函数
 */

#include "../util/datatype.h"

void wit_port_init(void);

#endif // !BOARD_WIT_PORT_H
