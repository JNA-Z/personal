#include "st7735s.h"
#include "font.h"

static st7735s_port_t* g_st7735s_port = NULL;
st7735s_t* g_st7735s = NULL;

void st7735s_bind_port(const st7735s_port_t* port)
{
    g_st7735s_port = (st7735s_port_t*)port;
}

// defines for ease of subsequent writing set 数据 clr 命令
#define SET_LCD_BL  g_st7735s_port->write_pin(g_st7735s->bl_gpio, g_st7735s->bl_pin, 1)
#define SET_LCD_CS  g_st7735s_port->write_pin(g_st7735s->cs_gpio, g_st7735s->cs_pin, 1)
#define SET_LCD_DC  g_st7735s_port->write_pin(g_st7735s->dc_gpio, g_st7735s->dc_pin, 1)
#define SET_LCD_RES g_st7735s_port->write_pin(g_st7735s->res_gpio, g_st7735s->res_pin, 1)

#define CLR_LCD_BL  g_st7735s_port->write_pin(g_st7735s->bl_gpio, g_st7735s->bl_pin, 0)
#define CLR_LCD_CS  g_st7735s_port->write_pin(g_st7735s->cs_gpio, g_st7735s->cs_pin, 0)
#define CLR_LCD_DC  g_st7735s_port->write_pin(g_st7735s->dc_gpio, g_st7735s->dc_pin, 0)
#define CLR_LCD_RES g_st7735s_port->write_pin(g_st7735s->res_gpio, g_st7735s->res_pin, 0)

#define delay_ms(ms) g_st7735s_port->delay_ms(ms)

void st7735s_select_register(u8 data);
void st7735s_write_u8(u8 data);

/**
 * @brief LCD初始化
 * 
 * @param lcd LCD结构体指针
 */
void st7735s_init(st7735s_t* lcd)
{
    g_st7735s = lcd;

    SET_LCD_RES;
    delay_ms(10);
    CLR_LCD_RES;
    delay_ms(10);
    SET_LCD_RES;
    delay_ms(200);

    st7735s_select_register(0x11);   // Sleep out
    delay_ms(120);

    st7735s_select_register(0xB1);
    st7735s_write_u8(0x05);
    st7735s_write_u8(0x3C);
    st7735s_write_u8(0x3C);

    st7735s_select_register(0xB2);
    st7735s_write_u8(0x05);
    st7735s_write_u8(0x3C);
    st7735s_write_u8(0x3C);

    st7735s_select_register(0xB3);
    st7735s_write_u8(0x05);
    st7735s_write_u8(0x3C);
    st7735s_write_u8(0x3C);
    st7735s_write_u8(0x05);
    st7735s_write_u8(0x3C);
    st7735s_write_u8(0x3C);

    st7735s_select_register(0xB4);
    st7735s_write_u8(0x03);

    st7735s_select_register(0xC0);
    st7735s_write_u8(0x28);
    st7735s_write_u8(0x08);
    st7735s_write_u8(0x04);

    st7735s_select_register(0xC1);
    st7735s_write_u8(0XC0);

    st7735s_select_register(0xC2);
    st7735s_write_u8(0x0D);
    st7735s_write_u8(0x00);

    st7735s_select_register(0xC3);
    st7735s_write_u8(0x8D);
    st7735s_write_u8(0x2A);

    st7735s_select_register(0xC4);
    st7735s_write_u8(0x8D);
    st7735s_write_u8(0xEE);

    st7735s_select_register(0xC5);
    st7735s_write_u8(0x1A);

    st7735s_select_register(0x36);
    st7735s_write_u8(0xC0);

    st7735s_select_register(0xE0);
    st7735s_write_u8(0x04);
    st7735s_write_u8(0x22);
    st7735s_write_u8(0x07);
    st7735s_write_u8(0x0A);
    st7735s_write_u8(0x2E);
    st7735s_write_u8(0x30);
    st7735s_write_u8(0x25);
    st7735s_write_u8(0x2A);
    st7735s_write_u8(0x28);
    st7735s_write_u8(0x26);
    st7735s_write_u8(0x2E);
    st7735s_write_u8(0x3A);
    st7735s_write_u8(0x00);
    st7735s_write_u8(0x01);
    st7735s_write_u8(0x03);
    st7735s_write_u8(0x13);

    st7735s_select_register(0xE1);
    st7735s_write_u8(0x04);
    st7735s_write_u8(0x16);
    st7735s_write_u8(0x06);
    st7735s_write_u8(0x0D);
    st7735s_write_u8(0x2D);
    st7735s_write_u8(0x26);
    st7735s_write_u8(0x23);
    st7735s_write_u8(0x27);
    st7735s_write_u8(0x27);
    st7735s_write_u8(0x25);
    st7735s_write_u8(0x2D);
    st7735s_write_u8(0x3B);
    st7735s_write_u8(0x00);
    st7735s_write_u8(0x01);
    st7735s_write_u8(0x04);
    st7735s_write_u8(0x13);

    st7735s_select_register(0x3A);
    st7735s_write_u8(0x05);

    st7735s_select_register(0x29);
}

void st7735s_select_register(u8 data)
{
    CLR_LCD_DC;
    CLR_LCD_CS;
    g_st7735s_port->write_spi(g_st7735s, data);
    SET_LCD_CS;
}

void st7735s_write_u8(u8 data)
{
    SET_LCD_DC;
    CLR_LCD_CS;
    g_st7735s_port->write_spi(g_st7735s, data);
    SET_LCD_CS;
}

void st7735s_write_u16(u16 data)
{
    st7735s_write_u8(data >> 8);
    st7735s_write_u8(data);
}

void st7735s_set_address(u16 x1, u16 y1, u16 x2, u16 y2)
{
    st7735s_select_register(0x2A);
    st7735s_write_u8(x1 >> 8);
    st7735s_write_u8(x1 & 0xFF);
    st7735s_write_u8(x2 >> 8);
    st7735s_write_u8(x2 & 0xFF);
    st7735s_select_register(0x2B);
    st7735s_write_u8(y1 >> 8);
    st7735s_write_u8(y1 & 0xFF);
    st7735s_write_u8(y2 >> 8);
    st7735s_write_u8(y2 & 0xFF);

    st7735s_select_register(0x2C);
}

/**
 * @brief LCD开启显示
 * 
 */
void st7735s_display_on(void)
{
    st7735s_select_register(0x29);  
}

/**
 * @brief LCD关闭显示
 * 
 */
void st7735s_display_off(void)
{
    st7735s_select_register(0x28);   
}

/**
 * @brief LCD清屏
 * 
 * @param color 清屏颜色
 */
void st7735s_clear(u16 color)
{
    st7735s_set_address(0, 0, 130, 162);

    for (u16 i = 0; i < 130; i++) {
        for (u16 j = 0; j < 162; j++) {
            st7735s_write_u16(color);
        }
    }
}

/**
 * @brief LCD画点
 * 
 * @param x 点的横坐标
 * @param y 点的纵坐标
 * @param color 点的颜色
 */
void st7735s_draw_point(u16 x, u16 y, u16 color)
{
    st7735s_set_address(x, y, x, y);   // 设置光标位置
    st7735s_write_u16(color);
}

/**
 * @brief LCD区域填充
 * 
 * @param sx 起点横坐标
 * @param sy 起点纵坐标
 * @param ex 终点横坐标
 * @param ey 终点纵坐标
 * @param color 填充颜色
 */
void st7735s_fill(u16 sx, u16 sy, u16 ex, u16 ey, u16 color)
{
    u16 i, j;
    u16 xlen = 0;
    u16 ylen = 0;

    xlen = ex - sx + 1;
    ylen = ey - sy + 1;

    st7735s_set_address(sx, sy, ex, ey);
    for (i = 0; i < xlen; i++) {
        for (j = 0; j < ylen; j++) {
            st7735s_write_u16(color);
        }
    }
}

/**
 * @brief LCD画线
 * 
 * @param x1 起点坐标横坐标
 * @param y1 起点坐标纵坐标
 * @param x2 终点坐标横坐标
 * @param y2 终点坐标纵坐标
 * @param color 线条颜色
 */
void st7735s_draw_line(u16 x1, u16 y1, u16 x2, u16 y2,
                       u16 color)
{
    u16 t;
    int            xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int            incx, incy, uRow, uCol;
    delta_x = x2 - x1;   // 计算坐标增量
    delta_y = y2 - y1;
    uRow    = x1;
    uCol    = y1;

    // 设置单步方向
    if (delta_x > 0) {
        incx = 1;
    }
    else if (delta_x == 0)   // 垂直线
    {
        incx = 0;
    }
    else {
        incx    = -1;
        delta_x = -delta_x;
    }


    if (delta_y > 0) {
        incy = 1;
    }
    else if (delta_y == 0)   // 水平线
    {
        incy = 0;
    }
    else {
        incy    = -1;
        delta_y = -delta_y;
    }

    if (delta_x > delta_y)   // 选取基本增量坐标轴
    {
        distance = delta_x;
    }
    else {
        distance = delta_y;
    }

    for (t = 0; t <= distance + 1; t++)   // 画线输出
    {
        st7735s_draw_point(uRow, uCol, color);   // 画点
        xerr += delta_x;
        yerr += delta_y;
        if (xerr > distance) {
            xerr -= distance;
            uRow += incx;
        }

        if (yerr > distance) {
            yerr -= distance;
            uCol += incy;
        }
    }
}

/**
 * @brief LCD画矩形
 * 
 * @param x1 起点坐标横坐标
 * @param y1 起点坐标纵坐标
 * @param x2 终点坐标横坐标
 * @param y2 终点坐标纵坐标
 * @param color 线条颜色
 */
void st7735s_draw_rectangle(u16 x1, u16 y1, u16 x2,
                            u16 y2, u16 color)
{
    st7735s_draw_line(x1, y1, x2, y1, color);
    st7735s_draw_line(x1, y1, x1, y2, color);
    st7735s_draw_line(x1, y2, x2, y2, color);
    st7735s_draw_line(x2, y1, x2, y2, color);
}

/**
 * @brief LCD画圆
 * 
 * @param x0 圆心横坐标
 * @param y0 圆心纵坐标
 * @param r 半径
 * @param color 颜色
 */
void st7735s_draw_circle(u16 x0, u16 y0, u8 r,
                         u16 color)
{
    int a, b;
    int di;
    a  = 0;
    b  = r;
    di = 3 - (r << 1);   // 判断下个点位置的标志
    while (a <= b) {
        st7735s_draw_point(x0 + a, y0 - b, color);
        st7735s_draw_point(x0 + b, y0 - a, color);
        st7735s_draw_point(x0 + b, y0 + a, color);
        st7735s_draw_point(x0 + a, y0 + b, color);
        st7735s_draw_point(x0 - a, y0 + b, color);
        st7735s_draw_point(x0 - b, y0 + a, color);
        st7735s_draw_point(x0 - a, y0 - b, color);
        st7735s_draw_point(x0 - b, y0 - a, color);
        a++;
        // 使用Bresenham算法画圆
        if (di < 0) {
            di += 4 * a + 6;
        }
        else {
            di += 10 + 4 * (a - b);
            b--;
        }
    }
}

/**
 * @brief LCD画圆面
 * 
 * @param Xpos 圆心横坐标
 * @param Ypos 圆心纵坐标
 * @param Radius 半径
 * @param Color 颜色
 */
void st7735s_draw_full_circle(u16 Xpos, u16 Ypos, u16 Radius,
                              u16 Color)
{
    uint16_t x, y, r = Radius;
    for (y = Ypos - r; y < Ypos + r; y++) {
        for (x = Xpos - r; x < Xpos + r; x++) {
            if (((x - Xpos) * (x - Xpos) + (y - Ypos) * (y - Ypos)) <= r * r) {
                st7735s_draw_point(x, y, Color);
            }
        }
    }
}

/**
 * @brief LCD字符显示
 * 
 * @param x 起始横坐标
 * @param y 起始纵坐标
 * @param num 要显示的字符
 * @param size 字体大小 12/16/24
 * @param mode 显示模式（1：叠加 0：非叠加）
 * @param pen_color 字体颜色
 * @param back_color 背景颜色
 */
void st7735s_draw_char(u16 x, u16 y, u8 num, u8 size,
                       u8 mode, u16 pen_color, u16 back_color)
{
    u8  temp, t1, t;
    u16 y0 = y;
    u8  csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2);   // 得到字体一个字符对应点阵集所占的字节数
    num = num - ' ';   // 得到偏移后的值（ASCII字库是从空格开始取模，所以-' '就是对应字符的字库）

    for (t = 0; t < csize; t++) {
        if (size == 12)   // 调用1206字体
        {
            temp = asc2_1206[num][t];
        }
        else if (size == 16)   // 调用1608字体
        {
            temp = asc2_1608[num][t];
        }
        else if (size == 24)   // 调用2412字体
        {
            temp = asc2_2412[num][t];
        }
        else
            return;   // 没有的字库

        for (t1 = 0; t1 < 8; t1++) {
            if (temp & 0x80) {
                st7735s_draw_point(x, y, pen_color);
            }
            else if (mode == 0) {
                st7735s_draw_point(x, y, back_color);
            }
            temp <<= 1;
            y++;

            if (y >= 240)   // 超区域
            {
                return;
            }

            if ((y - y0) == size) {
                y = y0;
                x++;
                if (x >= 320)   // 超区域
                {
                    return;
                }
                break;
            }
        }
    }
}

/**
 * @brief LCD字符串显示
 * 
 * @param x 起点横坐标
 * @param y 起点纵坐标
 * @param width 显示区域宽度
 * @param height 显示区域高度
 * @param size 字体大小
 * @param p 字符串起始地址
 * @param pen_color 字体颜色
 * @param back_color 背景颜色
 */
void st7735s_draw_string(u16 x, u16 y, u16 width,
                         u16 height, u8 size, u8* p,
                         u16 pen_color, u16 back_color)
{
    u16 x0 = x;
    width += x;
    height += y;
    while ((*p <= '~') && (*p >= ' '))   // 判断是不是非法字符!
    {
        if (x >= width) {
            x = x0;
            y += size;
        }

        if (y >= height)   // 退出
        {
            break;
        }

        st7735s_draw_char(x, y, *p, size, 0, pen_color, back_color);
        x += size / 2;
        p++;
    }
}
