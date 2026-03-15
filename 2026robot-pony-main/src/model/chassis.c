#include "chassis.h"
#include "../board/motor_port.h"
#include "../driver/motor.h"
#include "util/datatype.h"
#include "stm32f7xx_hal.h"
#include "../driver/encoder.h"
#include "../board/encoder_port.h"
#include "../board/motor_port.h"


motor_t g_left_motor,g_right_motor;
pid_t g_left_pid,g_right_pid;

typedef struct state{
    i32 position;
    i32 angle;
    i32 platform;
} state_t;

i32 chasssis_raw_motor_get_speed(motor_t* motor,u32 tick){
    u32 speed = encoder_get_count(motor->encoder) / tick;
    return speed;
}

void chassis_raw_motor_set_speed(motor_t* motor,i32 speed){
    motor_set_speed(motor, speed);
}

void chassis_bind_motor(motor_t* left_motor, motor_t* right_motor){
    g_left_motor = *left_motor;
    g_right_motor = *right_motor;
}

void chassis_set_speed(i32 left_speed ,i32 right_speed){
    motor_set_speed(&g_left_motor, left_speed);
    motor_set_speed(&g_right_motor, right_speed);
}

void chassis_init(void){
    motor_init(&g_left_motor);
    motor_init(&g_right_motor);
}

void chassis_enable(void){
    motor_enable(&g_left_motor);
    motor_enable(&g_right_motor);
}

void chassis_disable(void){ 
    motor_disable(&g_left_motor);
    motor_disable(&g_right_motor);
}

void chassis_stop(void){
    motor_set_speed(&g_left_motor, 0);
    motor_set_speed(&g_right_motor, 0);
}

i32 pid_calc(motor_t* motor,pid_t* pid, i32 error){;
    i32 outputValue = pid->kp * error + pid->kd * (error - pid->last_error);
    pid->last_error = error;
    if(outputValue > pid->MaxOutput){
        outputValue = pid->MaxOutput;
    }
    return outputValue;
}

u32 chassis_get_speed(motor_t* motor,u32 tick){
    u32 left_encoder_speed = encoder_get_count(motor->encoder)/tick;
    return left_encoder_speed;
}

void chassis_pid(i32 left_speed ,i32 right_speed ,u32 tick){
    left_speed = pid_calc(&g_left_motor,&g_left_pid, left_speed - chassis_get_speed(&g_left_motor,tick));
    right_speed = pid_calc(&g_right_motor,&g_right_pid, right_speed - chassis_get_speed(&g_right_motor,tick));
    motor_set_speed(&g_left_motor, left_speed);
    motor_set_speed(&g_right_motor, right_speed);
}

void chassis_Open_Or_Close(u8 control_state,i32 left_speed ,i32 right_speed ,u32 tick){
    if(control_state == OPEN_CONTROL){
        motor_set_speed(&g_left_motor, left_speed);
        motor_set_speed(&g_right_motor, right_speed);
    }
    else if(control_state == CLOSE_CONTROL){
        chassis_pid(left_speed ,right_speed ,tick);
    }
}

void chassis_turn_left_local(i32 speed,u32 tick){
    g_left_motor.polarity = -1;
    g_right_motor.polarity = 1;
    motor_set_speed(&g_left_motor, speed);
    motor_set_speed(&g_right_motor, speed);
    chassis_Open_Or_Close(CLOSE_CONTROL, speed, speed, tick);
}

void chassis_turn_right_local(i32 speed,u32 tick){
    g_left_motor.polarity = 1;
    g_right_motor.polarity = -1;
    motor_set_speed(&g_left_motor, speed);
    motor_set_speed(&g_right_motor, speed);
    chassis_Open_Or_Close(CLOSE_CONTROL, speed, speed, tick);
}

void chassis_turn_left_forward(i32 speed,u32 production,u32 tick){
    g_left_motor.polarity = 1;
    g_right_motor.polarity = 1;
    u32 left_speed = speed * production / 100;
    u32 right_speed = speed;
    motor_set_speed(&g_left_motor, left_speed);
    motor_set_speed(&g_right_motor, right_speed);
    chassis_Open_Or_Close(CLOSE_CONTROL, left_speed, right_speed, tick);
}

void chassis_turn_right_forward(i32 speed,u32 production,u32 tick){
    g_left_motor.polarity = 1;
    g_right_motor.polarity = 1;
    u32 right_speed = speed * production / 100;
    u32 left_speed = speed;
    motor_set_speed(&g_left_motor, left_speed);
    motor_set_speed(&g_right_motor, right_speed);
    chassis_Open_Or_Close(CLOSE_CONTROL, left_speed, right_speed, tick);
}

void chassis_straight(i32 speed,u32 tick){
    g_left_motor.polarity = 1;
    g_right_motor.polarity = 1;
    motor_set_speed(&g_left_motor, speed);
    motor_set_speed(&g_right_motor, speed);
    chassis_Open_Or_Close(CLOSE_CONTROL, speed, speed, tick);
}

void chassis_scan_all(state_t state, i32 speed, u32 tick){
    state.platform = 0;
    // while(1){
    //   if(state.position == 100){
    //        state.platform+= 1;
    //        state.position = 101;
    //    }
    //     switch(state.platform){
    //         case 0:
    //             for( ; ; ){
    //                 switch(state.position){
    //                     case 0:
    //                         chassis_straight(speed, tick);
    //                         break;
    //                     case 1:
    //                         chassis_straight(speed, tick);
    //                         break;
    //                     case 2:
    //                         chassis_straight(speed, tick);
    //                         break;
    //                     case 3:
    //                         chassis_straight(speed, tick);
    //                         break;
    //                     case 100:
    //                         chassis_stop();
    //                         break;
    //                 }
    //                 if(state.position == 100) break;
    //             }
    //             break;
    //         case 1:
    //             for( ; ; ){
    //                 switch(state.position){
    //                     case 0:
    //                         chassis_straight(speed, tick);
    //                         break;
    //                     case 4:
    //                         while(state.angle < angle1){
    //                             chassis_turn_right_local(speed, tick);
    //                         }
    //                         clean_wit();
    //                         break;
    //                     case 5:
    //                         chassis_straight(speed, tick);
    //                         break;
    //                     case 6:
    //                         while(state.angle < angle2){
    //                             chassis_turn_right_local(speed, tick);
    //                         }
    //                         clean_wit();
    //                         break;
    //                     case 7:
    //                         chassis_straight(speed, tick);
    //                         break;
    //                     case 100:
    //                         chassis_stop();
    //                         break;
    //                     case 101:
    //                         while(state.angle == 180 || state.angle == -180){
    //                             chassis_turn_right_local(speed, tick);
    //                         }
    //                         clean_wit();
    //                         state.position = 0;
    //                         break;
    //                 }
    //                 if(state.position == 100) break;
    //             }
    //         case 2:
    //             for( ; ; ){
    //                 switch(state.position){
    //                     case 0:
    //                         chassis_straight(speed, tick);
    //                         break;
    //                     case 8:
    //                         chassis_straight(speed, tick);
    //                         break;
    //                     case 9:
    //                         chassis_straight(speed, tick);
    //                         break;
    //                     case 10:
    //                         chassis_straight(speed, tick);
    //                         break;
    //                     case 100:
    //                         chassis_stop();
    //                         break;
    //                     case 101:
    //                         while(state.angle == 180 || state.angle == -180){
    //                             chassis_turn_right_local(speed, tick);
    //                         }
    //                         clean_wit();
    //                         state.position = 0;
    //                         break;
    //                 }
    //                 if(state.position == 100) break;
    //             }
    //         case 3:
    //             for( ; ; ){
    //                 switch(state.position){
    //                     case 0:
    //                         chassis_straight(speed, tick);
    //                         break;
    //                     case 11:
    //                         while(state.angle > angle3){
    //                             chassis_turn_left_local(speed, tick);
    //                         }
    //                         clean_wit();
    //                         break;
    //                     case 12:
    //                         chassis_straight(speed, tick);
    //                     case 13:
    //                         while(state.angle > angle4){
    //                             chassis_turn_left_local(speed, tick);
    //                         }
    //                         clean_wit();
    //                         break;
    //                     case 100:
    //                         chassis_stop();
    //                         break;
    //                     case 101:
    //                         while(state.angle == 180 || state.angle == -180){
    //                             chassis_turn_right_local(speed, tick);
    //                         }
    //                         break;
    //                 }
    //                 if(state.position == 100) break;
    //             }
    //     }
    // }
}
