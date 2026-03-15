#include "lcd_port.h"
#include "../driver/st7735s.h"
#include "main.h"

static st7735s_port_t g_lcd_port = {0};

static void lcd_write_pin(void* gpio, u16 pin, u8 value)
{
    // HAL_GPIO_WritePin((GPIO_TypeDef*)gpio, pin, value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void lcd_write_spi(st7735s_t* lcd, u8 data)
{
    // HAL_SPI_Transmit((SPI_HandleTypeDef*)lcd->spi, &data, 1, HAL_MAX_DELAY);
}

static void delay_ms(u32 ms)
{
    HAL_Delay(ms);
}

void lcd_port_init(void)
{
    g_lcd_port.write_pin = lcd_write_pin;
    g_lcd_port.write_spi = lcd_write_spi;
    g_lcd_port.delay_ms = delay_ms;
    st7735s_bind_port(&g_lcd_port);
}
