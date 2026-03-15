/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "tasks.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for LCD_Show */
osThreadId_t LCD_ShowHandle;
const osThreadAttr_t LCD_Show_attributes = {
  .name = "LCD_Show",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal2,
};
/* Definitions for Robot_Run */
osThreadId_t Robot_RunHandle;
const osThreadAttr_t Robot_Run_attributes = {
  .name = "Robot_Run",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for Hardware_Handle */
osThreadId_t Hardware_HandleHandle;
const osThreadAttr_t Hardware_Handle_attributes = {
  .name = "Hardware_Handle",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal7,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartLCD_Show(void *argument);
void StartRobot_Run(void *argument);
void StartHardware_Handle(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of LCD_Show */
  LCD_ShowHandle = osThreadNew(StartLCD_Show, NULL, &LCD_Show_attributes);

  /* creation of Robot_Run */
  Robot_RunHandle = osThreadNew(StartRobot_Run, NULL, &Robot_Run_attributes);

  /* creation of Hardware_Handle */
  Hardware_HandleHandle = osThreadNew(StartHardware_Handle, NULL, &Hardware_Handle_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  app_task_default(argument);
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartLCD_Show */
/**
* @brief Function implementing the LCD_Show thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLCD_Show */
void StartLCD_Show(void *argument)
{
  /* USER CODE BEGIN StartLCD_Show */
  app_task_lcd_show(argument);
  /* USER CODE END StartLCD_Show */
}

/* USER CODE BEGIN Header_StartRobot_Run */
/**
* @brief Function implementing the Robot_Run thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartRobot_Run */
void StartRobot_Run(void *argument)
{
  /* USER CODE BEGIN StartRobot_Run */
  app_task_robot_run(argument);
  /* USER CODE END StartRobot_Run */
}

/* USER CODE BEGIN Header_StartHardware_Handle */
/**
* @brief Function implementing the Hardware_Handle thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartHardware_Handle */
void StartHardware_Handle(void *argument)
{
  /* USER CODE BEGIN StartHardware_Handle */
  app_task_hardware_handle(argument);
  /* USER CODE END StartHardware_Handle */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

