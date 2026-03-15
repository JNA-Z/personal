#ifndef _DELAY_H
#define _DELAY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void delay_init(uint8_t SYSCLK);
void delay_ms(uint16_t nms);
void delay_us(uint32_t nus);

/**
 * @brief 非阻塞式按 RTOS tick 延时
 * @param ticks 需要延时的 tick 数
 */
void delay_tick_noblock(uint32_t ticks);

/**
 * @brief 非阻塞式按毫秒延时（内部完成 ms 到 tick 的换算）
 * @param ms 需要延时的毫秒数
 */
void delay_ms_noblock(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif
