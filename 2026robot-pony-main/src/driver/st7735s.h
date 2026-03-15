#ifndef __ST7735S_H
#define __ST7735S_H

#include "../util/datatype.h"

#define WHITE	    0xFFFF
#define BLACK	    0x0000	  
#define BLUE	    0x001F  
#define BRED	    0XF81F
#define GRED	    0XFFE0
#define GBLUE	    0X07FF
#define RED		    0xF800
#define MAGENTA	    0xF81F
#define GREEN	    0x07E0
#define CYAN	    0x7FFF
#define YELLOW	    0xFFE0
#define BROWN	    0XBC40 
#define BRRED	    0XFC07 
#define GRAY	    0X8430 
#define DARKBLUE	0X01CF	
#define LIGHTBLUE	0X7D7C	
#define GRAYBLUE	0X5458

typedef struct st7735s{
    void* bl_gpio;
    u16 bl_pin;
    void* cs_gpio;
    u16 cs_pin;
    void* dc_gpio;
    u16 dc_pin;
    void* res_gpio;
    u16 res_pin;
    void* spi;
}st7735s_t;

// port
typedef struct st7735s_port{
    void (*write_pin)(void* gpio, u16 pin, u8 value);
    void (*write_spi)(st7735s_t* lcd, u8 data);
    void (*delay_ms)(u32 ms);
}st7735s_port_t;

// 绑定port
void st7735s_bind_port(const st7735s_port_t *port);

// api

void st7735s_init(st7735s_t* lcd);
void st7735s_display_on(void);
void st7735s_display_off(void);
void st7735s_clear(u16 color);
void st7735s_draw_point(u16 x, u16 y, u16 color);
void st7735s_fill(u16 sx, u16 sy, u16 ex, u16 ey, u16 color);
void st7735s_draw_line(u16 x1, u16 y1, u16 x2, u16 y2, u16 color);
void st7735s_draw_rectangle(u16 x1, u16 y1, u16 x2, u16 y2, u16 color);
void st7735s_draw_circle(u16 x0, u16 y0, u8 r, u16 color);
void st7735s_draw_full_circle(u16 Xpos, u16 Ypos, u16 Radius, u16 Color);
void st7735s_draw_char(u16 x, u16 y, u8 num, u8 size,
                       u8 mode, u16 pen_color, u16 back_color);
void st7735s_draw_string(u16 x, u16 y, u16 width,
                         u16 height, u8 size, u8* p,
                         u16 pen_color, u16 back_color);


#endif
