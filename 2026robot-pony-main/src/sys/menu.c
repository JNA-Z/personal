/**
 * @file menu.c
 * @brief LCD调试显示模块实现
 */

#include "menu.h"
#include "lcd.h"
#include "model.h"
#include "bsp_adc_photo.h"
#include "bsp_encoder.h"
#include "bsp_motor.h"
#include "chassis.h"
#include "distance_patrol.h"
#include <stdio.h>

/* 外部变量 */
extern distance_patrol_t DistancePatrol;

/* ============================================================================
 * 内部变量
 * ============================================================================ */
static menu_page_t s_current_page = MENU_PAGE_MAIN;

/* ============================================================================
 * 初始化函数
 * ============================================================================ */
void menu_init(void)
{
    s_current_page = MENU_PAGE_MAIN;
}

/* ============================================================================
 * 页面控制
 * ============================================================================ */
void menu_next_page(void)
{
    s_current_page = (menu_page_t)((s_current_page + 1) % MENU_PAGE_COUNT);
}

/* ============================================================================
 * 显示函数
 * ============================================================================ */
static void menu_show_main(void)
{
    /* 主页面：替代原LCD_Show() */
    LCD_ShowIntNum(0, 0, distance_patrol_get_capture_right_count(&DistancePatrol), 5, BLACK, WHITE, 12);
    LCD_ShowIntNum(0, 12, bsp_adc_photo_get_raw(&adc_photo, 5), 5, BLACK, WHITE, 12);
}

static void menu_show_adc_raw(void)
{
    char str[24];
    uint8_t i;

    LCD_Fill(0, 0, LCD_W, LCD_H, WHITE);
    LCD_ShowString(0, 0, (uint8_t*)"ADC Raw", BLACK, WHITE, 16, 0);

    for (i = 0; i < 5; i++) {
        sprintf(str, "CH%d:%4d", i, bsp_adc_photo_get_raw(&adc_photo, i));
        LCD_ShowString(0, 20 + i * 14, (uint8_t*)str, BLACK, WHITE, 12, 0);
    }
    for (i = 5; i < 10; i++) {
        sprintf(str, "CH%d:%4d", i, bsp_adc_photo_get_raw(&adc_photo, i));
        LCD_ShowString(64, 20 + (i - 5) * 14, (uint8_t*)str, BLACK, WHITE, 12, 0);
    }
}

static void menu_show_encoder(void)
{
    char str[24];

    LCD_Fill(0, 0, LCD_W, LCD_H, WHITE);
    LCD_ShowString(0, 0, (uint8_t*)"Encoder", BLACK, WHITE, 16, 0);

    sprintf(str, "L:%ld R:%ld", 
            bsp_encoder_get_count(&encoder_left),
            bsp_encoder_get_count(&encoder_right));
    LCD_ShowString(0, 24, (uint8_t*)str, BLACK, WHITE, 12, 0);

    sprintf(str, "L:%.1f R:%.1f", encoder_left.shaft_speed, encoder_right.shaft_speed);
    LCD_ShowString(0, 40, (uint8_t*)str, BLACK, WHITE, 12, 0);
}

static void menu_show_motor(void)
{
    char str[24];

    LCD_Fill(0, 0, LCD_W, LCD_H, WHITE);
    LCD_ShowString(0, 0, (uint8_t*)"Motor", BLACK, WHITE, 16, 0);

    sprintf(str, "L:%d R:%d", 
            bsp_motor_get_speed(&motor_left),
            bsp_motor_get_speed(&motor_right));
    LCD_ShowString(0, 24, (uint8_t*)str, BLACK, WHITE, 12, 0);

    sprintf(str, "En:%d %d", 
            motor_left.enabled,
            motor_right.enabled);
    LCD_ShowString(0, 40, (uint8_t*)str, BLACK, WHITE, 12, 0);
}

void menu_refresh(void)
{
    switch (s_current_page) {
    case MENU_PAGE_MAIN:
        menu_show_main();
        break;
    case MENU_PAGE_ADC_RAW:
        menu_show_adc_raw();
        break;
    case MENU_PAGE_ENCODER:
        menu_show_encoder();
        break;
    case MENU_PAGE_MOTOR:
        menu_show_motor();
        break;
    default:
        break;
    }
}
