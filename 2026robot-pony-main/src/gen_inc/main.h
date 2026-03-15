/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define Debug 0    //1:调试模式  2；普通模式
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Infrared_up_Pin GPIO_PIN_4
#define Infrared_up_GPIO_Port GPIOE
#define colorcode_front_left_Pin GPIO_PIN_8
#define colorcode_front_left_GPIO_Port GPIOF
#define colorcode6_Pin GPIO_PIN_9
#define colorcode6_GPIO_Port GPIOF
#define Grayscale_IN10_Pin GPIO_PIN_0
#define Grayscale_IN10_GPIO_Port GPIOA
#define Grayscale_IN2_Pin GPIO_PIN_1
#define Grayscale_IN2_GPIO_Port GPIOA
#define Grayscale_IN1_Pin GPIO_PIN_2
#define Grayscale_IN1_GPIO_Port GPIOA
#define colorcode_left_Pin GPIO_PIN_2
#define colorcode_left_GPIO_Port GPIOH
#define colorcode1_Pin GPIO_PIN_3
#define colorcode1_GPIO_Port GPIOH
#define KEY_START_Pin GPIO_PIN_4
#define KEY_START_GPIO_Port GPIOH
#define Grayscale_IN3_Pin GPIO_PIN_3
#define Grayscale_IN3_GPIO_Port GPIOA
#define Grayscale_IN4_Pin GPIO_PIN_4
#define Grayscale_IN4_GPIO_Port GPIOA
#define Grayscale_IN5_Pin GPIO_PIN_5
#define Grayscale_IN5_GPIO_Port GPIOA
#define Grayscale_IN6_Pin GPIO_PIN_6
#define Grayscale_IN6_GPIO_Port GPIOA
#define Grayscale_IN7_Pin GPIO_PIN_7
#define Grayscale_IN7_GPIO_Port GPIOA
#define Grayscale_IN8_Pin GPIO_PIN_0
#define Grayscale_IN8_GPIO_Port GPIOB
#define Grayscale_IN9_Pin GPIO_PIN_1
#define Grayscale_IN9_GPIO_Port GPIOB
#define colorcode8_Pin GPIO_PIN_7
#define colorcode8_GPIO_Port GPIOE
#define colorcode_front_right_Pin GPIO_PIN_8
#define colorcode_front_right_GPIO_Port GPIOE
#define KEY4_Pin GPIO_PIN_9
#define KEY4_GPIO_Port GPIOE
#define LCD_RES_Pin GPIO_PIN_10
#define LCD_RES_GPIO_Port GPIOE
#define LCD_DC_Pin GPIO_PIN_11
#define LCD_DC_GPIO_Port GPIOE
#define LCD_CS_Pin GPIO_PIN_12
#define LCD_CS_GPIO_Port GPIOE
#define LCD_BL_Pin GPIO_PIN_13
#define LCD_BL_GPIO_Port GPIOE
#define colorcode4_Pin GPIO_PIN_14
#define colorcode4_GPIO_Port GPIOE
#define colorcode_right_Pin GPIO_PIN_15
#define colorcode_right_GPIO_Port GPIOE
#define motor3_Pin GPIO_PIN_10
#define motor3_GPIO_Port GPIOB
#define motor4_Pin GPIO_PIN_11
#define motor4_GPIO_Port GPIOB
#define Servo4_Pin GPIO_PIN_10
#define Servo4_GPIO_Port GPIOH
#define Servo3_Pin GPIO_PIN_11
#define Servo3_GPIO_Port GPIOH
#define Servo1_Pin GPIO_PIN_12
#define Servo1_GPIO_Port GPIOH
#define Encoder_Left_B_Pin GPIO_PIN_12
#define Encoder_Left_B_GPIO_Port GPIOD
#define Encoder_Left_A_Pin GPIO_PIN_13
#define Encoder_Left_A_GPIO_Port GPIOD
#define LED1_Pin GPIO_PIN_14
#define LED1_GPIO_Port GPIOD
#define LED2_Pin GPIO_PIN_15
#define LED2_GPIO_Port GPIOD
#define Encoder_Right_A_Pin GPIO_PIN_6
#define Encoder_Right_A_GPIO_Port GPIOC
#define Encoder_Right_B_Pin GPIO_PIN_7
#define Encoder_Right_B_GPIO_Port GPIOC
#define WT901C_TX_Pin GPIO_PIN_9
#define WT901C_TX_GPIO_Port GPIOA
#define WT901C_RX_Pin GPIO_PIN_10
#define WT901C_RX_GPIO_Port GPIOA
#define Servo2_Pin GPIO_PIN_0
#define Servo2_GPIO_Port GPIOI
#define motor2_Pin GPIO_PIN_15
#define motor2_GPIO_Port GPIOA
#define KEY3_Pin GPIO_PIN_0
#define KEY3_GPIO_Port GPIOD
#define Infrared_Hit_Pin GPIO_PIN_1
#define Infrared_Hit_GPIO_Port GPIOD
#define V831_TX_Pin GPIO_PIN_5
#define V831_TX_GPIO_Port GPIOD
#define V831_RX_Pin GPIO_PIN_6
#define V831_RX_GPIO_Port GPIOD
#define LCD_SDA_Pin GPIO_PIN_7
#define LCD_SDA_GPIO_Port GPIOD
#define LCD_SCL_Pin GPIO_PIN_11
#define LCD_SCL_GPIO_Port GPIOG
#define motor1_Pin GPIO_PIN_3
#define motor1_GPIO_Port GPIOB
#define KEY1_Pin GPIO_PIN_4
#define KEY1_GPIO_Port GPIOB
#define Infrared_down_left_Pin GPIO_PIN_6
#define Infrared_down_left_GPIO_Port GPIOB
#define KEY2_Pin GPIO_PIN_7
#define KEY2_GPIO_Port GPIOB
#define Infrared_down_right_Pin GPIO_PIN_8
#define Infrared_down_right_GPIO_Port GPIOB
#define Infrared4_Pin GPIO_PIN_0
#define Infrared4_GPIO_Port GPIOE
#define Infrared5_Pin GPIO_PIN_1
#define Infrared5_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/*编码器相关宏定义*/
#define ENCODER_MODE								TIM_ENCODERMODE_TI12//TI12都计数，是四倍频模式
#define ENCODER_RESOLUTION							13					//霍尔编码器线数为13
#if	(ENCODER_MODE==TIM_ENCODERMODE_TI12)
#define	ENCODER_READ_RESOLUTION					(ENCODER_RESOLUTION*4)//本例采用四倍频模式
#else
#define	ENCODER_READ_RESOLUTION					(ENCODER_RESOLUTION*2)//二倍频模式
#endif
/*减速电机减速比 */
#define REDUCTION_RATIO              				35

#define red             1
#define green           2      
#define blue            3
#define Servo_Keep      1000               //让舵机保持原角度
#define location_left   1                  //目标在左边
#define location_mid    2                  //目标在中间
#define location_right  3                  //目标在右边
#define Have            0                  
#define No              1                  
#define Online          1                  
#define Offline         0                  
#define QR_357          1                  //二维码为357
#define QR_358          2                  //二维码为358
#define QR_367          3                  //二维码为367
#define QR_368          4                  //二维码为368
#define QR_457          5                  //二维码为457
#define QR_458          6                  //二维码为458
#define QR_467          7                  //二维码为467
#define QR_468          8                  //二维码为468

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
