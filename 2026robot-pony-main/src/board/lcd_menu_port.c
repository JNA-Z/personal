#include "lcd_menu_port.h"
#include "../driver/st7735s.h"
#include "../driver/key.h"
#include "main.h"

//width和height参数暂时写为屏幕分辨率128x160，字体大小16 st77335s
#define wideth 128
#define height 160
#define FONT_SIZE 16

static lcd_menu_port_t g_lcd_menu_port = {0};

static void lcd_menu_showstring(u16 x, u16 y, u8* p, u16 pen_color, u16 back_color)
{
    //调用lcd显示字符函数
    st7735s_draw_string(x, y, wideth, height, FONT_SIZE, p, pen_color, back_color);
}

static void lcd_menu_clear(u16 color)
{
    //调用lcd清屏函数
    st7735s_clear(color);
}

static void key_scan(key_t *keys, usize keys_size)
{
    //调用按键扫描函数
    key_scan_all(keys, keys_size);
}

static void delay_ms(u32 ms)
{
    HAL_Delay(ms);
}

void lcd_menu_port_init(void)
{
    g_lcd_menu_port.show_string = lcd_menu_showstring;
    g_lcd_menu_port.clear = lcd_menu_clear; 
    g_lcd_menu_port.key_scan = key_scan;
    g_lcd_menu_port.delay_ms = delay_ms;
    lcd_menu_bind_port(&g_lcd_menu_port);
}
