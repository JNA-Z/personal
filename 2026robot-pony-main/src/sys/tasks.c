#include "tasks.h"
#include "pack.h"
#include "cmsis_os.h"

void app_entry_main_init(void)
{
    Pack_Init();
}

void app_task_default(void *argument)
{
    (void)argument;
    for (;;) {
        osDelay(1);
    }
}

void app_task_lcd_show(void *argument)
{
    (void)argument;
    for (;;) {
        osDelay(1);
    }
}

void app_task_robot_run(void *argument)
{
    (void)argument;
    for (;;) {
        PatrolLineTest();
        osDelay(1);
    }
}

void app_task_hardware_handle(void *argument)
{
    (void)argument;
    for (;;) {
        __Hardware_Handler();
        osDelay(2);
    }
}
