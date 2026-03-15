/**
 * @file navigation.h
 * @author canrad (1517807724@qq.com)
 * @brief 小车的路径规划
 * @version 0.1
 * @date 2026-01-11
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef APP_NAVIGATION_H
#define APP_NAVIGATION_H

#include "../util/datatype.h"

typedef struct nav_stage nav_stage_t;

struct nav_stage
{
    void (*stage_init)(void* ctx);
    // 返回下一个阶段
    nav_stage_t* (*stage_run)(void* ctx);
};

typedef struct 
{
    // 可以根据需要添加导航上下文数据
    int dummy_data;
} nav_context_t;

#endif // !APP_NAVIGATION_H
