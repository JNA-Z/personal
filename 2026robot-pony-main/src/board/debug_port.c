#include "debug_port.h"
#include "../driver/debug.h"
#include "../util/datatype.h"
#include "main.h"
#include <string.h>

void hw_puts_output(const char* str)
{
    // HAL_UART_Transmit(&huart1, (const uint8_t *)str, strlen(str), HAL_MAX_DELAY);
}

void debug_port_init(void)
{
    // 初始化调试端口
    debug_init(hw_puts_output);
}
