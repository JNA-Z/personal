/**
 * @file menu.h
 * @brief LCD调试显示模块
 * @note 用于替代LCD_Show()，支持多页面切换显示调试数据
 */

#ifndef __MENU_H
#define __MENU_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * 菜单页面枚举
 * ============================================================================ */
typedef enum {
    MENU_PAGE_MAIN = 0,      /* 主页面：编码器计数 + ADC通道5 */
    MENU_PAGE_ADC_RAW,       /* ADC原始数据 */
    MENU_PAGE_ENCODER,       /* 编码器详情 */
    MENU_PAGE_MOTOR,         /* 电机状态 */
    MENU_PAGE_COUNT          /* 页面总数 */
} menu_page_t;

/* ============================================================================
 * 接口函数
 * ============================================================================ */

/**
 * @brief 初始化菜单模块
 */
void menu_init(void);

/**
 * @brief 切换到下一页
 */
void menu_next_page(void);

/**
 * @brief 刷新当前页面显示（替代LCD_Show）
 * @note 应在主循环中周期调用
 */
void menu_refresh(void);

#ifdef __cplusplus
}
#endif

#endif /* __MENU_H */
