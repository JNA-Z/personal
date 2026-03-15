#ifndef TASKS_H
#define TASKS_H

#ifdef __cplusplus
extern "C" {
#endif

void app_entry_main_init(void);

void app_task_default(void *argument);
void app_task_lcd_show(void *argument);
void app_task_robot_run(void *argument);
void app_task_hardware_handle(void *argument);

#ifdef __cplusplus
}
#endif

#endif
