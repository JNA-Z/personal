/**
 * @file lcd_menu.h
 * @author Qiao
 * @brief LCD菜单系统头文件
 * @version 0.1
 * @date 2026-01-22 以OOP方式实现 
 */
 
#ifndef LCD_MENU_H
#define LCD_MENU_H

#include "../util/datatype.h"
#include "driver/key.h"

// port
typedef struct lcd_menu_port{
    void (*show_string)(u16 x, u16 y, u8* p, u16 pen_color, u16 back_color);
    void (*clear)(u16 color);
    void (*key_scan)(key_t *keys, usize keys_size);
    void (*delay_ms)(u32 ms);  
}lcd_menu_port_t;

//绑定port
void lcd_menu_bind_port(const lcd_menu_port_t *port);

// 页面枚举
typedef enum {
    PAGE_MAIN = 0,
    PAGE_BASIC,
    PAGE_ADVANCE,
    PAGE_TASK1,
    PAGE_TASK2,
    PAGE_TASK3,
    PAGE_TASK4
    // ... 可扩展更多页面
} menu_page_e;

// 菜单主结构体
typedef struct lcd_menu {
    // 状态变量
    menu_page_e current_page; // 当前页面
    u8          cursor_pos;   // 当前光标位置
    bool        need_refresh; // 刷新标志位
    bool        is_running;   // 运行标志

    // 硬件资源引用 (聚合)
    key_t*      keys;         // 指向按键数组
    usize       key_cnt;      // 按键数量
} lcd_menu_t;

//API

//初始化lcd_menu
void lcd_menu_init(lcd_menu_t* menu);

//主循环
void lcd_menu_loop(lcd_menu_t* menu);

#endif // !LCD_MENU_H
