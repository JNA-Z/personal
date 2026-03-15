#include "delay.h"
#include "main.h"
#include "cmsis_os.h"

	 
// 延时函数代码，支持ucos, 采用SysTick作为时基.
#if SYSTEM_SUPPORT_OS
//#include "includes.h"					//ucos 支持	  
#endif

/**
 * @brief 非阻塞式按 RTOS tick 延时
 * @param ticks 需要延时的 tick 数
 */
void delay_tick_noblock(uint32_t ticks)
{
	osDelay(ticks);
}

/**
 * @brief 非阻塞式按毫秒延时（内部完成 ms 到 tick 的换算）
 * @param ms 需要延时的毫秒数
 */
void delay_ms_noblock(uint32_t ms)
{
	uint32_t tick_freq = osKernelGetTickFreq();
	uint32_t ticks;

	if (tick_freq == 0U) {
		ticks = ms;
	} else {
		ticks = (ms * tick_freq + 999U) / 1000U;
	}

	if (ms > 0U && ticks == 0U) {
		ticks = 1U;
	}

	delay_tick_noblock(ticks);
}
//*********************************************************************************  
// 本代码只供学习使用，未经作者许可，不得用于其它任何用途
// ALIENTEK STM32F429开发板
// 使用SysTick的普通模式实现延时功能(支持ucosii/ucosiii)
// 包含delay_us, delay_ms函数
// 作者: ALIENTEK
// 论坛: www.openedv.com
// 日期: 2015/6/10
// 版本: V1.1
// 版权所有，盗版必究。
// Copyright(C) 广州市星翼电子科技有限公司 2014-2024
// All rights reserved
//********************************************************************************
// 说明：


static uint32_t fac_us = 0;							// us延时倍乘数

#if SYSTEM_SUPPORT_OS		
static u16 fac_ms = 0;				        // ms延时倍乘数, 在OS下, 表示每个节拍的ms数
#endif

#if SYSTEM_SUPPORT_OS							// 当定义SYSTEM_SUPPORT_OS时, 表示要支持OS(例如UCOS).
// 当delay_us/delay_ms需要支持OS时, 需要对以下3个宏进行支持
// 定义以下3个宏:
// delay_osrunning: 表示OS当前是否运行, 0, 表示未运行; 1, 表示正在运行
// delay_ostickspersec: 表示OS设定的时基频率, delay_init会根据这个值来初始化SYSTICK
// delay_osintnesting: 表示OS中断嵌套级别, 因为中断中不能调度, delay_ms使用该参数判断是否可以调度

// 对UCOSII和UCOSIII的支持宏, 可根据OS类型取消注释
// 支持UCOSII
#ifdef 	OS_CRITICAL_METHOD						// OS_CRITICAL_METHOD定义, 表示要支持UCOSII				
#define delay_osrunning		OSRunning			// OS是否运行: 0, 未运行; 1, 运行中
#define delay_ostickspersec	OS_TICKS_PER_SEC	// OS时基频率, 即每秒节拍数
#define delay_osintnesting 	OSIntNesting		// 中断嵌套级别, 嵌套越深级别越高
#endif

// 支持UCOSIII
#ifdef 	CPU_CFG_CRITICAL_METHOD					// CPU_CFG_CRITICAL_METHOD定义, 表示要支持UCOSIII	
#define delay_osrunning		OSRunning			// OS是否运行: 0, 未运行; 1, 运行中
#define delay_ostickspersec	OSCfg_TickRate_Hz	// OS时基频率, 即每秒节拍数
#define delay_osintnesting 	OSIntNestingCtr		// 中断嵌套级别, 嵌套越深数值越大
#endif


// us延时时, 锁定调度器(防止打断SysTick计时)
void delay_osschedlock(void)
{
#ifdef CPU_CFG_CRITICAL_METHOD   			// 使用UCOSIII
	OS_ERR err; 
	OSSchedLock(&err);						// UCOSIII方式, 锁定调度器, 防止任务调度
#else										// 否则UCOSII
	OSSchedLock();							// UCOSII方式, 锁定调度器
#endif
}

// us延时时, 解锁调度器
void delay_osschedunlock(void)
{	
#ifdef CPU_CFG_CRITICAL_METHOD   			// 使用UCOSIII
	OS_ERR err; 
	OSSchedUnlock(&err);					// UCOSIII方式, 解锁调度器
#else										// 否则UCOSII
	OSSchedUnlock();						// UCOSII方式, 解锁调度器
#endif
}

// 调用OS的延时函数
// ticks: 延时的节拍数
void delay_ostimedly(u32 ticks)
{
#ifdef CPU_CFG_CRITICAL_METHOD
	OS_ERR err; 
	OSTimeDly(ticks, OS_OPT_TIME_PERIODIC, &err); // UCOSIII延时函数(周期模式)
#else
	OSTimeDly(ticks);						    // UCOSII延时函数
#endif 
}
 
// SysTick中断服务函数, OS启用时调用
void SysTick_Handler(void)
{	
    HAL_IncTick();
	if (delay_osrunning == 1)					// OS已启动, 执行OS的时基处理
	{
		OSIntEnter();						// 进入中断
		OSTimeTick();       				// 触发UCOS时基更新              
		OSIntExit();       	 				// 退出中断
	}
}
#endif
			   
// 初始化延时函数
// 当使用ucos时, 必须设置ucos的时基频率
// SYSTICK时钟源为AHB时钟
// SYSCLK: 系统时钟频率(单位MHz)
void delay_init(uint8_t SYSCLK)
{
#if SYSTEM_SUPPORT_OS 						// 需要支持OS
	u32 reload;
#endif
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK); // SysTick时钟源设为HCLK
	fac_us = SYSCLK;						// 初始化us延时倍乘数（不使用OS时也需用到）
#if SYSTEM_SUPPORT_OS 						// 需要支持OS
	reload = SYSCLK;					    // 每节拍的计数值（单位为千次）	   
	reload *= 1000000 / delay_ostickspersec;	// 根据OS时基频率计算自动重装载值
											// reload为24位寄存器, 最大值16777216, 若系统时钟180MHz, 则最大延时约0.745s
	fac_ms = 1000 / delay_ostickspersec;		// 计算OS节拍对应的ms数	   
	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk; // 使能SysTick中断
	SysTick->LOAD = reload; 					// 设定自动重装载值（每1/OS_TICKS_PER_SEC秒中断一次）	
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; // 使能SysTick
#else
#endif
}								    

#if SYSTEM_SUPPORT_OS 						// 需要支持OS
// 延时nus
// nus: 要延时的us数	
// nus范围: 0~190887435（受限于2^32/fac_us，当fac_us=22.5时）	    								   
void delay_us(u32 nus)
{		
	u32 ticks;
	u32 told, tnow, tcnt = 0;
	u32 reload = SysTick->LOAD;				// 获取自动重装载值	    	 
	ticks = nus * fac_us; 						// 计算需要的总节拍数 
	delay_osschedlock();					// 锁定调度器，防止延时被任务切换打断
	told = SysTick->VAL;        				// 记录当前计数值
	while (1)
	{
		tnow = SysTick->VAL;	
		if (tnow != told)
		{	    
			if (tnow < told) tcnt += told - tnow;	// 正常计数递减（当计数值减小时）
			else tcnt += reload - tnow + told;		// 计数值归零后翻转（环形计数器特性）  
			told = tnow;
			if (tcnt >= ticks) break;			// 时间达到/超过目标延时，退出循环
		}  
	};
	delay_osschedunlock();					// 解锁调度器											    
}  
// 延时nms
// nms: 要延时的ms数
// nms范围: 0~65535
void delay_ms(u16 nms)
{	
	if (delay_osrunning && delay_osintnesting == 0) // OS正在运行且不在中断中
	{		 
		if (nms >= fac_ms)						// 延时时间超过OS最小调度单位
		{ 
   			delay_ostimedly(nms / fac_ms);	// 调用OS延时函数
		}
		nms %= fac_ms;						// 剩余时间用普通方式延时    
	}
	delay_us((u32)(nms * 1000));				// 普通方式延时（非OS调度）
}
#else  // 不使用ucos时

// 延时nus
// nus为要延时的us数	
// nus范围: 0~190887435（受限于2^32/fac_us，当fac_us=22.5时）	 
void delay_us(uint32_t nus)
{		
	uint32_t ticks;
	uint32_t told, tnow, tcnt = 0;
	uint32_t reload = SysTick->LOAD;				// 获取自动重装载值	    	 
	ticks = nus * fac_us; 						// 计算需要的总节拍数 
	told = SysTick->VAL;        				// 记录当前计数值
	while (1)
	{
		tnow = SysTick->VAL;	
		if (tnow != told)
		{	    
			if (tnow < told) tcnt += told - tnow;	// 正常计数递减（当计数值减小时）
			else tcnt += reload - tnow + told;		// 计数值归零后翻转（环形计数器特性）  
			told = tnow;
			if (tcnt >= ticks) break;			// 时间达到/超过目标延时，退出循环
		}  
	};
}

// 延时nms
// nms: 要延时的ms数
void delay_ms(uint16_t nms)
{
	uint32_t i;
	for (i = 0; i < nms; i++) delay_us(1000);
}
#endif
