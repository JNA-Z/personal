/**
 * @file model.c
 * @brief 硬件对象模型层实现
 * @note 所有硬件配置在此层静态初始化
 */

#include "model.h"
#include "tim.h"

/* ============================================================================
 * LED对象 - 静态初始化硬件配置
 * ============================================================================ */
bsp_led_t led1 = {
    .gpiox = LED1_GPIO_Port,
    .gpio_pin = LED1_Pin
};

bsp_led_t led2 = {
    .gpiox = LED2_GPIO_Port,
    .gpio_pin = LED2_Pin
};

/* ============================================================================
 * 按键对象 - 静态初始化硬件配置
 * ============================================================================ */
bsp_key_t key1 = {
    .gpiox = KEY1_GPIO_Port,
    .gpio_pin = KEY1_Pin,
    .pressed = 0
};

bsp_key_t key2 = {
    .gpiox = KEY2_GPIO_Port,
    .gpio_pin = KEY2_Pin,
    .pressed = 0
};

bsp_key_t key3 = {
    .gpiox = KEY3_GPIO_Port,
    .gpio_pin = KEY3_Pin,
    .pressed = 0
};

bsp_key_t key4 = {
    .gpiox = KEY4_GPIO_Port,
    .gpio_pin = KEY4_Pin,
    .pressed = 0
};

/* ============================================================================
 * 编码器对象 - 静态初始化硬件配置
 * ============================================================================ */
bsp_encoder_t encoder_left = {
    .htim = &htim3,
    .capture_count = 0,
    .last_count = 0,
    .overflow_count = 0,
    .shaft_speed = 0.0f,
    .direction = ENCODER_DIR_FORWARD
};

bsp_encoder_t encoder_right = {
    .htim = &htim4,
    .capture_count = 0,
    .last_count = 0,
    .overflow_count = 0,
    .shaft_speed = 0.0f,
    .direction = ENCODER_DIR_FORWARD
};

/* ============================================================================
 * 电机对象 - 静态初始化硬件配置
 * ============================================================================ */
bsp_motor_t motor_left = {
    .htim = &htim2,
    .fwd_channel = TIM_CHANNEL_4,   /* 左电机正转通道 */
    .rev_channel = TIM_CHANNEL_3,   /* 左电机反转通道 */
    .enabled = 0,
    .speed = 0,
    .limit_spd = 1000,
    .dir = MOTOR_DIR_BRAKE
};

bsp_motor_t motor_right = {
    .htim = &htim2,
    .fwd_channel = TIM_CHANNEL_2,   /* 右电机正转通道 */
    .rev_channel = TIM_CHANNEL_1,   /* 右电机反转通道 */
    .enabled = 0,
    .speed = 0,
    .limit_spd = 1000,
    .dir = MOTOR_DIR_BRAKE
};

/* ============================================================================
 * ADC对象 - 光敏二极管
 * ============================================================================ */
bsp_adc_photo_t adc_photo = {
    .initialized = 0
};

/* ============================================================================
 * 按键扫描接口
 * ============================================================================ */
uint8_t model_scan_keys(void)
{
    if (bsp_key_scan(&key1)) return 1;
    if (bsp_key_scan(&key2)) return 2;
    if (bsp_key_scan(&key3)) return 3;
    if (bsp_key_scan(&key4)) return 4;
    return 0;
}

/* ============================================================================
 * 初始化函数
 * ============================================================================ */
void model_init(void)
{
    /* 初始化LED */
    bsp_led_init(&led1);
    bsp_led_init(&led2);
    
    /* 初始化按键 */
    bsp_key_init(&key1);
    bsp_key_init(&key2);
    bsp_key_init(&key3);
    bsp_key_init(&key4);
    
    /* 初始化编码器 */
    bsp_encoder_init(&encoder_left);
    bsp_encoder_init(&encoder_right);
    
    /* 初始化ADC */
    bsp_adc_photo_init(&adc_photo);
    bsp_adc_photo_start(&adc_photo);
    
    /* 初始化电机由 chassis_init() 完成 */
}
