#include "lcd_menu.h"
#include "key.h"
#include "util/datatype.h"

#define C_BACK   0x0000 // BLACK
#define C_TEXT   0xFFFF // WHITE

typedef enum {
    KEY_NONE  = 0,
    KEY_UP    = 1,
    KEY_DOWN  = 2,
    KEY_BACK  = 3,
    KEY_ENTER = 4,
} key_bit_e;

static lcd_menu_port_t* g_lcd_menu_port = NULL;

void lcd_menu_bind_port(const lcd_menu_port_t* port)
{
    g_lcd_menu_port = (lcd_menu_port_t*)port;
}

// 初始化菜单状态
void lcd_menu_init(lcd_menu_t* menu) {
    menu->current_page = PAGE_MAIN;
    menu->cursor_pos   = 0;
    menu->need_refresh = true;
    menu->is_running   = true;
}

// 0=无, 1=上, 2=下, 3=返回, 4=确认
static int get_input(lcd_menu_t* menu) {
    if (! g_lcd_menu_port || !menu->keys) return KEY_NONE;

    // 调用硬件扫描
     g_lcd_menu_port->key_scan(menu->keys, menu->key_cnt);

    // 读取标志位 (假设 keys[0]=Up, keys[1]=Down, keys[2]=Back, keys[3]=Enter)
    if (menu->keys[0].short_flag) { menu->keys[0].short_flag = 0; return KEY_UP; }
    if (menu->keys[1].short_flag) { menu->keys[1].short_flag = 0; return KEY_DOWN; }
    if (menu->keys[2].short_flag) { menu->keys[2].short_flag = 0; return KEY_BACK; }
    if (menu->keys[3].short_flag) { menu->keys[3].short_flag = 0; return KEY_ENTER; }

    return KEY_NONE;
}

// ---------------------------------------------------------
// 页面逻辑函数
// ---------------------------------------------------------

//main
static void page_main(lcd_menu_t* menu) {
    if (menu->need_refresh) {
        if ( g_lcd_menu_port->clear)  g_lcd_menu_port->clear(C_BACK);
        g_lcd_menu_port->show_string(10, 0,  (u8*)"MAIN MENU", C_TEXT,C_BACK);
        g_lcd_menu_port->show_string(20, 30, (menu->cursor_pos == 0 ? (u8*)"-> Basic" :(u8*) "   Basic"), C_TEXT,C_BACK);
        g_lcd_menu_port->show_string(20, 50, (menu->cursor_pos == 1 ? (u8*)"-> Advance" : (u8*)"   Advance"), C_TEXT,C_BACK);
        menu->need_refresh = false;
    }

    int key = get_input(menu);
    if (key == KEY_UP) { // Up
        menu->cursor_pos = 0;
        menu->need_refresh = true;
    } else if (key == KEY_DOWN) { // Down
        menu->cursor_pos = 1;
        menu->need_refresh = true;
    } else if (key == KEY_ENTER) { // Enter
        menu->current_page = (menu->cursor_pos == 0) ? PAGE_BASIC : PAGE_ADVANCE;
        menu->cursor_pos = 0;
        menu->need_refresh = true;
    }
}

//main->basic
static void page_basic(lcd_menu_t* menu) {
    if (menu->need_refresh) {
        if ( g_lcd_menu_port->clear)  g_lcd_menu_port->clear(C_BACK);
        g_lcd_menu_port->show_string(10, 0,  (u8*)"BASIC MENU", C_TEXT,C_BACK);
        g_lcd_menu_port->show_string(20, 30, (menu->cursor_pos == 0 ? (u8*)"-> Task 1" : (u8*)"   Task 1"), C_TEXT,C_BACK);
        g_lcd_menu_port->show_string(20, 50, (menu->cursor_pos == 1 ? (u8*)"-> Task 2" : (u8*)"   Task 2"), C_TEXT,C_BACK);
        menu->need_refresh = false;
    }

    int key = get_input(menu);
    if (key == KEY_UP) { // Up
        if (menu->cursor_pos > 0) menu->cursor_pos--;
        menu->need_refresh = true;
    } else if (key == KEY_DOWN) { // Down
        if (menu->cursor_pos < 1) menu->cursor_pos++;
        menu->need_refresh = true;
    } else if (key == KEY_BACK) { // Back
        menu->current_page = PAGE_MAIN;
        menu->cursor_pos = 0;
        menu->need_refresh = true;
    } else if (key == KEY_ENTER) { // Enter
        if (menu->cursor_pos == 0) {
            menu->current_page = PAGE_TASK1;
            menu->cursor_pos = 1; 
            menu->need_refresh = true;
        }
        else if (menu->cursor_pos == 1) {
            menu->current_page = PAGE_TASK2;
            menu->cursor_pos = 1; 
            menu->need_refresh = true;
        }
    }
}

//main->advance
static void page_advance(lcd_menu_t* menu) {
    if (menu->need_refresh) {
        if ( g_lcd_menu_port->clear)  g_lcd_menu_port->clear(C_BACK);
        g_lcd_menu_port->show_string(10, 0,  (u8*)"ADVANCE MENU", C_TEXT,C_BACK);
        g_lcd_menu_port->show_string(20, 30, (menu->cursor_pos == 0 ? (u8*)"-> Task 3" : (u8*)"   Task3"), C_TEXT,C_BACK);
        g_lcd_menu_port->show_string(20, 50, (menu->cursor_pos == 1 ? (u8*)"-> Task 4" : (u8*)"   Task 4"), C_TEXT,C_BACK);
        menu->need_refresh = false;
    }

    int key = get_input(menu);
    if (key == KEY_UP) { // Up
        if (menu->cursor_pos > 0) menu->cursor_pos--;
        menu->need_refresh = true;
    } else if (key == KEY_DOWN) { // Down
        if (menu->cursor_pos < 1) menu->cursor_pos++;
        menu->need_refresh = true;
    } else if (key == KEY_BACK) { // Back
        menu->current_page = PAGE_MAIN;
        menu->cursor_pos = 0;
        menu->need_refresh = true;
    } else if (key == KEY_ENTER) { // Enter
        if (menu->cursor_pos == 0) {
            menu->current_page = PAGE_TASK3;
            menu->cursor_pos = 1; 
            menu->need_refresh = true;
        }
        else if (menu->cursor_pos == 1) {
            menu->current_page = PAGE_TASK4;
            menu->cursor_pos = 1; 
            menu->need_refresh = true;
        }
    }
}

//basic->task1
static void page_task1(lcd_menu_t* menu) {
    if (menu->need_refresh) {
        if ( g_lcd_menu_port->clear)  g_lcd_menu_port->clear(C_BACK);
        g_lcd_menu_port->show_string(0, 0, (u8*)"TASK 1", C_TEXT,C_BACK);
        menu->need_refresh = false;
    }
    int key = get_input(menu);
    if (key == KEY_UP) { // Up
        
    } else if (key == KEY_DOWN) { // Down
        
    } else if (key == KEY_BACK) { // Back
        
    } else if (key == KEY_ENTER) { // Enter
        if ( g_lcd_menu_port->clear)  g_lcd_menu_port->clear(C_BACK);
        g_lcd_menu_port->show_string(20, 60, (u8*)"A", C_TEXT,C_BACK);
         g_lcd_menu_port->delay_ms(1000);
        //menu->is_running = false;
        menu->current_page = PAGE_BASIC;
        menu->cursor_pos = 0;
        menu->need_refresh = true;
        //函数执行结束 返回上一级菜单
    }
}

//basic->task2
static void page_task2(lcd_menu_t* menu) {
    if (menu->need_refresh) {
        if ( g_lcd_menu_port->clear)  g_lcd_menu_port->clear(C_BACK);
        g_lcd_menu_port->show_string(0, 0, (u8*)"TASK 2", C_TEXT,C_BACK);
        menu->need_refresh = false;
    }

    int key = get_input(menu);
    if (key == KEY_UP) { // Up

    } else if (key == KEY_DOWN) { // Down

    } else if (key == KEY_BACK) { // Back

    } else if (key == 4) { // Enter
        if ( g_lcd_menu_port->clear)  g_lcd_menu_port->clear(C_BACK);
        g_lcd_menu_port->show_string(20, 60, (u8*)"B", C_TEXT,C_BACK);
        g_lcd_menu_port->delay_ms(1000);
        //menu->is_running = false;
        menu->current_page = PAGE_BASIC;
        menu->cursor_pos = 0;
        menu->need_refresh = true;
        //函数执行结束 返回上一级菜单  
    }
}

//advance->task3
static void page_task3(lcd_menu_t* menu) {
   
    if (menu->need_refresh) {
        if ( g_lcd_menu_port->clear)  g_lcd_menu_port->clear(C_BACK);
        g_lcd_menu_port->show_string(0, 0, (u8*)"TASK 3", C_TEXT,C_BACK);
        menu->need_refresh = false;
    }

int key = get_input(menu);
    if (key == KEY_UP) { // Up
        
    } else if (key == KEY_DOWN) { // Down
        
    } else if (key == KEY_BACK) { // Back
        
    } else if (key == KEY_ENTER) { // Enter
        if ( g_lcd_menu_port->clear)  g_lcd_menu_port->clear(C_BACK);
        g_lcd_menu_port->show_string(20, 60, (u8*)"C", C_TEXT,C_BACK);
        g_lcd_menu_port->delay_ms(1000);
        //menu->is_running = false;
        menu->current_page = PAGE_ADVANCE;
        menu->cursor_pos = 0;
        menu->need_refresh = true;
        //函数执行结束 返回上一级菜单 
    }
}

//advance->task4
static void page_task4(lcd_menu_t* menu) {
    if (menu->need_refresh) {
        if ( g_lcd_menu_port->clear)  g_lcd_menu_port->clear(C_BACK);
        g_lcd_menu_port->show_string(0, 0, (unsigned char*)"TASK 4", C_TEXT,C_BACK);
        menu->need_refresh = false;
    }

    int key = get_input(menu);
    if (key == KEY_UP) { // Up

    } else if (key == KEY_DOWN) { // Down

    } else if (key == KEY_BACK) { // Back
         
    } else if (key == KEY_ENTER) { // Enter
        if ( g_lcd_menu_port->clear)  g_lcd_menu_port->clear(C_BACK);
        g_lcd_menu_port->show_string(20, 60, (u8*)"D", C_TEXT,C_BACK);
        g_lcd_menu_port->delay_ms(1000);
        //menu->is_running = false;
        menu->current_page = PAGE_ADVANCE;
        menu->cursor_pos = 0;
        menu->need_refresh = true;
        //函数执行结束 返回上一级菜单 
    }
}

// ---------------------------------------------------------
// 主循环
// ---------------------------------------------------------
void lcd_menu_loop(lcd_menu_t* menu) {
    if (!menu->is_running) return;

    switch (menu->current_page) {
        case PAGE_MAIN:    page_main(menu); break;
        case PAGE_BASIC:   page_basic(menu); break;
        case PAGE_ADVANCE: page_advance(menu); break; 
        case PAGE_TASK1:   page_task1(menu); break;
        case PAGE_TASK2:   page_task2(menu); break;
        case PAGE_TASK3:   page_task3(menu); break;
        case PAGE_TASK4:   page_task4(menu); break;
        default:           menu->current_page = PAGE_MAIN; break;
    }

    if ( g_lcd_menu_port &&  g_lcd_menu_port->delay_ms) {
         g_lcd_menu_port->delay_ms(5); // 简单的帧率控制
    }
}
