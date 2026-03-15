#ifndef __RUNTIME_BSP_H__
#define __RUNTIME_BSP_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct Sflag_Device;

/**
 * @brief 判断启动按键是否被按下
 * @return 1: 已按下; 0: 未按下
 */
uint8_t bsp_start_key_is_pressed(void);

/**
 * @brief 判断挡板是否处于遮挡状态
 * @return 1: 遮挡; 0: 非遮挡
 */
uint8_t bsp_barrier_is_blocked(void);

/**
 * @brief 判断挡板是否已移开
 * @return 1: 已移开; 0: 未移开
 */
uint8_t bsp_barrier_is_cleared(void);

/**
 * @brief 启动摄像头串口单字节中断接收
 */
void bsp_camera_start_rx(void);

/**
 * @brief 采样设备状态并写入状态结构
 * @param state 设备状态结构体指针
 */
void bsp_sample_device_state(struct Sflag_Device* state);

#ifdef __cplusplus
}
#endif

#endif
