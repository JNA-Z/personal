#include "soft_timer_port.h"
#include "../util/soft_timer.h"
#include "main.h"
#include "../util/datatype.h"

// DWT初始化
void dwt_init(void)
{
    
  CoreDebug->DEMCR |= 1 << 24 ;    //使能DWT外设
  DWT->CYCCNT       = 0;           //清零CYCCNT
  DWT->CTRL        |= 1 << 0;     //使能计数

}

// 获取当前DWT计数值
u32 dwt_get_count(void)
{
    return DWT->CYCCNT;
}

// 计算计数差值  只是用来计算  不能超过最大测量时间 59秒
u32 dwt_get_count_dx(uint32_t start_tick, uint32_t stop_tick)
{
	uint32_t tick;
	if(stop_tick < start_tick) {
		tick = (0xffffffff - start_tick) + stop_tick + 1;
	}
	else {
		tick = stop_tick - start_tick;
	}
	
	return tick;
}

/**
 * @brief 获取当前毫秒内的微秒偏移量 (0 ~ 999)
 * @note 利用SysTick的寄存器值获取当前微秒数 
 * 自动适配系统时钟频率 (通过读取 SysTick->LOAD)
 */
static inline u32 get_current_us(void)
{
    u32 us;

    // 1. 读取当前计数值和重装载值
    u32 val = SysTick->VAL;
    u32 load = SysTick->LOAD;

    // 2. 处理计数器回滚 (关键步骤)
    // 如果 COUNTFLAG 为 1，说明刚才 VAL 刚好从 0 减到溢出，硬件自动重载了 LOAD
    // 此时读到的 val 其实是新周期的 LOAD (值很大)
    // 如果不处理，算出来会是 0，导致时间戳看起来“倒退”了 1ms (比如从 1000.999 变回 1001.000)
    if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)
    {
        us = 0;
    }
    else
    {
        // 3. 计算逻辑
        // SysTick->LOAD + 1  = 1ms 包含的总 tick 数 (记为 Ticks_Per_1ms)
        // (LOAD - val)       = 当前周期内过去的 tick 数
        // 
        // 微秒数 = 过去的 ticks / (Ticks_Per_1ms / 1000)
        //       = (LOAD - val) * 1000 / (LOAD + 1)
        //
        // 这样写的好处：
        // 1. 不需要 SystemCoreClock 变量，完全依赖硬件当前运行状态
        // 2. 自动适配任何频率 (48M, 72M, 168M, 480M...)
        // 3. 纯整数运算，精度最高
        
        us = ((load - val) * 1000) / (load + 1);
    }

    return us;
}

static inline u32 get_current_ms(void)
{
    return HAL_GetTick();
}

void time_init(void)
{
    time_set_ms_provider(get_current_ms);
    time_set_us_provider(get_current_us);
}

