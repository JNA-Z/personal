#include "navigation.h"
#include <stdlib.h>

nav_context_t g_nav_context = {0};

void         stage_start_init(void* ctx);
nav_stage_t* stage_start_run(void* ctx);
nav_stage_t  g_start_stage = {
     .stage_init = stage_start_init,
     .stage_run  = stage_start_run,
};

void nav_init(void) {}

void nav_task(void* args)
{
    nav_stage_t* current_stage = &g_start_stage;   // 从第一个阶段开始
    nav_stage_t* next_stage    = NULL;             // 初始化 next_stage 指针

    while (1) {
        // 1. 检查上位机命令：如果 next_stage 不为空，就切换
        if (next_stage != NULL) {
            current_stage = next_stage;
            next_stage    = NULL;   // 重置，为下一次切换做准备
            // 初始化新阶段
            if (current_stage->stage_init) {
                current_stage->stage_init(&g_nav_context);
            }
        }

        if (current_stage != NULL) {
            // 2. 运行当前阶段，并获取它自己计算的下一个阶段
            nav_stage_t* stage_computed_next = NULL;
            if (current_stage->stage_run) {
                stage_computed_next = current_stage->stage_run(&g_nav_context);
            }

            // 3. 优先级判断：上位机的命令 > 阶段自己的计算
            if (next_stage == NULL) {
                // 如果上位机没有下命令，就用阶段自己计算的下一个阶段
                next_stage = stage_computed_next;
            }
        }
        else {
            // 阶段为NULL，任务结束
            // chassis_set_speed(0, 0);
        }

        // vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void         stage_start_init(void* ctx)
{

}    

nav_stage_t* stage_start_run(void* ctx)
{
    return NULL;
}
