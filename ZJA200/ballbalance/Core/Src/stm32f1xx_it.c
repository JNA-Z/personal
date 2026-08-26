/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f1xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* ---- 故障现场保存 (HardFault 触发后, 可在调试器 Watch/内存窗口查看) ---- */
volatile uint32_t g_fault_r0;    /* 异常栈帧 r0  */
volatile uint32_t g_fault_r1;    /* 异常栈帧 r1  */
volatile uint32_t g_fault_r2;    /* 异常栈帧 r2  */
volatile uint32_t g_fault_r3;    /* 异常栈帧 r3  */
volatile uint32_t g_fault_r12;   /* 异常栈帧 r12 */
volatile uint32_t g_fault_lr;    /* 异常栈帧 LR  (返回地址) */
volatile uint32_t g_fault_pc;    /* 异常栈帧 PC  (故障指令地址) */
volatile uint32_t g_fault_psr;   /* 异常栈帧 xPSR */
volatile uint32_t g_fault_sp;    /* 故障时 MSP 栈指针 */
volatile uint32_t g_fault_cfsr;  /* SCB->CFSR: MMFSR|BFSR|UFSR 汇总 */
volatile uint32_t g_fault_bfar;  /* SCB->BFAR: 总线故障地址 (BFARVALID=1 时有效) */
volatile uint32_t g_fault_mmar;  /* SCB->MMFAR: 内存管理故障地址 */
volatile uint32_t g_fault_hfsr;  /* SCB->HFSR: 硬故障状态寄存器 */
volatile uint32_t g_fault_isfr;  /* SCB->DFSR: 调试故障状态寄存器 */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M3 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */
  __disable_irq();   /* 关中断, 防止现场被后续异常破坏 */

  /* 读取 Cortex-M3 故障状态寄存器 */
  g_fault_cfsr = SCB->CFSR;    /* 含 MMFSR/BFSR/UFSR, 可分解定位故障类型 */
  g_fault_bfar = SCB->BFAR;    /* 总线故障地址 */
  g_fault_mmar = SCB->MMFAR;   /* 内存管理故障地址 */
  g_fault_hfsr = SCB->HFSR;    /* 硬故障状态寄存器 (FORCED 位等) */
  g_fault_isfr = SCB->DFSR;    /* 调试故障状态寄存器 */

  /* 当前栈指针: 无 RTOS 时线程模式用 MSP, 故障异常也在 MSP 上压栈 */
  g_fault_sp = __get_MSP();

  /* 异常栈帧 (入栈 8 个寄存器): [r0 r1 r2 r3 r12 LR PC xPSR] */
  const uint32_t *sp = (const uint32_t *)g_fault_sp;
  g_fault_r0  = sp[0];
  g_fault_r1  = sp[1];
  g_fault_r2  = sp[2];
  g_fault_r3  = sp[3];
  g_fault_r12 = sp[4];
  g_fault_lr  = sp[5];   /* 被中断处的返回地址 */
  g_fault_pc  = sp[6];   /* 故障指令地址 */
  g_fault_psr = sp[7];

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F1xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f1xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */

  /* USER CODE END USART1_IRQn 0 */
  HAL_UART_IRQHandler(&huart1);
  /* USER CODE BEGIN USART1_IRQn 1 */

  /* USER CODE END USART1_IRQn 1 */
}

/**
  * @brief This function handles USART3 global interrupt.
  */
void USART3_IRQHandler(void)
{
  /* USER CODE BEGIN USART3_IRQn 0 */

  /* USER CODE END USART3_IRQn 0 */
  HAL_UART_IRQHandler(&huart3);
  /* USER CODE BEGIN USART3_IRQn 1 */

  /* USER CODE END USART3_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
