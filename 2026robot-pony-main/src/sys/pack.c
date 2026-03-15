#include <stdio.h>
#include "main.h"
#include "pack.h"
#include "cmsis_os.h"

/* ========== Model层头文件 (硬件对象定义) ========== */
#include "model.h"

/* ========== BSP层头文件 ========== */
#include "bsp.h"

/* ========== Sys层头文件 (算法模块) ========== */
#include "pid.h"
#include "patrol_line.h"
#include "distance_patrol.h"
#include "chassis.h"
#include "door_manager.h"
#include "menu.h"

/* ========== 全局对象定义 (sys模块) ========== */
patrol_line_t    pid_value;         /* 巡线PID控制器 */
distance_patrol_t DistancePatrol;   /* 距离巡逻控制器 */
pid_positional_t turn_direction;    /* 角度环PID控制器 */

char str[63]={0};	                                                   //结合sprintf将数据输出到指定的字符数组中
Sflag flag = {0};													   //放置所有标志位状态位的结构体
Sflag_Device flag_Device = {0};                                        //放置所有设备状态位的结构体
Slocation loc = {0};												   //放置小车所在位置的结构体
                                                     

/********************函数寻址********************/
/*
	功能函数：
	机器人初始化                           （0x00）
	实时检测各个硬件设备                   （0x01）
	过桥                                   （0x02）
	更改方向函数(右转)                     （0x03）
	更改方向函数(左转)                     （0x04）
	平台编码值停止                         （0x05）
	过跷跷板							   （0x06）									   
	撞景点								   （0x07）

	运行函数：                             
    机器人启动							   （0x10）
	机器人从1号平台到2号平台               （0x11）
	机器人从2号平台到3号平台			   （0x12）
	机器人从3号平台到1号门				   （0x13）
	机器人从1号门到2号门				   （0x14）
	机器人从1号门到7号平台				   （0x15）
	机器人从2号门到7号平台				   （0x16）
	机器人从2号门过4号门到7号平台		   （0x17）
	机器人从7号平台到8号平台			   （0x18）
	机器人从8号平台到回家点1			   （0x19）
	机器人从回家点1到1号门				   （0x20）
	机器人从1号门到回家点2				   （0x21）
	机器人从1号门到3号门				   （0x22）
	机器人从3号门到回家点2				   （0x23）
	机器人从3号门过2号门到回家点2		   （0x24）
	机器人从回家点2到家					   （0x25）
	机器人启动（第二轮）				   （0x26）
	机器人从3号平台直达7号平台（第二轮）   （0x27）
	机器人从回家点1直达回家点2（第二轮）   （0x28）
	机器人去六号平台                       （0x29） 
*/




/*机器人初始化（0x00）*/
void Pack_Init(void)                                                   
{	 
	delay_init(216);                                                   //Delay初始化
	LCD_Init();                                                        //LCD初始化
	LCD_Fill(0,0,LCD_W,LCD_H,WHITE);                                   
	
	/* Model层初始化 (硬件对象) */
	model_init();
	
	/* 使能电机和编码器 */
	chassis_init();
	chassis_enable();
	bsp_encoder_start(&encoder_left);
	bsp_encoder_start(&encoder_right);
	
	/* Sys模块初始化 */
	patrol_line_init(&pid_value, &adc_photo);
	patrol_line_set_compare_ad(&pid_value, 4, 5);
	distance_patrol_init(&DistancePatrol, &encoder_right);
	door_manager_init();
	menu_init();
	
	Gyroscope_Init();                                                  //陀螺仪初始化
	// 陀螺仪串口接收由统一 ISR 回调链路管理
	bsp_camera_start_rx();                                              //开启摄像头串口接收中断
}


/*实时检测各个硬件设备（0x01）*/
void Check_Device(void)
{
	bsp_sample_device_state(&flag_Device);
}


/*过桥（0x02）*/
void Up_Down_Bridge(void)
{
	patrol_line_set_pid_kp_kd_yaw(&pid_value, 8,50);
	/*上桥*/
	while(IF_UPHILL)
	{
		if(flag_Device.Color_code_front_left==Online||flag_Device.Color_code_left==Online)
		{
			chassis_patrol_distance_protect(150,0,patrol_line_yaw_compensation(&pid_value, yaw),-60);
			osDelay(1);
		}
		else if(flag_Device.Color_code_front_right==Online||flag_Device.Color_code_right==Online)
		{
			chassis_patrol_distance_protect(150,0,patrol_line_yaw_compensation(&pid_value, yaw),60);
			osDelay(1);
		}
		else
		{
			chassis_patrol_distance_protect(150,0,patrol_line_yaw_compensation(&pid_value, yaw),0);
			osDelay(1);
		}
		osDelay(1);
	}
	while(IF_FLAT)
	{
		if(flag_Device.Color_code_front_left==Online||flag_Device.Color_code_left==Online)
		{
			chassis_patrol_distance_protect(150,0,patrol_line_yaw_compensation(&pid_value, yaw),-60);
			osDelay(1);
		}
		else if(flag_Device.Color_code_front_right==Online||flag_Device.Color_code_right==Online)
		{
			chassis_patrol_distance_protect(150,0,patrol_line_yaw_compensation(&pid_value, yaw),60);
			osDelay(1);
		}
		else
		{
			chassis_patrol_distance_protect(150,0,patrol_line_yaw_compensation(&pid_value, yaw),0);
			osDelay(1);
		}
		osDelay(1);
	}
	while(IF_DOWNHILL)
	{
		if(flag_Device.Color_code_front_left==Online||flag_Device.Color_code_left==Online)
		{
			chassis_patrol_distance_protect(150,0,patrol_line_yaw_compensation(&pid_value, yaw),-60);
			osDelay(1);
		}
		else if(flag_Device.Color_code_front_right==Online||flag_Device.Color_code_right==Online)
		{
			chassis_patrol_distance_protect(150,0,patrol_line_yaw_compensation(&pid_value, yaw),60);
			osDelay(1);
		}
		else
		{
			chassis_patrol_distance_protect(150,0,patrol_line_yaw_compensation(&pid_value, yaw),0);
			osDelay(1);
		}
		osDelay(1);
	}
	/*桥面*/
	while(IF_FLAT)
	{
		if(flag_Device.Color_code_front_left==Online||flag_Device.Color_code_left==Online)
		{
			chassis_patrol_distance_protect(150,0,patrol_line_yaw_compensation(&pid_value, yaw),-60);
			osDelay(1);
		}
		else if(flag_Device.Color_code_front_right==Online||flag_Device.Color_code_right==Online)
		{
			chassis_patrol_distance_protect(150,0,patrol_line_yaw_compensation(&pid_value, yaw),60);
			osDelay(1);
		}
		else
		{
			chassis_patrol_distance_protect(150,0,patrol_line_yaw_compensation(&pid_value, yaw),0);
			osDelay(1);
		}
		osDelay(1);
	}
	/*下桥*/
	while(IF_DOWNHILL)
	{
		if(flag_Device.Color_code_front_left==Online||flag_Device.Color_code_left==Online)
		{
			chassis_patrol_distance_protect(150,0,patrol_line_yaw_compensation(&pid_value, yaw),-60);
			osDelay(1);
		}
		else if(flag_Device.Color_code_front_right==Online||flag_Device.Color_code_right==Online)
		{
			chassis_patrol_distance_protect(150,0,patrol_line_yaw_compensation(&pid_value, yaw),60);
			osDelay(1);
		}
		else
		{
			chassis_patrol_distance_protect(150,0,patrol_line_yaw_compensation(&pid_value, yaw),0);
			osDelay(1);
		}
		osDelay(1);
	}
	while(IF_FLAT)
	{
		if(flag_Device.Color_code_front_left==Online||flag_Device.Color_code_left==Online)
		{
			chassis_patrol_distance_protect(150,0,patrol_line_yaw_compensation(&pid_value, yaw),-60);
			osDelay(1);
		}
		else if(flag_Device.Color_code_front_right==Online||flag_Device.Color_code_right==Online)
		{
			chassis_patrol_distance_protect(150,0,patrol_line_yaw_compensation(&pid_value, yaw),60);
			osDelay(1);
		}
		else
		{
			chassis_patrol_distance_protect(150,0,patrol_line_yaw_compensation(&pid_value, yaw),0);
			osDelay(1);
		}
		osDelay(1);
	}
	while(IF_UPHILL)
	{
		chassis_patrol_distance_protect(150,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
}



/*更改方向函数(右转) 参数：需要更改的角度（绝对值）,位置（平台为1，地皮为0）（0x03）*/
void Change_dir_Pid_Right(float angle,uint8_t location)
{
	float  yaw_compensation;  							 //上平台时取姿态的值用于比较
	float  yaw_value;         							 //处理后的值
	float  yaw_abs;           							 //两角度差值
	uint16_t pid_times;                                  //pid巡线调整次数
	yaw_compensation=yaw;                                                                        
	if(angle>90&&location==1)                           //大角度平台转向参数
	{                                                    
		pid_times = 4000;  
		pid_positional_init_target(&turn_direction, angle,200,angle-25); 	
		pid_positional_set_kp_ki_kd(&turn_direction, 1,1,0.65);      
	}else if(angle<=90&&location==1)
	{                                                    //小角度平台转向参数
        pid_times = 4000;    
		pid_positional_init_target(&turn_direction, angle,200,angle-20); 
		pid_positional_set_kp_ki_kd(&turn_direction, 1.5,1,0.65);
	}else if(angle>90&&location==0)
	{                                                    //大角度地皮转向参数
        pid_times = 5000;    	
		pid_positional_init_target(&turn_direction, angle,300,angle-20); 
		pid_positional_set_kp_ki_kd(&turn_direction, 2,1,0.65);
	}else if(angle<=90&&location==0)
	{                                                    //小角度地皮转向参数
        pid_times = 5000;
		pid_positional_init_target(&turn_direction, angle,300,angle-20); 
		pid_positional_set_kp_ki_kd(&turn_direction, 2,1,0.65);
	}
	osDelay(1);
	for(int i=0;i<pid_times;i++)
	{	
		yaw_value=yaw;
		if(yaw_value>yaw_compensation+10)                //对数据进行处理
		{
			osDelay(1);  yaw_value=yaw_value-360;
		}
		yaw_abs=yaw_compensation-yaw_value;
		chassis_set_speed_lr( 1.0f*(pid_positional_calculate(&turn_direction, yaw_abs)+100),-1.0f*(pid_positional_calculate(&turn_direction, yaw_abs)+115));  //所有平台掉头调速的入口
		if(yaw_abs>=angle-10)
		{
			chassis_set_speed( 0);
			break;
		}
		osDelay(1);
	}
	osDelay(50);
}


/*更改方向函数(左转) 参数：需要更改的角度（绝对值）,位置（平台为1，地皮为0）（0x04）*/
void Change_dir_Pid_Left(float angle,uint8_t location)
{
	float  yaw_compensation;  							 //上平台时取姿态的值用于比较
	float  yaw_value;         							 //处理后的值
	float  yaw_abs;           							 //两角度差值
	uint16_t pid_times;                                  //pid巡线调整次数
	yaw_compensation=yaw;                                                                                
	if(angle>90&&location==1)                           //大角度平台转向参数
	{                                                    
		pid_times = 4000;  
		pid_positional_init_target(&turn_direction, angle,200,angle-25); 	
		pid_positional_set_kp_ki_kd(&turn_direction, 1,1,0.65);       
	}else if(angle<=90&&location==1)
	{                                                    //小角度平台转向参数
        pid_times = 4000;         	
		pid_positional_init_target(&turn_direction, angle,200,angle-20); 
		pid_positional_set_kp_ki_kd(&turn_direction, 1.5,1,0.65);
	}else if(angle>90&&location==0)
	{                                                    //大角度地皮转向参数
        pid_times = 4000;         		
		pid_positional_init_target(&turn_direction, angle,200,angle-20); 
		pid_positional_set_kp_ki_kd(&turn_direction, 2,1.5,0.65);
	}else if(angle<=90&&location==0)
	{                                                    //小角度地皮转向参数
        pid_times = 4000;         		
		pid_positional_init_target(&turn_direction, angle,300,angle-20); 
		pid_positional_set_kp_ki_kd(&turn_direction, 2,1.5,0.65);
	}
	
	for(int i=0;i<pid_times;i++)
	{	
		yaw_value=yaw;
		if(yaw_value<yaw_compensation-10)                //对数据进行处理
		{
			yaw_value=yaw_value+360;
		}
		yaw_abs=yaw_value-yaw_compensation;
		chassis_set_speed_lr( -1.0f*(pid_positional_calculate(&turn_direction, yaw_abs)+100),1.0f*(pid_positional_calculate(&turn_direction, yaw_abs)+100));  //所有平台掉头调速的入口
		if(yaw_abs>=angle-10)
		{
			chassis_set_speed( 0);
			break;
		}
		osDelay(1);
	}
	osDelay(50);
}



/*平台编码值停止（0x05）*/
void Color_stop_terrace(u32 speed)
{
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 5,50);
	distance_patrol_encoder_init(&DistancePatrol);
	while(!distance_patrol_run_right_distance(&DistancePatrol, 1800))
	{
		chassis_patrol_distance_protect(speed,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));
		osDelay(1);
	}
	chassis_set_speed( 0);
	chassis_enable();
	osDelay(50);
}



/*过跷跷板（0x06）*/
void Upper_seesaw(void)
{
	
	while(!(IF_FLAT)){
	if(flag_Device.Color_code_front_right==Online)
    {
		  chassis_patrol_distance_protect(220,0,0,80 );//进行位置pid巡线
			osDelay(1);
		}
		else if(flag_Device.Color_code_front_left==Online)
		{
			chassis_patrol_distance_protect(220,0,0,-40);//进行位置pid巡线
			osDelay(1);
		}
		 if(flag_Device.Color_code_left==Online)
		{
			chassis_patrol_distance_protect(220,0,0,-60);//进行位置pid巡线
			osDelay(1);
		}
		else if(flag_Device.Color_code_right==Online)
		{
			chassis_patrol_distance_protect(220,0,0,60 );//进行位置pid巡线
			osDelay(1);
		}
		else
		{
			chassis_patrol_distance_protect(220,0,0,0);//进行位置pid巡线
			osDelay(1);
		}
	
	}
	
	
	
	
	
	
	 distance_patrol_encoder_init(&DistancePatrol);
	 
   while(!(distance_patrol_run_right_distance(&DistancePatrol, 3000)))
	 {
		 if(flag_Device.Color_code_front_right==Online)
    {
		  chassis_patrol_distance_protect(180,0,0,120 );//进行位置pid巡线
			osDelay(1);
		}
		else if(flag_Device.Color_code_front_left==Online)
		{
			chassis_patrol_distance_protect(180,0,0,-40);//进行位置pid巡线
			osDelay(1);
		}
		 if(flag_Device.Color_code_left==Online)
		{
			chassis_patrol_distance_protect(180,0,0,-60);//进行位置pid巡线
			osDelay(1);
		}
		else if(flag_Device.Color_code_right==Online)
		{
			chassis_patrol_distance_protect(180,0,0,60 );//进行位置pid巡线
			osDelay(1);
		}
		else
		{
			chassis_patrol_distance_protect(180,0,0,0);//进行位置pid巡线
			osDelay(1);
		}
		 
	 }
		 chassis_enable();
	      
}



/*撞景点（0x07）*/
void Robot_Hit_Spot(void)
{
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
	while(flag_Device.Infrared_Hit!=Online)
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
//	chassis_set_speed( 0);
//	osDelay(1000);
	for(u32 i=0;i<=300;i++)
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	chassis_set_speed( 0);
	osDelay(30);
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	patrol_line_set_pid_kp_kd_yaw(&pid_value, 10,20);
	for(u32 i=0;i<=200;i++)
	{	
		chassis_patrol_back( 250,-patrol_line_yaw_compensation(&pid_value, yaw));
		osDelay(1);
	}
}




/*机器人启动（0x10）*/
void Robot_Start(void)
{
	delay_tick_noblock(1000);
	while(roll<=0.001f&&roll>=-0.001f&&pitch<=0.001f&&pitch>=-0.001f&&yaw<=180.001f&&yaw>=179.999f)//若无值时陀螺仪初始化失败
	{
		sprintf(str,"Gyroscope Init Error!");
		LCD_ShowString(8,8,(u8 *)str,BLACK,WHITE,12,0);
		delay_tick_noblock(100);
	}
	/*WT901C姿态传感器初始化成功后，屏幕显示提示信息*/
	sprintf(str,"Gyroscope init success");
	LCD_ShowString(8,8,(u8 *)str,GREEN,WHITE,12,0);
	sprintf(str,"USE START KEY START");
	LCD_ShowString(8,20,(u8 *)str,RED,WHITE,12,0);
	/*等待开始按钮打开*/
	while(!bsp_start_key_is_pressed())
	{	
		delay_tick_noblock(1);
	}
	LCD_Fill(0,0,LCD_W,LCD_H,WHITE);	
	bsp_led_on(&led2);
	flag.start=1;
	/*等待挡板移开，小车启动*/
	while(flag.start)
	{
		if(bsp_barrier_is_blocked())
		{
			delay_tick_noblock(200);
			if(bsp_barrier_is_blocked())
			{
				flag.start=0;
			}
		}
	}
	while(!(flag.start))
	{
		if(bsp_barrier_is_cleared())
		{
			delay_tick_noblock(200);
			if(bsp_barrier_is_cleared())
			{
				flag.start=1;
			}
		}
	}
	
	bsp_led_off(&led2);
	flag.Lcd=1;
	flag.Direction = 0;//设定出发方向
}




/*机器人启动（第二轮）（0x26）*/
void Robot_Start_2(void)
{
	bsp_led_on(&led2);
	/*等待挡板放下，小车复位*/
	while(flag.start)
	{
		if(bsp_barrier_is_blocked())
		{
			delay_tick_noblock(200);
			if(bsp_barrier_is_blocked())
			{
				flag.start=0;
			}
		}
	}
	LCD_Fill(0,0,LCD_W,LCD_H,WHITE);
	sprintf(str,"Reset successfully");
	LCD_ShowString(8,8,(u8 *)str,GREEN,WHITE,12,0);
	flag.Lcd=1;
	flag.Direction = 0;//设定出发方向
	/*等待挡板移开，小车启动*/
	while(!flag.start)
	{
		if(bsp_barrier_is_cleared())
		{
			delay_tick_noblock(200);
			if(bsp_barrier_is_cleared())
			{
				flag.start=1;
			}
		}
	}
	bsp_led_off(&led2);
}





/*机器人从1号平台到2号平台（0x11）*/
 void Robot_loc_1_to_loc_2(void)
{
	/****************************************1号平台起步****************************************/
	chassis_enable();
	while(IF_FLAT)
	{	
		chassis_set_speed( 200);
		osDelay(1);
	}
	patrol_line_set_pid_kp_kd(&pid_value, 0.07,0.01);
	for(u32 i=0;i<=700;i++)
	{
		bsp_adc_photo_update(&adc_photo); 
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	patrol_line_set_pid_kp_kd(&pid_value, 0.015,0.01);
	while(!(IF_UPHILL))
	{
		bsp_adc_photo_update(&adc_photo); 
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	patrol_line_set_yaw_compensation(&pid_value, yaw);		 
	patrol_line_set_pid_kp_kd_yaw(&pid_value, 8 ,46);
	while(!(IF_DOWNHILL))
	{
	if(flag_Device.Color_code_left==Online)
		{
			chassis_patrol_distance_protect(200,0,patrol_line_yaw_compensation(&pid_value, yaw),-60);//进行位置pid巡线
			osDelay(1);
		}
		else if(flag_Device.Color_code_right==Online)
		{
			chassis_patrol_distance_protect(200,0,patrol_line_yaw_compensation(&pid_value, yaw),60 );//进行位置pid巡线
			osDelay(1);
		}
		else
		{
			chassis_patrol_distance_protect(200,0,patrol_line_yaw_compensation(&pid_value, yaw),0);//进行位置pid巡线
			osDelay(1);
		}
	}
while(IF_DOWNHILL)
	{
	if(flag_Device.Color_code_left==Online)
		{
			chassis_patrol_distance_protect(300,0,patrol_line_yaw_compensation(&pid_value, yaw),-60);//进行位置pid巡线
			osDelay(1);
		}
		else if(flag_Device.Color_code_right==Online)
		{
			chassis_patrol_distance_protect(300,0,patrol_line_yaw_compensation(&pid_value, yaw),60 );//进行位置pid巡线
			osDelay(1);
		}
		else
		{
			chassis_patrol_distance_protect(300,0,patrol_line_yaw_compensation(&pid_value, yaw),0);//进行位置pid巡线
			osDelay(1);
		}
	}
while(!(IF_DOWNHILL))
	{
	if(flag_Device.Color_code_left==Online)
		{
			chassis_patrol_distance_protect(300,0,patrol_line_yaw_compensation(&pid_value, yaw),-60);//进行位置pid巡线
			osDelay(1);
		}
		else if(flag_Device.Color_code_right==Online)
		{
			chassis_patrol_distance_protect(300,0,patrol_line_yaw_compensation(&pid_value, yaw),60 );//进行位置pid巡线
			osDelay(1);
		}
		else
		{
			chassis_patrol_distance_protect(300,0,patrol_line_yaw_compensation(&pid_value, yaw),0);//进行位置pid巡线
			osDelay(1);
		}
	}
	while(IF_DOWNHILL)
	{
	if(flag_Device.Color_code_left==Online)
		{
			chassis_patrol_distance_protect(300,0,patrol_line_yaw_compensation(&pid_value, yaw),-60);//进行位置pid巡线
			osDelay(1);
		}
		else if(flag_Device.Color_code_right==Online)
		{
			chassis_patrol_distance_protect(300,0,patrol_line_yaw_compensation(&pid_value, yaw),60 );//进行位置pid巡线
			osDelay(1);
		}
		else
		{
			chassis_patrol_distance_protect(300,0,patrol_line_yaw_compensation(&pid_value, yaw),0);//进行位置pid巡线
			osDelay(1);
		}
	}
	while(!(IF_UPHILL))
	{
	if(flag_Device.Color_code_left==Online)
		{
			chassis_patrol_distance_protect(80,0,patrol_line_yaw_compensation(&pid_value, yaw),-60);//进行位置pid巡线
			osDelay(1);
		}
		else if(flag_Device.Color_code_right==Online)
		{
			chassis_patrol_distance_protect(80,0,patrol_line_yaw_compensation(&pid_value, yaw),60 );//进行位置pid巡线
			osDelay(1);
		}
		else
		{
			chassis_patrol_distance_protect(80,0,patrol_line_yaw_compensation(&pid_value, yaw),0);//进行位置pid巡线
			osDelay(1);
		}
	}
	flag.ad_state=0;
    flag.track_state=0;
	while(flag_Device.Color_code_front_left!=Online)
	{	
		chassis_patrol_distance_protect2( 300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.1);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 5,50);
    /****************************************巡线上坡****************************************/
	while(!(IF_UPHILL))
	{
		chassis_patrol_distance_protect(300,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));
		osDelay(2);
	}
	
	/****************************************巡线上坡****************************************/
	patrol_line_set_pid_kp_kd(&pid_value, 0.05,0.01);
	while(!(IF_DOWNHILL))
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(2);
	}
	Color_stop_terrace(200);	
	/****************************************闪灯展示掉头****************************************/
	osDelay(300);
	bsp_led_toggle(&led1);
	osDelay(300);
	bsp_led_toggle(&led1);
	osDelay(300);
	bsp_led_toggle(&led1);
	osDelay(300);
	bsp_led_toggle(&led1);
	Change_dir_Pid_Right(180,1);
}



/*机器人从2号平台到3号平台（0x12）*/
void Robot_loc_2_to_loc_3(void)
{
	/****************************************2号平台起步****************************************/
	while(!(IF_DOWNHILL))
	{
		chassis_set_speed( 200);
		osDelay(1);
	}
	/****************************************2号平台下坡****************************************/
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.015);
	for(u32 i=0;i<400;i++)
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	/****************************************岔口A转弯检测****************************************/
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	patrol_line_set_pid_kp_kd_yaw(&pid_value, 3,50);
	while(flag_Device.Color_code_front_right!=Online)
	{
		chassis_patrol_distance_protect(300,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));
		osDelay(1);
	}
	chassis_set_speed( 0);
	osDelay(100);
	Change_dir_Pid_Right(40,0);
	/****************************************上山检测****************************************/
	patrol_line_set_pid_kp_kd(&pid_value, 0.01,0.015);
	while(!(IF_UPHILL))
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	/****************************************过山****************************************/
	patrol_line_set_pid_kp_kd(&pid_value, 0.01,0.015);
	for(u32 i=0;i<2500;i++)
	{
		chassis_patrol_distance_protect(150,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	/****************************************岔口C转弯检测****************************************/
	while(flag_Device.Color_code_front_right!=Online)
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	patrol_line_set_pid_kp_kd(&pid_value, 0.007,0.003);
	patrol_line_set_pid_kp_kd_yaw(&pid_value, 7,50);
	while(flag_Device.Color_code_front_left!=Online)
	{
		chassis_patrol_distance_protect(300,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	chassis_set_speed( 0);
	osDelay(100);
	Change_dir_Pid_Right(125,0);
	/****************************************巡线方向矫正****************************************/
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
	for(u32 i=0;i<=1000;i++)
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	/****************************************位置式巡线****************************************/
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	distance_patrol_move_distance(&DistancePatrol, 16000,400,14000);
	distance_patrol_set_kp_ki_kd(&DistancePatrol, 0.005,0.055,0.08);
	distance_patrol_set_max_output(&DistancePatrol, 350);
	patrol_line_set_pid_kp_kd(&pid_value, 0.02,0.012);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 3,50);
	for(u32 i=0;i<=2000;i++)
	{
		chassis_patrol_distance_protect(200,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	/****************************************上坡止停****************************************/
	patrol_line_set_pid_kp_kd(&pid_value, 0.05,0.01);
	while(!(IF_DOWNHILL))
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(2);
	}
	Color_stop_terrace(200);
	Change_dir_Pid_Right(180,1);
}




/*机器人从2号平台到4号平台*/
void Robot_loc_2_to_loc_4(void)
{
	/****************************************2号平台起步****************************************/
	while(!(IF_DOWNHILL))
	{
		chassis_set_speed( 200);
		osDelay(1);
	}
	/****************************************2号平台下坡****************************************/
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.015);
	for(u32 i=0;i<400;i++)
	{
		chassis_patrol_distance_protect(350,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	/****************************************岔口A转弯检测****************************************/
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	patrol_line_set_pid_kp_kd_yaw(&pid_value, 3,50);
	while(flag_Device.Color_code_front_right!=Online)
	{
		chassis_patrol_distance_protect(350,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));
		osDelay(1);
	}
	chassis_set_speed( 0);
	osDelay(50);
	Change_dir_Pid_Right(40,0);
	/****************************************上山检测****************************************/
	patrol_line_set_pid_kp_kd(&pid_value, 0.01,0.015);
	while(!(IF_UPHILL))
	{
		chassis_patrol_distance_protect(350,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	/****************************************过山****************************************/
	patrol_line_set_pid_kp_kd(&pid_value, 0.01,0.015);
	for(u32 i=0;i<2000;i++)
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	/****************************************岔口C转弯检测****************************************/
	while(flag_Device.Color_code_front_right!=Online)
	{
		chassis_patrol_distance_protect(350,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.003);
	patrol_line_set_pid_kp_kd_yaw(&pid_value, 7,50);
	while(flag_Device.Color_code_front_left!=Online)
	{
		chassis_patrol_distance_protect(300,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	chassis_set_speed( 0);
	osDelay(50);
	Change_dir_Pid_Left(40,0);	
	/****************************************巡线方向矫正****************************************/
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
	for(u32 i=0;i<=800;i++)
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	distance_patrol_move_distance(&DistancePatrol, 20000,400,18000);
	distance_patrol_set_kp_ki_kd(&DistancePatrol, 0.005,0.06,0.08);
	distance_patrol_set_max_output(&DistancePatrol, 350);
	patrol_line_set_pid_kp_kd(&pid_value, 0.008,0.015);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 7,50);
	while(!(IF_UPHILL))//for(u32 i=0;i<=2000;i++)
	{
		chassis_patrol_distance_protect(300,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	/****************************************上坡止停****************************************/
	patrol_line_set_pid_kp_kd(&pid_value, 0.05,0.01);
	while(!(IF_DOWNHILL))
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(2);
	}
	Color_stop_terrace(200);
	Change_dir_Pid_Right(180,1);
}



/*机器人从4号平台到3号平台*/
void Robot_loc_4_to_loc_3(void)
{
	/****************************************4号平台起步****************************************/
	while(!(IF_DOWNHILL))
	{		
		chassis_set_speed( 200);
		osDelay(1);
	}
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.015);
	for(u32 i=0;i<=1000;i++)
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	distance_patrol_move_distance(&DistancePatrol, 40000,400,38000);
	distance_patrol_set_kp_ki_kd(&DistancePatrol, 0.002,0.05,0.08);
	distance_patrol_set_max_output(&DistancePatrol, 400);
	patrol_line_set_pid_kp_kd(&pid_value, 0.01,0.015);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 7,50);
	while(!(IF_UPHILL))
	{
		chassis_patrol_distance_protect(300,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	/****************************************上坡止停****************************************/
	patrol_line_set_pid_kp_kd(&pid_value, 0.05,0.01);
	while(!(IF_DOWNHILL))
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(2);
	}
	Color_stop_terrace(200);
	Change_dir_Pid_Right(180,1);
}



/*机器人从3号平台到1号门（0x13）*/
void Robot_loc_3to_door1(void)
{
	/****************************************3号平台起步****************************************/
	while(!(IF_DOWNHILL))
	{		
		chassis_set_speed( 200);
		osDelay(1);
	}
	/****************************************3号平台下坡方向矫正****************************************/
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
	for(u32 i=0;i<=800;i++)
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	/****************************************位置循迹****************************************/
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	distance_patrol_move_distance(&DistancePatrol, 12000,400,10000);
	distance_patrol_set_kp_ki_kd(&DistancePatrol, 0.005,0.055,0.08);
	distance_patrol_set_max_output(&DistancePatrol, 400);
	patrol_line_set_pid_kp_kd(&pid_value, 0.02,0.015);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 5,50);
	while(flag_Device.Color_code_right!=Online)
	{
		chassis_patrol_distance_protect(200,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	chassis_set_speed( 0);
	osDelay(50);
	Change_dir_Pid_Right(90,0);//小角度右转
	/****************************************识别1号门****************************************/
	loc.door = 1;//所在位置1号门
	bsp_camera_start_rx();//开启摄像头串口接收中断
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.003);
	while(!(IF_UPHILL))
	{
		if(door_is_closed(DOOR_ID_1, DOOR_DIR_GO))//1号门禁行
		{
			chassis_set_speed( 0);
			osDelay(100);
			break;
		}else{
			chassis_patrol_distance_protect(350,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		}
		osDelay(1);
	}
	loc.door = 0;//门位挂空防止摄像头非法更改门禁信息
}



/*机器人从1号门到5号平台*/
void Robot_door1_to_loc_5(void)
{
	/****************************************巡线过1号门****************************************/
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.003);
	distance_patrol_encoder_init(&DistancePatrol);
	while(!distance_patrol_run_right_distance(&DistancePatrol, 3500))
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	chassis_enable();
	while(flag_Device.Color_code_front_left!=Online)
	{
		chassis_patrol_distance_protect(400,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	patrol_line_set_pid_kp_kd(&pid_value, 0.01,0.015);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 7,50);
	while(flag_Device.Color_code_right!=Online)
	{
		chassis_patrol_distance_protect(200,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));
		osDelay(1);
	}
	chassis_set_speed( 0);
	osDelay(50);
	Change_dir_Pid_Left(100,0);
	for(u32 i=0;i<=800;i++)
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	distance_patrol_move_distance(&DistancePatrol, 25000,400,23000);
	distance_patrol_set_kp_ki_kd(&DistancePatrol, 0.005,0.05,0.08);
	distance_patrol_set_max_output(&DistancePatrol, 400);
	patrol_line_set_pid_kp_kd(&pid_value, 0.01,0.015);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 7,50);
	while(!(IF_UPHILL))
	{
		chassis_patrol_distance_protect(300,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	patrol_line_set_pid_kp_kd(&pid_value, 0.05,0.01);
	while(!(IF_DOWNHILL))
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	Color_stop_terrace(200);
	Change_dir_Pid_Right(180,1);
}



/*机器人从1号门到2号门（0x14）*/
void Robot_door1_to_door2(void)
{
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	patrol_line_set_pid_kp_kd_yaw(&pid_value, 10,20);
	while(flag_Device.Color_code_right!=Online)
	{	
		chassis_patrol_back( 250,-patrol_line_yaw_compensation(&pid_value, yaw));
		osDelay(1);
	}
	chassis_set_speed( 0);
	osDelay(50);
	Change_dir_Pid_Left(50,0);
	/****************************************识别2号门****************************************/
	loc.door = 2;
	bsp_camera_start_rx();//开启摄像头串口接收中断
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.003);
	while(!(IF_UPHILL))
	{
		if(door_is_closed(DOOR_ID_2, DOOR_DIR_GO))//2号门禁行
		{
			chassis_set_speed( 0);
			osDelay(50);
			break;
		}else{
			chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		}
		osDelay(1);
	}
	loc.door = 0;
}



/*机器人从2号门到5号平台*/
void Robot_door2_to_loc_5(void)
{
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.003);
	distance_patrol_encoder_init(&DistancePatrol);
	while(!distance_patrol_run_right_distance(&DistancePatrol, 3500))
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	chassis_enable();
	/****************************************位置巡线****************************************/
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	distance_patrol_move_distance(&DistancePatrol, 10000,400,8000);
	distance_patrol_set_kp_ki_kd(&DistancePatrol, 0.007,0.06,0.08);
	distance_patrol_set_max_output(&DistancePatrol, 350);
	patrol_line_set_pid_kp_kd(&pid_value, 0.01,0.015);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 7,50);
	while(flag_Device.Color_code_front_right!=Online)
	{
		chassis_patrol_distance_protect(300,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	while(flag_Device.Color_code_front_right==Online)
	{
		chassis_patrol_distance_protect(300,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	while(flag_Device.Color_code_front_right!=Online)
	{
		chassis_patrol_distance_protect(300,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	patrol_line_set_pid_kp_kd(&pid_value, 0.02,0.015);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 5,50);
	while(flag_Device.Color_code_left!=Online&&flag_Device.Color_code_right!=Online)
	{
		chassis_patrol_distance_protect(200,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	chassis_set_speed( 0);
	osDelay(50);
	Change_dir_Pid_Left(40,0);
	patrol_line_set_pid_kp_kd(&pid_value, 0.007,0.01);
	for(u32 i=0;i<=800;i++)
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	distance_patrol_move_distance(&DistancePatrol, 10000,400,9000);
	distance_patrol_set_kp_ki_kd(&DistancePatrol, 0.007,0.05,0.08);
	distance_patrol_set_max_output(&DistancePatrol, 350);
	patrol_line_set_pid_kp_kd(&pid_value, 0.01,0.015);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 7,50);
	while(!(IF_UPHILL))
	{
		chassis_patrol_distance_protect(200,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	patrol_line_set_pid_kp_kd(&pid_value, 0.05,0.01);
	while(!(IF_DOWNHILL))
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	Color_stop_terrace(200);
	Change_dir_Pid_Right(180,1);
}



/*机器人从2号门过4号门到5号平台*/
void Robot_door2_to_door4_to_loc_5(void)
{
	/****************************************倒车转向****************************************/
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	patrol_line_set_pid_kp_kd_yaw(&pid_value, 10,20);
	while(flag_Device.Color_code_right!=Online)
	{	
		chassis_patrol_back( 250,-patrol_line_yaw_compensation(&pid_value, yaw));
		osDelay(1);
	}
	osDelay(50);
	chassis_set_speed( 0);
	Change_dir_Pid_Left(40,0);
	/****************************************巡线矫正****************************************/
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
	for(u32 i=0;i<=500;i++)
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	/****************************************位置巡线****************************************/
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	distance_patrol_move_distance(&DistancePatrol, 9000,400,8500);
	distance_patrol_set_kp_ki_kd(&DistancePatrol, 0.009,0.07,0.08);
	distance_patrol_set_max_output(&DistancePatrol, 350);
	patrol_line_set_pid_kp_kd(&pid_value, 0.02,0.015);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 5,50);
	for(u32 i=0;i<=1500;i++)
	{
		chassis_patrol_distance_protect(200,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	/****************************************岔口D转弯检测****************************************/
	while(flag_Device.Color_code_front_right!=Online)
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	patrol_line_set_pid_kp_kd(&pid_value, 0.02,0.015);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 5,50);
	while(flag_Device.Color_code_front_right!=Online)
	{
		chassis_patrol_distance_protect(300,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));
		osDelay(1);
	}
	while(flag_Device.Color_code_right!=Online)
	{
		chassis_patrol_distance_protect(200,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));
		osDelay(1);
	}
	while(flag_Device.Color_code_right==Online)
	{
		chassis_patrol_distance_protect(200,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));
		osDelay(1);
	}
	while(flag_Device.Color_code_right!=Online)
	{
		chassis_patrol_distance_protect(200,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));
		osDelay(1);
	}
	chassis_set_speed( 0);
		osDelay(50);
	Change_dir_Pid_Right(90,0);
	/****************************************巡线过门****************************************/
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
	while(!(IF_UPHILL))
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.003);
	distance_patrol_encoder_init(&DistancePatrol);
	while(!distance_patrol_run_right_distance(&DistancePatrol, 3500))
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	chassis_enable();	
	/****************************************岔口J转弯检测****************************************/
	while(flag_Device.Color_code_left!=Online)
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	chassis_set_speed( 0);
	Change_dir_Pid_Left(90,0);
	/****************************************巡线矫正****************************************/
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
	for(u32 i=0;i<=500;i++)
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	/****************************************位置巡线****************************************/
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	distance_patrol_move_distance(&DistancePatrol, 9000,400,8500);
	distance_patrol_set_kp_ki_kd(&DistancePatrol, 0.008,0.07,0.08);
	distance_patrol_set_max_output(&DistancePatrol, 350);
	patrol_line_set_pid_kp_kd(&pid_value, 0.02,0.015);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 5,50);
	while(!(IF_DOWNHILL))
	{
		chassis_patrol_distance_protect(200,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	/****************************************上坡止停****************************************/
	patrol_line_set_pid_kp_kd(&pid_value, 0.05,0.01);
	while(!(IF_DOWNHILL))
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(2);
	}
	Color_stop_terrace(200);
	Change_dir_Pid_Right(180,1);
}


/*机器人从悬崖到7号平台（内部函数15、16、17）*/
void Robot_Precipice_to_loc_7(void)
{
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
	while(!(IF_UPHILL))
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	patrol_line_set_pid_kp_kd(&pid_value, 0.08,0.015);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 6,50);
	while(flag_Device.Color_code_left!=Online)
	{
		chassis_patrol_distance_protect(200,0,patrol_line_yaw_compensation(&pid_value, yaw),0);//进行位置pid巡线
		osDelay(1);
	}
	chassis_set_speed( 0);
	osDelay(100);
	Change_dir_Pid_Left(90,0);
	/****************************************巡线方向矫正****************************************/
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
	for(u32 i=0;i<=600;i++)
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	distance_patrol_move_distance(&DistancePatrol, 15000,400,14000);
	distance_patrol_set_kp_ki_kd(&DistancePatrol, 0.006,0.06,0.08);
	distance_patrol_set_max_output(&DistancePatrol, 350);
	patrol_line_set_pid_kp_kd(&pid_value, 0.01,0.015);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 7,50);
	while(!(IF_UPHILL))
	{
		chassis_patrol_distance_protect(200,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	/****************************************巡线上坡****************************************/
	patrol_line_set_pid_kp_kd(&pid_value, 0.05,0.01);
	while(!(IF_DOWNHILL))
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(2);
	}
	Color_stop_terrace(200);	
	Change_dir_Pid_Right(180,1);
}




/*机器人从1号门到7号平台（0x15）*/
void Robot_door1_to_loc_7(void)
{
	/****************************************巡线过1号门****************************************/
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.003);
	distance_patrol_encoder_init(&DistancePatrol);
	while(!distance_patrol_run_right_distance(&DistancePatrol, 4000))
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	chassis_enable();
	/****************************************岔口H转弯检测****************************************/
	while(flag_Device.Color_code_front_left!=Online)
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(2);
	}
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 5,50);
	while(flag_Device.Color_code_right!=Online)
	{
		chassis_patrol_distance_protect(300,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));
		osDelay(1);
	}
	chassis_set_speed( 0);
	osDelay(100);
	Change_dir_Pid_Left(100,0);
	/****************************************巡线方向矫正****************************************/
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
	for(u32 i=0;i<=600;i++)
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	/****************************************岔口I转弯检测****************************************/
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	distance_patrol_move_distance(&DistancePatrol, 6000,400,4500);
	distance_patrol_set_kp_ki_kd(&DistancePatrol, 0.01,0.08,0.08);
	distance_patrol_set_max_output(&DistancePatrol, 350);
	patrol_line_set_pid_kp_kd(&pid_value, 0.02,0.01);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 5,50);
	while(flag_Device.Color_code_right!=Online)
	{
		chassis_patrol_distance_protect(200,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	chassis_set_speed( 0);
	osDelay(100);
	Change_dir_Pid_Right(90,0);
	/****************************************悬崖-7号平台****************************************/
	Robot_Precipice_to_loc_7();
}





/*机器人从5号平台到7号平台*/
void Robot_loc_5to_loc7(void)
{
	/****************************************5号平台起步****************************************/
	while(!(IF_DOWNHILL))
	{		
		chassis_set_speed( 100);
		osDelay(1);
	}
	/****************************************5号平台下坡方向矫正****************************************/
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
	for(u32 i=0;i<=1000;i++)
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	/****************************************位置巡线****************************************/
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	distance_patrol_move_distance(&DistancePatrol, 18000,400,17000);
	distance_patrol_set_kp_ki_kd(&DistancePatrol, 0.005,0.05,0.08);
	distance_patrol_set_max_output(&DistancePatrol, 350);
	patrol_line_set_pid_kp_kd(&pid_value, 0.01,0.015);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 7,50);
	for(u32 i=0;i<=2300;i++)
	{
		chassis_patrol_distance_protect(300,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	while(flag_Device.Color_code_left!=Online)
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	chassis_set_speed( 0);
	osDelay(50);
	Change_dir_Pid_Left(90,0);
	/****************************************悬崖-7号平台****************************************/
	Robot_Precipice_to_loc_7();
}












/*机器人从2号门到7号平台（0x16）*/
void Robot_door2_to_loc_7(void)
{
	/****************************************巡线过2号门****************************************/
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.003);
	distance_patrol_encoder_init(&DistancePatrol);
	while(!distance_patrol_run_right_distance(&DistancePatrol, 4000))
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	chassis_enable();
	/****************************************位置巡线****************************************/
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	distance_patrol_move_distance(&DistancePatrol, 10000,400,9000);
	distance_patrol_set_kp_ki_kd(&DistancePatrol, 0.007,0.05,0.08);
	distance_patrol_set_max_output(&DistancePatrol, 350);
	patrol_line_set_pid_kp_kd(&pid_value, 0.02,0.015);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 5,50);
	while(flag_Device.Color_code_front_right!=Online)
	{
		chassis_patrol_distance_protect(200,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	while(flag_Device.Color_code_front_right==Online)
	{
		chassis_patrol_distance_protect(200,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	while(flag_Device.Color_code_front_right!=Online)
	{
		chassis_patrol_distance_protect(200,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	/****************************************岔口J转弯检测****************************************/
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 5,50);
	while(flag_Device.Color_code_left!=Online&&flag_Device.Color_code_right!=Online)
	{
		chassis_patrol_distance_protect(200,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	osDelay(100);
	chassis_set_speed( 0);
	osDelay(100);
	Change_dir_Pid_Right(130,0);
	/****************************************巡线方向矫正****************************************/
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
	for(u32 i=0;i<=400;i++)
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	/****************************************位置巡线****************************************/
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	distance_patrol_move_distance(&DistancePatrol, 6000,400,4500);
	distance_patrol_set_kp_ki_kd(&DistancePatrol, 0.005,0.06,0.08);
	distance_patrol_set_max_output(&DistancePatrol, 350);
	patrol_line_set_pid_kp_kd(&pid_value, 0.02,0.015);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 5,50);
	while(flag_Device.Color_code_left!=Online)
	{
		chassis_patrol_distance_protect(200,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	chassis_set_speed( 0);
	osDelay(100);
	Change_dir_Pid_Left(90,0);
	/****************************************悬崖-7号平台****************************************/
	Robot_Precipice_to_loc_7();
}




/*机器人从2号门过4号门到7号平台（0x17）*/
void Robot_door2_to_door4_to_loc_7(void)
{
	/****************************************倒车转向****************************************/
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	patrol_line_set_pid_kp_kd_yaw(&pid_value, 10,20);
	while(flag_Device.Color_code_right!=Online)
	{	
		chassis_patrol_back( 250,-patrol_line_yaw_compensation(&pid_value, yaw));
		osDelay(1);
	}
	osDelay(50);
	chassis_set_speed( 0);
	osDelay(100);
	Change_dir_Pid_Left(40,0);
	/****************************************巡线矫正****************************************/
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
	for(u32 i=0;i<=600;i++)
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	/****************************************位置巡线****************************************/
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	distance_patrol_move_distance(&DistancePatrol, 9000,400,8000);
	distance_patrol_set_kp_ki_kd(&DistancePatrol, 0.005,0.07,0.08);
	distance_patrol_set_max_output(&DistancePatrol, 350);
	patrol_line_set_pid_kp_kd(&pid_value, 0.02,0.015);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 5,50);
	for(u32 i=0;i<=1500;i++)
	{
		chassis_patrol_distance_protect(200,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	/****************************************岔口D转弯检测****************************************/
	while(flag_Device.Color_code_front_right!=Online)
	{		
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	patrol_line_set_pid_kp_kd(&pid_value, 0.02,0.015);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 5,50);
	while(flag_Device.Color_code_front_right!=Online)
	{
		chassis_patrol_distance_protect(200,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));
		osDelay(1);
	}
	while(flag_Device.Color_code_right!=Online)
	{
		chassis_patrol_distance_protect(200,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));
		osDelay(1);
	}
	while(flag_Device.Color_code_right==Online)
	{
		chassis_patrol_distance_protect(200,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));
		osDelay(1);
	}
	while(flag_Device.Color_code_right!=Online)
	{
		chassis_patrol_distance_protect(200,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));
		osDelay(1);
	}
	chassis_set_speed( 0);
	Change_dir_Pid_Right(90,0);
	osDelay(100);
	/****************************************过4号门****************************************/
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
	while(!(IF_UPHILL))
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(2);
	}
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.003);
	distance_patrol_encoder_init(&DistancePatrol);
	while(!distance_patrol_run_right_distance(&DistancePatrol, 4000))
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	chassis_enable();

	/****************************************岔口J转弯检测****************************************/
	while(flag_Device.Color_code_front_right!=Online)
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(2);
	}
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	patrol_line_set_pid_kp_kd(&pid_value, 0.02,0.015);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 5,50);
	while(flag_Device.Color_code_left!=Online)
	{
		chassis_patrol_distance_protect(300,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));
		osDelay(1);
	}
	chassis_set_speed( 0);
	osDelay(100);
	Change_dir_Pid_Right(100,0);
	/****************************************巡线方向矫正****************************************/
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
	for(u32 i=0;i<=400;i++)
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	/****************************************位置巡线岔口I转弯检测****************************************/
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	distance_patrol_move_distance(&DistancePatrol, 6000,400,4500);
	distance_patrol_set_kp_ki_kd(&DistancePatrol, 0.005,0.06,0.08);
	distance_patrol_set_max_output(&DistancePatrol, 350);
	patrol_line_set_pid_kp_kd(&pid_value, 0.02,0.015);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 5,50);
	while(flag_Device.Color_code_left!=Online)
	{
		chassis_patrol_distance_protect(200,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	chassis_set_speed( 0);
	osDelay(100);
	Change_dir_Pid_Left(90,0);
	/****************************************悬崖-7号平台****************************************/
	Robot_Precipice_to_loc_7();
}




/*机器人从7号平台到回家点1（未启用）*/
void Robot_loc_7_to_HomePoint1(void)
{
	/****************************************7号平台起步****************************************/
	while(!(IF_DOWNHILL))
	{		
		chassis_set_speed( 100);
		osDelay(1);
	}
	while(!(IF_UPHILL))
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	/****************************************7号平台下坡方向矫正****************************************/
	patrol_line_set_pid_kp_kd(&pid_value, 0.007,0.01);
	for(u32 i=0;i<=500;i++)
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	/****************************************位置巡线****************************************/
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	distance_patrol_move_distance(&DistancePatrol, 12000,400,11000);
	distance_patrol_set_kp_ki_kd(&DistancePatrol, 0.005,0.06,0.08);
	distance_patrol_set_max_output(&DistancePatrol, 350);
	patrol_line_set_pid_kp_kd(&pid_value, 0.02,0.015);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 5,50);
	for(u32 i=0;i<=1800;i++)
	{
		chassis_patrol_distance_protect(200,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	patrol_line_set_pid_kp_kd(&pid_value, 0.007,0.01);
	while(flag_Device.Color_code_right!=Online)
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	chassis_set_speed( 0);
	osDelay(100);
	Change_dir_Pid_Right(90,0);
	patrol_line_set_pid_kp_kd(&pid_value, 0.007,0.01);
	while(!(IF_UPHILL))
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	for(u32 i=0;i<=2000;i++)
	{
		chassis_patrol_distance_protect(150,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	while(flag_Device.Color_code_left!=Online&&flag_Device.Color_code_right!=Online)
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	chassis_set_speed( 0);
	osDelay(100);
	Change_dir_Pid_Right(90,0);
}



/*机器人从7号平台到8号平台（0x18）*/
void Robot_loc_7_to_loc_8(void)
{
	/****************************************7号平台起步****************************************/
	while(!(IF_DOWNHILL))
	{		
		chassis_set_speed( 100);
		osDelay(1);
	}
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
	/****************************************7号平台下坡****************************************/
	while(!(IF_UPHILL))
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	/****************************************7号平台下坡方向矫正****************************************/
	distance_patrol_encoder_init(&DistancePatrol);
	while(!distance_patrol_run_right_distance(&DistancePatrol, 1500))
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	chassis_enable();
	/****************************************位置巡线****************************************/
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	distance_patrol_move_distance(&DistancePatrol, 28000,400,26000);
	distance_patrol_set_kp_ki_kd(&DistancePatrol, 0.004,0.05,0.08);
	distance_patrol_set_max_output(&DistancePatrol, 400);
	patrol_line_set_pid_kp_kd(&pid_value, 0.01,0.015);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 6,50);
	while(!(IF_UPHILL))
	{
		chassis_patrol_distance_protect(300,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	/****************************************上坡****************************************/
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
	while(!(IF_DOWNHILL))
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	while(!(IF_UPHILL))
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	/****************************************巡线上坡****************************************/
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
	while(!(IF_DOWNHILL))
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 5,50);
	distance_patrol_encoder_init(&DistancePatrol);
	while(!distance_patrol_run_right_distance(&DistancePatrol, 2200))
	{
		chassis_patrol_distance_protect(200,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));
		osDelay(1);
	}
	chassis_set_speed( 0);
	chassis_enable();
	osDelay(100);	
	Change_dir_Pid_Left(180,1);
}



/*机器人从8号平台到回家点1（0x19）*/
void Robot_loc_8_to_HomePoint1(void)
{
	/****************************************8号平台起步下坡****************************************/
	while(!(IF_DOWNHILL))
	{		
		chassis_set_speed( 100);
		osDelay(1);
	}
	/****************************************8号平台下坡****************************************/
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
	while(!(IF_UPHILL))
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	while(!(IF_DOWNHILL))
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	while(!(IF_UPHILL))
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	/****************************************8号平台下坡方向矫正****************************************/
	for(u32 i=0;i<=500;i++)
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	/****************************************位置巡线岔口L转弯检测****************************************/
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	distance_patrol_move_distance(&DistancePatrol, 11000,400,9000);
	distance_patrol_set_kp_ki_kd(&DistancePatrol, 0.005,0.06,0.08);
	distance_patrol_set_max_output(&DistancePatrol, 400);
	patrol_line_set_pid_kp_kd(&pid_value, 0.01,0.015);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 7,50);
	while(flag_Device.Color_code_left!=Online)
	{
		chassis_patrol_distance_protect(300,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	osDelay(50);
	chassis_set_speed( 0);
	osDelay(100);
	Change_dir_Pid_Left(130,0);
	/****************************************过跷跷板****************************************/
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
	while(!(IF_UPHILL))
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 5,50);
	distance_patrol_encoder_init(&DistancePatrol);
	while(!distance_patrol_run_right_distance(&DistancePatrol, 4000))
	{
		chassis_patrol_distance_protect(200,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));
		osDelay(1);
	}
	chassis_enable();
	for(u32 i=0;i<=500;i++)
	{
		chassis_set_speed( -40);
		osDelay(2);
	}
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	while(!(IF_UPHILL))
	{
		chassis_patrol_distance_protect(200,0,patrol_line_yaw_compensation(&pid_value, yaw),0);
		osDelay(1);
	}
	for(u32 i=0;i<=500;i++)
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 5,50);
	while(!patrol_line_ad_if_white(&pid_value, 1)&&!patrol_line_ad_if_white(&pid_value, 8))
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	distance_patrol_encoder_init(&DistancePatrol);
	while(!distance_patrol_run_right_distance(&DistancePatrol, 500))
	{
		chassis_patrol_distance_protect(200,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));
		osDelay(1);
	}
	chassis_set_speed( 0);
	osDelay(50);
	chassis_enable();
	Change_dir_Pid_Left(45,0);
	Robot_Hit_Spot();//撞景点
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	patrol_line_set_pid_kp_kd_yaw(&pid_value, 10,20);
	while(flag_Device.Color_code_right!=Online)
	{	
		chassis_patrol_back( 250,-patrol_line_yaw_compensation(&pid_value, yaw));
		osDelay(1);
	}
	chassis_set_speed( 0);
	osDelay(50);
	Change_dir_Pid_Right(90,0);
	while(flag_Device.Color_code_front_right!=Online)
	{
		chassis_patrol_distance_protect(250,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 5,50);
	while(flag_Device.Color_code_right!=Online)
	{
		chassis_patrol_distance_protect(250,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));
		osDelay(1);
	}
	chassis_set_speed( 0);
	flag.Direction = 1;//设定回家方向
}




/*机器人从回家点1到1号门（0x20）*/
void Robot_HomePoint1_to_door1(void)
{
	Change_dir_Pid_Left(90,0);
	chassis_set_speed( 0);
	osDelay(100);
	/****************************************识别1号门****************************************/
	loc.door = 1;//所在位置1号门
	bsp_camera_start_rx();//开启摄像头串口接收中断
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.003);
	while(!(IF_UPHILL))
	{
		if(door_is_closed(DOOR_ID_1, DOOR_DIR_BACK))//1号门禁行
		{
			chassis_set_speed( 0);
			osDelay(100);
			break;
		}else{
			chassis_patrol_distance_protect(250,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		}
		osDelay(1);
	}
	loc.door = 0;//门位挂空防止摄像头非法更改门禁信息
}




/*机器人从1号门到回家点2（0x21）*/
void Robot_door1_to_HomePoint2(void)
{
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.003);
	distance_patrol_encoder_init(&DistancePatrol);
	while(!distance_patrol_run_right_distance(&DistancePatrol, 4000))
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	chassis_enable();
	while(flag_Device.Color_code_left!=Online)
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	chassis_set_speed( 0);
	osDelay(100);
	Change_dir_Pid_Right(90,0);
		/****************************************巡线方向矫正****************************************/
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
	for(u32 i=0;i<=400;i++)
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	/****************************************位置巡线岔口I转弯检测****************************************/
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	distance_patrol_move_distance(&DistancePatrol, 6000,400,4500);
	distance_patrol_set_kp_ki_kd(&DistancePatrol, 0.005,0.06,0.08);
	distance_patrol_set_max_output(&DistancePatrol, 350);
	patrol_line_set_pid_kp_kd(&pid_value, 0.02,0.015);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 5,50);
	while(flag_Device.Color_code_left!=Online)
	{
		chassis_patrol_distance_protect(200,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	osDelay(50);
	chassis_set_speed( 0);
    osDelay(100);
	Change_dir_Pid_Left(120,0);//大角度右转
}




/*机器人从1号门到3号门（0x22）*/
void Robot_door1_to_door3(void)
{
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	patrol_line_set_pid_kp_kd_yaw(&pid_value, 10,20);
	while(flag_Device.Color_code_left!=Online)
	{	
		chassis_patrol_back( 200,-patrol_line_yaw_compensation(&pid_value, yaw));
		osDelay(1);
	}
	chassis_set_speed( 0);
	osDelay(100);
	Change_dir_Pid_Right(50,0);
	patrol_line_set_pid_kp_kd(&pid_value, 0.007,0.01);
	for(u32 i=0;i<=1000;i++)
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	distance_patrol_move_distance(&DistancePatrol, 7000,400,6000);
	distance_patrol_set_kp_ki_kd(&DistancePatrol, 0.006,0.08,0.08);
	distance_patrol_set_max_output(&DistancePatrol, 350);
	patrol_line_set_pid_kp_kd(&pid_value, 0.02,0.015);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 5,50);
	while(flag_Device.Color_code_right!=Online)
	{
		chassis_patrol_distance_protect(200,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	/****************************************识别3号门****************************************/
	loc.door = 3;
	bsp_camera_start_rx();//开启摄像头串口接收中断
	patrol_line_set_pid_kp_kd(&pid_value, 0.007,0.003);
	while(!(IF_UPHILL))
	{
		if(door_is_closed(DOOR_ID_3, DOOR_DIR_BACK))//1号门禁行
		{
			chassis_set_speed( 0);
			osDelay(100);
			break;
		}
		else{
			chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		}
		osDelay(1);
	}
	loc.door = 0;
}



/*机器人从3号门到回家点2（0x23）*/
void Robot_door3_to_HomePoint2(void)
{
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.003);
	distance_patrol_encoder_init(&DistancePatrol);
	while(!distance_patrol_run_right_distance(&DistancePatrol, 4000))
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	chassis_enable();
	while(flag_Device.Color_code_front_right!=Online)
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 5,50);
	while(flag_Device.Color_code_front_right==Online)
	{
		chassis_patrol_distance_protect(300,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));
		osDelay(1);
	}
	while(flag_Device.Color_code_front_right!=Online)
	{
		chassis_patrol_distance_protect(300,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));
		osDelay(1);
	}
	chassis_set_speed( 0);
	osDelay(100);
	Change_dir_Pid_Left(130,0);
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.015);
	for(u32 i=0;i<=500;i++)
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	/****************************************再次位置循迹****************************************/
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	distance_patrol_move_distance(&DistancePatrol, 7000,400,6000);
	distance_patrol_set_kp_ki_kd(&DistancePatrol, 0.006,0.08,0.08);
	distance_patrol_set_max_output(&DistancePatrol, 350);
	patrol_line_set_pid_kp_kd(&pid_value, 0.007,0.015);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 6,50);
	while(flag_Device.Color_code_front_right!=Online)
	{
		chassis_patrol_distance_protect(360,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	chassis_set_speed( 0);
    osDelay(100);
	Change_dir_Pid_Right(40,0);//大角度右转
}



/*机器人从3号门过2号门到回家点2（0x24）*/
void Robot_door3_to_door2_to_HomePoint2(void)
{
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	patrol_line_set_pid_kp_kd_yaw(&pid_value, 10,20);
	while(flag_Device.Color_code_left!=Online)
	{	
		chassis_patrol_back( 200,-patrol_line_yaw_compensation(&pid_value, yaw));
		osDelay(1);
	}
	chassis_set_speed( 0);
	Change_dir_Pid_Left(90,0);
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
	while(!(IF_UPHILL))
	{
		chassis_patrol_distance_protect(350,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.003);
	distance_patrol_encoder_init(&DistancePatrol);
	while(!distance_patrol_run_right_distance(&DistancePatrol, 4000))
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	chassis_enable();
	while(flag_Device.Color_code_right!=Online)
	{
		chassis_patrol_distance_protect(350,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	patrol_line_set_pid_kp_kd_yaw(&pid_value, 10,20);
	while(flag_Device.Color_code_left!=Online)
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),0);
		osDelay(1);
	}
	chassis_set_speed( 0);
	osDelay(100);
	Change_dir_Pid_Right(140,0);
	/****************************************巡线方向矫正****************************************/
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
	for(u32 i=0;i<=500;i++)
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	/****************************************位置巡线岔口I转弯检测****************************************/
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	distance_patrol_move_distance(&DistancePatrol, 6000,400,4500);
	distance_patrol_set_kp_ki_kd(&DistancePatrol, 0.005,0.06,0.08);
	distance_patrol_set_max_output(&DistancePatrol, 350);
	patrol_line_set_pid_kp_kd(&pid_value, 0.02,0.015);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 5,50);
	while(flag_Device.Color_code_left!=Online)
	{
		chassis_patrol_distance_protect(200,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	osDelay(50);
	chassis_set_speed( 0);
    osDelay(100);
	Change_dir_Pid_Left(120,0);//大角度右转
}




/*机器人从回家点2到家（0x25）*/
void Robot_HomePoint2_to_Home(void)
{
	patrol_line_set_pid_kp_kd(&pid_value, 0.01,0.015);
	while(!(IF_UPHILL))
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	patrol_line_set_pid_kp_kd(&pid_value, 0.01,0.015);
	for(u32 i=0;i<2500;i++)
	{
		chassis_patrol_distance_protect(150,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	while(flag_Device.Color_code_front_right!=Online)
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	patrol_line_set_pid_kp_kd(&pid_value, 0.009,0.003);
	patrol_line_set_pid_kp_kd_yaw(&pid_value, 7,50);
	while(flag_Device.Color_code_front_left!=Online)
	{
		chassis_patrol_distance_protect(300,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	chassis_set_speed( 0);
    osDelay(100);
	Change_dir_Pid_Right(140,0);//大角度右转
	
	patrol_line_set_pid_kp_kd(&pid_value, 0.07,0.01);
	for(u32 i=0;i<=700;i++)
	{	
		chassis_patrol_distance_protect2( 200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	patrol_line_set_pid_kp_kd(&pid_value, 0.015,0.01);
	while(!(IF_UPHILL))
	{
		chassis_patrol_distance_protect2( 250,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	patrol_line_set_yaw_compensation(&pid_value, yaw);		 
	patrol_line_set_pid_kp_kd_yaw(&pid_value, 8 ,46);
	while(!(IF_DOWNHILL))
	{
		if(flag_Device.Color_code_left==Online)
		{
			chassis_patrol_distance_protect(200,0,patrol_line_yaw_compensation(&pid_value, yaw),-60);//进行位置pid巡线
			osDelay(1);
		}
		else if(flag_Device.Color_code_right==Online)
		{
			chassis_patrol_distance_protect(200,0,patrol_line_yaw_compensation(&pid_value, yaw),60 );//进行位置pid巡线
			osDelay(1);
		}
		else
		{
			chassis_patrol_distance_protect(200,0,patrol_line_yaw_compensation(&pid_value, yaw),0);//进行位置pid巡线
			osDelay(1);
		}
	}
	while(IF_DOWNHILL)
	{
		if(flag_Device.Color_code_left==Online)
		{
			chassis_patrol_distance_protect(300,0,patrol_line_yaw_compensation(&pid_value, yaw),-60);//进行位置pid巡线
			osDelay(1);
		}
		else if(flag_Device.Color_code_right==Online)
		{
			chassis_patrol_distance_protect(300,0,patrol_line_yaw_compensation(&pid_value, yaw),60 );//进行位置pid巡线
			osDelay(1);
		}
		else
		{
			chassis_patrol_distance_protect(300,0,patrol_line_yaw_compensation(&pid_value, yaw),0);//进行位置pid巡线
			osDelay(1);
		}
	}
	while(!(IF_DOWNHILL))
	{
		
		if(flag_Device.Color_code_left==Online)
		{
			chassis_patrol_distance_protect(300,0,patrol_line_yaw_compensation(&pid_value, yaw),-60);//进行位置pid巡线
			osDelay(1);
		}
		else if(flag_Device.Color_code_right==Online)
		{
			chassis_patrol_distance_protect(300,0,patrol_line_yaw_compensation(&pid_value, yaw),60 );//进行位置pid巡线
			osDelay(1);
		}
		else
		{
			chassis_patrol_distance_protect(300,0,patrol_line_yaw_compensation(&pid_value, yaw),0);//进行位置pid巡线
			osDelay(1);
		}
	}
	while(IF_DOWNHILL)
	{
		if(flag_Device.Color_code_left==Online)
		{
			chassis_patrol_distance_protect(300,0,patrol_line_yaw_compensation(&pid_value, yaw),-60);//进行位置pid巡线
			osDelay(1);
		}
		else if(flag_Device.Color_code_right==Online)
		{
			chassis_patrol_distance_protect(300,0,patrol_line_yaw_compensation(&pid_value, yaw),60 );//进行位置pid巡线
			osDelay(1);
		}
		else
		{
			chassis_patrol_distance_protect(300,0,patrol_line_yaw_compensation(&pid_value, yaw),0);//进行位置pid巡线
			osDelay(1);
		}
	}
	while(!(IF_UPHILL))
	{   	
		if(flag_Device.Color_code_left==Online)
		{
			chassis_patrol_distance_protect(150,0,patrol_line_yaw_compensation(&pid_value, yaw),-60);//进行位置pid巡线
			osDelay(1);
		}
		else if(flag_Device.Color_code_right==Online)
		{
			chassis_patrol_distance_protect(150,0,patrol_line_yaw_compensation(&pid_value, yaw),60 );//进行位置pid巡线
			osDelay(1);
		}
		else
		{
			chassis_patrol_distance_protect(150,0,patrol_line_yaw_compensation(&pid_value, yaw),0);//进行位置pid巡线
			osDelay(1);
		}
	}
	while(IF_UPHILL)
	{
		chassis_patrol_distance_protect2( 250,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);   
	}
	while(!(IF_UPHILL))
	{
		chassis_patrol_distance_protect2( 250,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}
	while(!(IF_UPHILL))
	{
        chassis_patrol_distance_protect2( 300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);	 
	}			 
	/****************************************巡线上坡****************************************/
	patrol_line_set_pid_kp_kd(&pid_value, 0.05,0.01);
	while(!(IF_DOWNHILL))
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(2);
	}
	Color_stop_terrace(200);	
	Change_dir_Pid_Right(180,1);
	chassis_set_speed( 0);
	osDelay(100);
}



/*机器人去六号平台（0x29）*/
void Robot_loc_6(void)
{
	Change_dir_Pid_Left(100,0);
	chassis_set_speed( 0);
	osDelay(100);
  /*********从西湖景点旁边的十字路口开始（转弯后）**********/
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
	for(u32 i=0;i<=500;i++)
	{
		bsp_adc_photo_update(&adc_photo); 		
		chassis_patrol_distance_protect(280,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.012);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 6,46);   
	while(flag_Device.Color_code_front_right!=Online)
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	patrol_line_set_pid_kp_kd(&pid_value, 0.003,0.006);
	patrol_line_set_pid_kp_kd_yaw(&pid_value, 2,12);
	while(flag_Device.Color_code_right!=Online)
	{
		chassis_patrol_distance_protect(200,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));
		osDelay(1);
	}
	chassis_set_speed( 0);
	osDelay(100);
    Change_dir_Pid_Right(130,0);
	chassis_set_speed( 0);
	patrol_line_set_compare_ad(&pid_value, 4,5);  	 
	while(!(IF_UPHILL))
	{
		chassis_patrol_distance_protect(250,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_circle_compensation(&pid_value));
		osDelay(1);
    }
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	patrol_line_set_pid_kp_kd_yaw(&pid_value, 13,51);
	distance_patrol_encoder_init(&DistancePatrol); 
	while(!(distance_patrol_run_right_distance(&DistancePatrol, 400)))
    {
	    chassis_patrol_distance_protect(100,0,patrol_line_yaw_compensation(&pid_value, yaw),0);
	    osDelay(1);
	}
	chassis_enable();
	while(flag_Device.Color_code_front_left!=Online)
		{
	    	chassis_turn_left_ratio( 200, 0);
			osDelay(1);
		}
	Upper_seesaw();//过跷跷板
	for(u32 i=0;i<=500;i++)
	{
		chassis_set_speed( -40);
		osDelay(2);
	}
//	osDelay(200);
	patrol_line_set_compare_ad(&pid_value, 4,5);  
	patrol_line_set_pid_kp_kd(&pid_value, 0.007,0.015);
	while(!(patrol_line_ad_if_white(&pid_value, 3)||patrol_line_ad_if_white(&pid_value, 4)))
	{
		chassis_turn_left_ratio( 300, 0.2);
	    osDelay(1);
	}
	
//	if(pid_value.ad_now[1]>2000||pid_value.ad_now[2]>2000||pid_value.ad_now[3]>2000||pid_value.ad_now[4]>2000||pid_value.ad_now[5]>2000||pid_value.ad_now[6]>2000||pid_value.ad_now[7]>2000||pid_value.ad_now[8]>2000)
//   {
//         
//		bsp_adc_photo_update(&adc_photo); 		
//		chassis_patrol_distance_protect(250,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
//		osDelay(1);
//   }
//   else 
//   {
//      Change_dir_Pid_Left(30,0);//小角度左转
//   
//   }
//	while(pid_value.ad_now[1]<2000&&pid_value.ad_now[2]<2000&&pid_value.ad_now[3]<2000&&pid_value.ad_now[3]<2000&&pid_value.ad_now[7]<2000&&pid_value.ad_now[8]<2000)
//    {
//		bsp_motor_turn_left(&motor_group, 200,-1);
//	    osDelay(1);
//	}
    /*************跷跷板下来到六号平台**********/   
    patrol_line_set_pid_kp_kd(&pid_value, 0.007,0.015);
	while(!(IF_FLAT))
	{
		bsp_adc_photo_update(&adc_photo); 		
		chassis_patrol_distance_protect(250,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	while(flag_Device.Color_code_front_left!=Online)
	{
		bsp_adc_photo_update(&adc_photo); 		
		chassis_patrol_distance_protect(250,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	while(flag_Device.Color_code_front_left==Online)
	{
		bsp_adc_photo_update(&adc_photo); 		
		chassis_patrol_distance_protect(250,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	while(flag_Device.Color_code_left!=Online)
	{
		chassis_patrol_distance_protect(250,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	chassis_set_speed( 0);
	osDelay(100);
	Change_dir_Pid_Left(90,0);//小角度左转
	/****************************************巡线上坡****************************************/
	patrol_line_set_pid_kp_kd(&pid_value, 0.05,0.01);
	while(!(IF_DOWNHILL))
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(2);
	}
	Color_stop_terrace(200);	
	Change_dir_Pid_Right(180,1);
	/************下面这里是平台转弯后要去下一个跷跷板了**************/
	while(!(IF_DOWNHILL))
	{
		chassis_set_speed( 100);
		osDelay(1);
	}
	patrol_line_set_compare_ad(&pid_value, 4,5);
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.015);	
	for(u32 i=0;i<=1000;i++)
	{		
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	patrol_line_set_yaw_compensation(&pid_value, yaw);	
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.015);
	patrol_line_set_pid_kp_kd_yaw(&pid_value, 13,51);
	while(flag_Device.Color_code_left!=Online&&flag_Device.Color_code_right!=Online)
	{
		chassis_patrol_distance_protect(200,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
		osDelay(1);
	}	 
	chassis_set_speed( 0);
	Change_dir_Pid_Left(90,0);//小角度左转
	patrol_line_set_compare_ad(&pid_value, 4,5);
	while(!(IF_UPHILL))
	{
		chassis_patrol_distance_protect(220,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	/*********同样的用上山状态来判断是否到达跷跷板底部（六号平台记得转弯后再用过跷跷板函数）************/
	distance_patrol_encoder_init(&DistancePatrol); 
	patrol_line_set_yaw_compensation(&pid_value, yaw);	
	while(!(distance_patrol_run_right_distance(&DistancePatrol, 400)))
    {
	    chassis_patrol_distance_protect(100,0,patrol_line_yaw_compensation(&pid_value, yaw),0);
	    osDelay(1);
	}
	 chassis_enable();
	while(flag_Device.Color_code_front_left!=Online)
		{
	    	chassis_turn_left_ratio( 200, 0);
			osDelay(1);
		}

	while(!(IF_FLAT))
	{
		if(flag_Device.Color_code_front_right==Online)
		{
			chassis_patrol_distance_protect(180,0,0,100 );//进行位置pid巡线
			osDelay(1);
		}
		else if(flag_Device.Color_code_front_left==Online)
		{
			chassis_patrol_distance_protect(180,0,0,-60);//进行位置pid巡线
			osDelay(1);
		}
		if(flag_Device.Color_code_left==Online)
		{
			chassis_patrol_distance_protect(180,0,0,-60);//进行位置pid巡线
			osDelay(1);
		}
		else if(flag_Device.Color_code_right==Online)
		{
			chassis_patrol_distance_protect(180,0,0,60 );//进行位置pid巡线
			osDelay(1);
		}
		else
		{
			chassis_patrol_distance_protect(180,0,0,0);//进行位置pid巡线
			osDelay(1);
		}
	}
	distance_patrol_encoder_init(&DistancePatrol); 
	while(!(distance_patrol_run_right_distance(&DistancePatrol, 3000)))
	{
		if(flag_Device.Color_code_front_right==Online)
		{
			chassis_patrol_distance_protect(180,0,0,60 );//进行位置pid巡线
			osDelay(1);
		}
		else if(flag_Device.Color_code_front_left==Online)
		{
			chassis_patrol_distance_protect(180,0,0,-60);//进行位置pid巡线
			osDelay(1);
		}
		 if(flag_Device.Color_code_left==Online)
		{
			chassis_patrol_distance_protect(180,0,0,-60);//进行位置pid巡线
			osDelay(1);
		}
		else if(flag_Device.Color_code_right==Online)
		{
			chassis_patrol_distance_protect(180,0,0,60 );//进行位置pid巡线
			osDelay(1);
		}
		else
		{
			chassis_patrol_distance_protect(180,0,0,0);//进行位置pid巡线
			osDelay(1);
		}
	 }
		chassis_enable();
	    chassis_set_speed( -40);
		osDelay(1000);
	        distance_patrol_encoder_init(&DistancePatrol);
	    patrol_line_set_yaw_compensation(&pid_value, yaw);		 
	    patrol_line_set_pid_kp_kd_yaw(&pid_value, 13,50);
	      
        while(!(distance_patrol_run_right_distance(&DistancePatrol, 300)))
        {
		   chassis_patrol_distance_protect(140,0,patrol_line_yaw_compensation(&pid_value, yaw),0);
		}
		 chassis_enable();//过跷跷板
     while(!(patrol_line_ad_if_white(&pid_value, 1)||patrol_line_ad_if_white(&pid_value, 2)||patrol_line_ad_if_white(&pid_value, 3)||patrol_line_ad_if_white(&pid_value, 4)||patrol_line_ad_if_white(&pid_value, 5)||patrol_line_ad_if_white(&pid_value, 6)||patrol_line_ad_if_white(&pid_value, 7)))
	  {
			chassis_turn_left_ratio( 300, 0.2);
			osDelay(1);
			
	   }
		 
//		while(pid_value.ad_now[1]<2000&&pid_value.ad_now[2]<2000&&pid_value.ad_now[3]<2000&&pid_value.ad_now[3]<2000&&pid_value.ad_now[7]<2000&&pid_value.ad_now[8]<2000)
//    {
//		bsp_motor_turn_left(&motor_group, 200,-1);
//	    osDelay(1);
//	}
	 

		while(flag_Device.Color_code_right!=Online)
		{
			chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));//进行位置pid巡线
			osDelay(1);
		}
		Change_dir_Pid_Left(30,0);
		patrol_line_set_pid_kp_kd_yaw(&pid_value, 10,50);
		patrol_line_set_yaw_compensation(&pid_value, yaw);
		while(!(patrol_line_ad_if_white(&pid_value, 0)||patrol_line_ad_if_white(&pid_value, 1)))
		{
			chassis_patrol_distance_protect(200,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
			osDelay(1);
		}
	
		while(flag_Device.Color_code_front_left!=Online)
		{
			chassis_patrol_distance_protect(200,0,patrol_line_yaw_compensation(&pid_value, yaw),0);
			osDelay(1);
		}
		while(flag_Device.Color_code_front_left==Online)
		{
			chassis_patrol_distance_protect(200,0,patrol_line_yaw_compensation(&pid_value, yaw),0);
			osDelay(1);
		}
//		while(flag_Device.Color_code_right!=Online)
//		{
//			chassis_patrol_distance_protect(200,0,patrol_line_yaw_compensation(&pid_value, yaw),0);//进行位置pid巡线
//			osDelay(1);
//		}


//		patrol_line_set_pid_kp_kd_yaw(&pid_value, 13,51);
//		patrol_line_set_yaw_compensation(&pid_value, yaw);
//		while(flag_Device.Color_code_left!=Online)
//		{
//			chassis_patrol_distance_protect(200,0,patrol_line_yaw_compensation(&pid_value, yaw),0);//进行位置pid巡线
//			osDelay(1);
//		}
//		while(1)
//		{
//		chassis_set_speed( 0);
//			osDelay(1);
//		}
//		chassis_set_speed( 0);
//		osDelay(100);
//		Change_dir_Pid_Left(30,0);
//		while(!(patrol_line_ad_if_white(&pid_value, 2)||patrol_line_ad_if_white(&pid_value, 3)||patrol_line_ad_if_white(&pid_value, 4)))
//		{
//			chassis_turn_left_ratio( 200, 0);
//			osDelay(1);
//		}
//		for(u32 i=0;i<=700;i++)
//		{
//			chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));//进行位置pid巡线
//			osDelay(2);
//		}
//		while(pid_value.ad_now[2]<2000&&pid_value.ad_now[3]<2000&&pid_value.ad_now[4]<2000)
//        {
//				bsp_motor_turn_left(&motor_group, 200,-1);
//		}
		for(u32 i=0;i<=700;i++)
		{
			chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));//进行位置pid巡线
			osDelay(2);
		}
		patrol_line_set_yaw_compensation(&pid_value, yaw);
		patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.015);
		patrol_line_set_pid_kp_kd_yaw(&pid_value, 5,50);
		while(flag_Device.Color_code_right!=Online&&flag_Device.Color_code_left!=Online)
		{
			chassis_patrol_distance_protect(300,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
			osDelay(1);
		}
		chassis_set_speed( 0);
		osDelay(100);
}



void Robot_HomePoint1_to_loc_6(void)
{
	Change_dir_Pid_Left(100,0);
	chassis_set_speed( 0);
	osDelay(100);
  /*********从西湖景点旁边的十字路口开始（转弯后）**********/
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
	for(u32 i=0;i<=500;i++)
	{
		bsp_adc_photo_update(&adc_photo); 		
		chassis_patrol_distance_protect(280,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.012);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 6,46);   
	while(flag_Device.Color_code_front_right!=Online)
	{
		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	patrol_line_set_pid_kp_kd(&pid_value, 0.003,0.006);
	patrol_line_set_pid_kp_kd_yaw(&pid_value, 2,12);
	while(flag_Device.Color_code_right!=Online)
	{
		chassis_patrol_distance_protect(200,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));
		osDelay(1);
	}
	chassis_set_speed( 0);
	osDelay(50);
    Change_dir_Pid_Right(130,0);
	chassis_set_speed( 0);
	distance_patrol_encoder_init(&DistancePatrol);
	while(!distance_patrol_run_right_distance(&DistancePatrol, 3800))
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	chassis_set_speed( 0);
	chassis_enable();
	Change_dir_Pid_Left(30,0);
	chassis_set_speed( 0);
	osDelay(50);
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 5,50);
	distance_patrol_encoder_init(&DistancePatrol);
	while(!distance_patrol_run_right_distance(&DistancePatrol, 3800))
	{
		chassis_patrol_distance_protect(200,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));
		osDelay(1);
	}
	chassis_enable();
	chassis_set_speed( 0);
	osDelay(1000);
	while(!(patrol_line_ad_if_white(&pid_value, 3)||patrol_line_ad_if_white(&pid_value, 4)))
	{
		chassis_turn_left_ratio( 350, 0.2);
		osDelay(1);
	}
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
	for(u32 i=0;i<=800;i++)
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	while(flag_Device.Color_code_left!=Online)
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	Change_dir_Pid_Left(90,0);
	chassis_set_speed( 0);
	osDelay(50);
		/****************************************上坡止停****************************************/
	patrol_line_set_pid_kp_kd(&pid_value, 0.05,0.01);
	while(!(IF_DOWNHILL))
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(2);
	}
	Color_stop_terrace(200);
	Change_dir_Pid_Right(180,1);
	
	while(!(IF_DOWNHILL))
	{	
		chassis_set_speed( 100);
		osDelay(1);
	}
	for(u32 i=0;i<=800;i++)
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);

	}
	while(flag_Device.Color_code_left!=Online)
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	Change_dir_Pid_Left(100,0);
	chassis_set_speed( 0);
	osDelay(50);
	distance_patrol_encoder_init(&DistancePatrol);
	while(!distance_patrol_run_right_distance(&DistancePatrol, 4900))
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	chassis_set_speed( 0);
	chassis_enable();
	Change_dir_Pid_Left(35,0);
	chassis_set_speed( 0);
	osDelay(50);
	patrol_line_set_yaw_compensation(&pid_value, yaw);
	patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
    patrol_line_set_pid_kp_kd_yaw(&pid_value, 5,50);
	distance_patrol_encoder_init(&DistancePatrol);
	while(!distance_patrol_run_right_distance(&DistancePatrol, 3700))
	{
		chassis_patrol_distance_protect(200,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));
		osDelay(1);
	}
	chassis_enable();
	chassis_set_speed( 0);
	osDelay(1000);
	while(!(patrol_line_ad_if_white(&pid_value, 3)||patrol_line_ad_if_white(&pid_value, 4)))
	{
		chassis_turn_left_ratio( 350, 0.2);
		osDelay(1);
	}
	while(flag_Device.Color_code_front_right!=Online)
	{
		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
		osDelay(1);
	}
	distance_patrol_encoder_init(&DistancePatrol);
	while(!distance_patrol_run_right_distance(&DistancePatrol, 3500))
	{
		chassis_turn_left_ratio( 200, 0.8);
		osDelay(1);
	}
	chassis_set_speed( 0);
	chassis_enable();
	
}






/*机器人从3号平台直达7号平台（第二轮）（0x27）*/
void Robot_loc_3_to_loc_7(void)
{

	if(door_is_closed(DOOR_ID_1, DOOR_DIR_GO)&&door_is_closed(DOOR_ID_2, DOOR_DIR_GO))//1、2号门都禁行时走4号门
	{
		/****************************************3号平台起步****************************************/
		while(!(IF_DOWNHILL))
		{		
			chassis_set_speed( 200);
			osDelay(1);
		}
		 patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
		while(!(IF_UPHILL))
		{		
			chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
			osDelay(1);
		}
		for(u32 i=0;i<=800;i++)
		{
			chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
			osDelay(1);
		}
		patrol_line_set_yaw_compensation(&pid_value, yaw);
		distance_patrol_move_distance(&DistancePatrol, 25000,400,23000);
		distance_patrol_set_kp_ki_kd(&DistancePatrol, 0.005,0.04,0.08);
		distance_patrol_set_max_output(&DistancePatrol, 350);
		patrol_line_set_pid_kp_kd(&pid_value, 0.01,0.015);
		patrol_line_set_pid_kp_kd_yaw(&pid_value, 6,50);
		for(u32 i=0;i<=2800;i++)
		{
			chassis_patrol_distance_protect(200,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
			osDelay(1);
		} 
		patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
		while(flag_Device.Color_code_front_right!=Online)
		{
			chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
			osDelay(1);
		}
		patrol_line_set_yaw_compensation(&pid_value, yaw);
		patrol_line_set_pid_kp_kd_yaw(&pid_value, 6,50);
		while(flag_Device.Color_code_right!=Online)
		{
			chassis_patrol_distance_protect(200,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));
			osDelay(1);
		}
		while(flag_Device.Color_code_right==Online)
		{
			chassis_patrol_distance_protect(200,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));
			osDelay(1);
		}
				while(flag_Device.Color_code_right!=Online)
		{
			chassis_patrol_distance_protect(200,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));
			osDelay(1);
		}
		chassis_set_speed( 0);
		osDelay(100);
		Change_dir_Pid_Right(90,0);
		
		patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
		while(!(IF_UPHILL))
		{
			chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
			osDelay(2);
		}
		/****************************************巡线过4号门****************************************/
		patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.003);
		distance_patrol_encoder_init(&DistancePatrol);
		while(!distance_patrol_run_right_distance(&DistancePatrol, 4000))
		{
			chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
			osDelay(1);
		}
		chassis_enable();
		while(flag_Device.Color_code_front_right!=Online)
		{	
			chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
			osDelay(2);
		}
		patrol_line_set_yaw_compensation(&pid_value, yaw);
		patrol_line_set_pid_kp_kd_yaw(&pid_value, 6,50);
		while(flag_Device.Color_code_left!=Online)
		{	
			chassis_patrol_distance_protect(300,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));
			osDelay(1);
		}
		chassis_set_speed( 0);
		osDelay(100);
		Change_dir_Pid_Right(90,0);
		/****************************************巡线方向矫正****************************************/
		patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
		for(u32 i=0;i<=400;i++)
		{
			chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
			osDelay(1);
		}
		/****************************************位置巡线岔口I转弯检测****************************************/
		patrol_line_set_yaw_compensation(&pid_value, yaw);
		distance_patrol_move_distance(&DistancePatrol, 6000,400,4500);
		distance_patrol_set_kp_ki_kd(&DistancePatrol, 0.005,0.06,0.08);
		distance_patrol_set_max_output(&DistancePatrol, 350);
		patrol_line_set_pid_kp_kd(&pid_value, 0.02,0.015);
		patrol_line_set_pid_kp_kd_yaw(&pid_value, 5,50);
		while(flag_Device.Color_code_left!=Online)
		{
			chassis_patrol_distance_protect(200,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
			osDelay(1);
		}
		chassis_set_speed( 0);
		osDelay(100);
		Change_dir_Pid_Left(90,0);
	}else{
		/****************************************3号平台起步****************************************/
		while(!(IF_DOWNHILL))
		{		
			chassis_set_speed( 200);
			osDelay(1);
		}
		/****************************************3号平台下坡方向矫正****************************************/
		patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
		for(u32 i=0;i<=800;i++)
		{
			chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
			osDelay(1);
		}
		/****************************************位置循迹****************************************/
		patrol_line_set_yaw_compensation(&pid_value, yaw);
		distance_patrol_move_distance(&DistancePatrol, 10000,400,9000);
		distance_patrol_set_kp_ki_kd(&DistancePatrol, 0.005,0.055,0.08);
		distance_patrol_set_max_output(&DistancePatrol, 350);
		patrol_line_set_pid_kp_kd(&pid_value, 0.02,0.015);
		patrol_line_set_pid_kp_kd_yaw(&pid_value, 5,50);
		while(flag_Device.Color_code_right!=Online)
		{
			chassis_patrol_distance_protect(200,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
			osDelay(1);
		}
		chassis_set_speed( 0);
		osDelay(100);
		if(door_is_passable(DOOR_ID_1, DOOR_DIR_GO))//1号门通行
		{
			Change_dir_Pid_Right(90,0);//小角度右转
			patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
			while(!(IF_UPHILL))
			{		
				chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
				osDelay(1);
			}
			/****************************************巡线过1号门****************************************/
			patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.003);
			distance_patrol_encoder_init(&DistancePatrol);
			while(!distance_patrol_run_right_distance(&DistancePatrol, 4000))
			{
				chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
				osDelay(1);
			}
			chassis_enable();
			while(flag_Device.Color_code_front_left!=Online)
			{	
				chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
				osDelay(2);
			}
			patrol_line_set_yaw_compensation(&pid_value, yaw);
			patrol_line_set_pid_kp_kd_yaw(&pid_value, 6,50);
			while(flag_Device.Color_code_right!=Online)
			{	
				chassis_patrol_distance_protect(300,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));
				osDelay(1);
			}
			chassis_set_speed( 0);
			osDelay(100);
			Change_dir_Pid_Left(90,0);
			/****************************************巡线方向矫正****************************************/
			patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
			for(u32 i=0;i<=400;i++)
			{
				chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
				osDelay(1);
			}
			/****************************************位置巡线岔口I转弯检测****************************************/
			patrol_line_set_yaw_compensation(&pid_value, yaw);
			distance_patrol_move_distance(&DistancePatrol, 6000,400,4500);
			distance_patrol_set_kp_ki_kd(&DistancePatrol, 0.005,0.06,0.08);
			distance_patrol_set_max_output(&DistancePatrol, 350);
			patrol_line_set_pid_kp_kd(&pid_value, 0.02,0.015);
			patrol_line_set_pid_kp_kd_yaw(&pid_value, 5,50);
			while(flag_Device.Color_code_right!=Online)
			{
				chassis_patrol_distance_protect(200,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
				osDelay(1);
			}
			chassis_set_speed( 0);
			osDelay(100);
			Change_dir_Pid_Right(90,0);
		}else{//1号门禁行，走2号门
			Change_dir_Pid_Right(40,0);//小角度右转
			patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
			while(!(IF_UPHILL))
			{		
				chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
				osDelay(2);
			}
			/****************************************巡线过2号门****************************************/
			patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.003);
			distance_patrol_encoder_init(&DistancePatrol);
			while(!distance_patrol_run_right_distance(&DistancePatrol, 4500))
			{
				chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
				osDelay(1);
			}
			chassis_enable();
			/****************************************位置循迹****************************************/
			patrol_line_set_yaw_compensation(&pid_value, yaw);
			distance_patrol_move_distance(&DistancePatrol, 10000,400,8000);
			distance_patrol_set_kp_ki_kd(&DistancePatrol, 0.005,0.055,0.08);
			distance_patrol_set_max_output(&DistancePatrol, 350);
			patrol_line_set_pid_kp_kd(&pid_value, 0.01,0.015);
			patrol_line_set_pid_kp_kd_yaw(&pid_value, 5,50);
			for(u32 i=0;i<=1400;i++)
			{
				chassis_patrol_distance_protect(200,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
				osDelay(1);
			}
			while(flag_Device.Color_code_front_left!=Online)
			{
				chassis_patrol_distance_protect(200,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
				osDelay(1);
			}
			while(flag_Device.Color_code_front_left==Online)
			{
				chassis_patrol_distance_protect(200,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
				osDelay(1);
			}
			while(flag_Device.Color_code_front_left!=Online)
			{
				chassis_patrol_distance_protect(200,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
				osDelay(1);
			}
			while(flag_Device.Color_code_front_left==Online)
			{
				chassis_patrol_distance_protect(200,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
				osDelay(1);
			}
			chassis_set_speed( 0);
			osDelay(100);
			Change_dir_Pid_Right(140,0);
			/****************************************巡线方向矫正****************************************/
			patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
			for(u32 i=0;i<=400;i++)
			{
				chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
				osDelay(1);
			}
			/****************************************位置巡线岔口I转弯检测****************************************/
			patrol_line_set_yaw_compensation(&pid_value, yaw);
			distance_patrol_move_distance(&DistancePatrol, 6000,400,4500);
			distance_patrol_set_kp_ki_kd(&DistancePatrol, 0.005,0.06,0.08);
			distance_patrol_set_max_output(&DistancePatrol, 350);
			patrol_line_set_pid_kp_kd(&pid_value, 0.02,0.015);
			patrol_line_set_pid_kp_kd_yaw(&pid_value, 5,50);
			while(flag_Device.Color_code_left!=Online)
			{
				chassis_patrol_distance_protect(200,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
				osDelay(1);
			}
			chassis_set_speed( 0);
			osDelay(100);
			Change_dir_Pid_Left(90,0);
		}
	}
	/****************************************悬崖-7号平台****************************************/
	Robot_Precipice_to_loc_7();
}





/*机器人从回家点1直达回家点2（第二轮）（0x28）*/
void Robot_HomePoint1_to_HomePoint2(void)
{
	if(door_is_passable(DOOR_ID_1, DOOR_DIR_GO))//1号门通行
	{
		patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.1);
		while(!(IF_UPHILL))
		{
			chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
			osDelay(2);
		}
		/****************************************巡线过1号门****************************************/
		patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.003);
		distance_patrol_encoder_init(&DistancePatrol);
		while(!distance_patrol_run_right_distance(&DistancePatrol, 4000))
		{
			chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
			osDelay(1);
		}
		chassis_enable();
		while(flag_Device.Color_code_front_right!=Online)
		{
			chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
			osDelay(1);
		}
		patrol_line_set_yaw_compensation(&pid_value, yaw);
		patrol_line_set_pid_kp_kd_yaw(&pid_value, 5,50);
		while(flag_Device.Color_code_left!=Online)
		{
			chassis_patrol_distance_protect(300,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));
			osDelay(1);
		}
		chassis_set_speed( 0);
		osDelay(50);
		Change_dir_Pid_Right(100,0);
		/****************************************巡线方向矫正****************************************/
		patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
		for(u32 i=0;i<=600;i++)
		{
			chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
			osDelay(1);
		}
		/****************************************位置巡线岔口I转弯检测****************************************/
		patrol_line_set_yaw_compensation(&pid_value, yaw);
		distance_patrol_move_distance(&DistancePatrol, 6000,400,4500);
		distance_patrol_set_kp_ki_kd(&DistancePatrol, 0.005,0.06,0.08);
		distance_patrol_set_max_output(&DistancePatrol, 350);
		patrol_line_set_pid_kp_kd(&pid_value, 0.01,0.015);
		patrol_line_set_pid_kp_kd_yaw(&pid_value, 7,50);
		while(flag_Device.Color_code_left!=Online)
		{
			chassis_patrol_distance_protect(200,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
			osDelay(1);
		}
		osDelay(50);
		chassis_set_speed( 0);
		osDelay(100);
		Change_dir_Pid_Left(130,0);//大角度右转
	}else if(door_is_passable(DOOR_ID_2, DOOR_DIR_GO)){
		Change_dir_Pid_Right(60,0);
	    chassis_set_speed( 0);
	    osDelay(50);
		patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
		for(u32 i=0;i<=500;i++)
		{
			chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
			osDelay(1);
		}
		patrol_line_set_yaw_compensation(&pid_value, yaw);
		distance_patrol_move_distance(&DistancePatrol, 7000,400,6000);
		distance_patrol_set_kp_ki_kd(&DistancePatrol, 0.006,0.08,0.08);
		distance_patrol_set_max_output(&DistancePatrol, 350);
		patrol_line_set_pid_kp_kd(&pid_value, 0.01,0.015);
		patrol_line_set_pid_kp_kd_yaw(&pid_value, 7,50);
		while(flag_Device.Color_code_left!=Online)
		{
			chassis_patrol_distance_protect(200,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
			osDelay(1);
		}
		chassis_set_speed( 0);
		osDelay(50);
		Change_dir_Pid_Left(100,0);
		patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
		while(!(IF_UPHILL))
		{
			chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
			osDelay(1);
		}
		patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
		distance_patrol_encoder_init(&DistancePatrol);
		while(!distance_patrol_run_right_distance(&DistancePatrol, 4000))
		{
			chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
			osDelay(1);
		}
		chassis_enable();
		while(flag_Device.Color_code_front_right!=Online)
		{
			chassis_patrol_distance_protect(400,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
			osDelay(1);
		}
		patrol_line_set_yaw_compensation(&pid_value, yaw);
		patrol_line_set_pid_kp_kd(&pid_value, 0.01,0.015);
		patrol_line_set_pid_kp_kd_yaw(&pid_value, 7,50);
		while(flag_Device.Color_code_right!=Online)
		{
			chassis_patrol_distance_protect(200,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));
			osDelay(1);
		}
		chassis_set_speed( 0);
		osDelay(50);
		Change_dir_Pid_Right(130,0);
		/****************************************巡线方向矫正****************************************/
		patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
		for(u32 i=0;i<=600;i++)
		{
			chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
			osDelay(1);
		}
		/****************************************位置巡线岔口I转弯检测****************************************/
		patrol_line_set_yaw_compensation(&pid_value, yaw);
		distance_patrol_move_distance(&DistancePatrol, 6000,400,4500);
		distance_patrol_set_kp_ki_kd(&DistancePatrol, 0.005,0.06,0.08);
		distance_patrol_set_max_output(&DistancePatrol, 350);
		patrol_line_set_pid_kp_kd(&pid_value, 0.01,0.015);
		patrol_line_set_pid_kp_kd_yaw(&pid_value, 7,50);
		while(flag_Device.Color_code_left!=Online)
		{
			chassis_patrol_distance_protect(200,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
			osDelay(1);
		}
		osDelay(50);
		chassis_set_speed( 0);
		osDelay(100);
		Change_dir_Pid_Left(130,0);//大角度右转
	}else if(door_is_passable(DOOR_ID_3, DOOR_DIR_GO)){//1号门禁行，3号门通行
		Change_dir_Pid_Right(50,0);
	    chassis_set_speed( 0);
	    osDelay(100);
		patrol_line_set_pid_kp_kd(&pid_value, 0.007,0.01);
		for(u32 i=0;i<=500;i++)
		{
			chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
			osDelay(1);
		}
		patrol_line_set_yaw_compensation(&pid_value, yaw);
		distance_patrol_move_distance(&DistancePatrol, 7000,400,6000);
		distance_patrol_set_kp_ki_kd(&DistancePatrol, 0.006,0.08,0.08);
		distance_patrol_set_max_output(&DistancePatrol, 350);
		patrol_line_set_pid_kp_kd(&pid_value, 0.02,0.015);
		patrol_line_set_pid_kp_kd_yaw(&pid_value, 5,50);
		while(!(IF_UPHILL))
		{
			chassis_patrol_distance_protect(200,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
			osDelay(1);
		}
		patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.003);
		distance_patrol_encoder_init(&DistancePatrol);
		while(!distance_patrol_run_right_distance(&DistancePatrol, 4000))
		{
			chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
			osDelay(1);
		}
		chassis_enable();
        while(flag_Device.Color_code_left!=Online)
		{
			chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
			osDelay(1);
		}
		osDelay(100);
		chassis_set_speed( 0);
		osDelay(100);
		Change_dir_Pid_Left(140,0);//大角度左转
		patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.01);
		for(u32 i=0;i<=500;i++)
		{
			chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
			osDelay(1);
		}
		/****************************************再次位置循迹****************************************/
		patrol_line_set_yaw_compensation(&pid_value, yaw);
		distance_patrol_move_distance(&DistancePatrol, 7000,400,6000);
		distance_patrol_set_kp_ki_kd(&DistancePatrol, 0.006,0.08,0.08);
		distance_patrol_set_max_output(&DistancePatrol, 350);
		patrol_line_set_pid_kp_kd(&pid_value, 0.005,0.015);
		patrol_line_set_pid_kp_kd_yaw(&pid_value, 5,50);
		while(flag_Device.Color_code_front_right!=Online)
		{
			chassis_patrol_distance_protect(200,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
			osDelay(1);
		}
		chassis_set_speed( 0);
		osDelay(100);
		Change_dir_Pid_Right(40,0);//小角度右转
	}
}



//void Robot_HomePoint1_to_door4(void)
//{
//	flag.Direction = 1;//将行驶方向改为回家方向
//	patrol_line_set_pid_kp_kd(&pid_value, 0.007,0.01);
//	while(flag_Device.Color_code_left!=Online)
//	{
//		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
//		osDelay(1);
//	}
//	while(flag_Device.Color_code_left==Online)
//	{
//		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
//		osDelay(1);
//	}
//	while(flag_Device.Color_code_left!=Online)
//	{
//		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
//		osDelay(1);
//	}
//	chassis_set_speed( 0);
//	osDelay(100);
//	Change_dir_Pid_Left(90,0);
//	/****************************************识别4号门****************************************/
//	loc.door = 4;
//	bsp_camera_start_rx();//开启摄像头串口接收中断
//	patrol_line_set_pid_kp_kd(&pid_value, 0.007,0.003);
//	while(!(IF_UPHILL))
//	{
//		if(door_is_closed(DOOR_ID_4, DOOR_DIR_BACK))//1号门禁行
//		{
//			chassis_set_speed( 0);
//			osDelay(100);
//			break;
//		}else{
//			chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
//		}
//		osDelay(1);
//	}
//	loc.door = 0;
//}


//void Robot_door4_to_door2(void)
//{
//	patrol_line_set_yaw_compensation(&pid_value, yaw);
//	patrol_line_set_pid_kp_kd_yaw(&pid_value, 10,20);
//	while(flag_Device.Color_code_right!=Online)
//	{	
//		chassis_patrol_back( 150,-patrol_line_yaw_compensation(&pid_value, yaw));
//		osDelay(1);
//	}
//	chassis_set_speed( 0);
//	Change_dir_Pid_Left(60,0);
//	patrol_line_set_pid_kp_kd(&pid_value, 0.007,0.01);
//	for(u32 i=0;i<=1000;i++)
//	{
//		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
//		osDelay(1);
//	}
//	patrol_line_set_yaw_compensation(&pid_value, yaw);
//	distance_patrol_move_distance(&DistancePatrol, 7000,400,6000);
//	distance_patrol_set_kp_ki_kd(&DistancePatrol, 0.006,0.08,0.08);
//	distance_patrol_set_max_output(&DistancePatrol, 350);
//	patrol_line_set_pid_kp_kd(&pid_value, 0.02,0.015);
//    patrol_line_set_pid_kp_kd_yaw(&pid_value, 5,50);
//	while(flag_Device.Color_code_right!=Online)
//	{
//		chassis_patrol_distance_protect(200,distance_patrol_get_calculate_speed(&DistancePatrol),patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
//		osDelay(1);
//	}
//	/****************************************识别2号门****************************************/
//	loc.door = 2;
//	bsp_camera_start_rx();//开启摄像头串口接收中断
//	patrol_line_set_pid_kp_kd(&pid_value, 0.007,0.003);
//	while(!(IF_UPHILL))
//	{
//		if(flag.Back_doorClose[2] == 1)//1号门禁行
//		{
//			chassis_set_speed( 0);
//			osDelay(100);
//			break;
//		}
//		else{
//			chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
//		}
//		osDelay(1);
//	}
//	loc.door = 0;
//}


//void Robot_door2_to_door3_to_HomePoint2(void)
//{
//	patrol_line_set_yaw_compensation(&pid_value, yaw);
//	patrol_line_set_pid_kp_kd_yaw(&pid_value, 10,20);
//	while(flag_Device.Color_code_right!=Online)
//	{	
//		chassis_patrol_back( 150,-patrol_line_yaw_compensation(&pid_value, yaw));
//		osDelay(1);
//	}
//	chassis_set_speed( 0);
//	Change_dir_Pid_Right(100,0);
//	patrol_line_set_pid_kp_kd(&pid_value, 0.007,0.01);
//	while(!(IF_UPHILL))
//	{
//		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
//		osDelay(1);
//	}
//	patrol_line_set_pid_kp_kd(&pid_value, 0.007,0.01);
//	for(u32 i=0;i<=600;i++)
//	{
//		chassis_patrol_distance_protect(200,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
//		osDelay(2);
//	}
//	while(flag_Device.Color_code_right!=Online)
//	{
//		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
//		osDelay(1);
//	}
//	chassis_set_speed( 0);
//	osDelay(100);
//	Change_dir_Pid_Left(130,0);
//	patrol_line_set_pid_kp_kd(&pid_value, 0.008,0.015);
//	for(u32 i=0;i<=500;i++)
//	{
//		chassis_patrol_distance_protect(300,0,patrol_line_calcu_compensation(&pid_value),patrol_line_ad_compensation(&pid_value));
//		osDelay(1);
//	}
//	/****************************************再次位置循迹****************************************/
//	patrol_line_set_yaw_compensation(&pid_value, yaw);
//	distance_patrol_set_max_output(&DistancePatrol, 350);
//	patrol_line_set_pid_kp_kd(&pid_value, 0.007,0.015);
//    patrol_line_set_pid_kp_kd_yaw(&pid_value, 6,50);
//	while(flag_Device.Color_code_front_right!=Online)
//	{
//			chassis_patrol_distance_protect(360,0,patrol_line_yaw_compensation(&pid_value, yaw),patrol_line_calcu_compensation(&pid_value));//进行位置pid巡线
//		osDelay(1);
//	}
//	chassis_set_speed( 0);
//    osDelay(100);
//	Change_dir_Pid_Right(40,0);//小角度右转
//}



void __Hardware_Handler(void)
{
	Gyroscope_Start();
	Check_Device();
}

void LCD_Show(void)
{
	menu_refresh();
}




/*测试函数（仅调试）
 *@灰度循迹
 *@要用哪个();解注释
 */
void PatrolLineTest(void)
{
	Robot_Start();
	Robot_loc_1_to_loc_2();
	Robot_loc_2_to_loc_4();
	Robot_loc_4_to_loc_3();	
	Robot_loc_3to_door1();
	if(door_is_closed(DOOR_ID_1, DOOR_DIR_GO))
	{
		Robot_door1_to_door2();
		if(door_is_closed(DOOR_ID_2, DOOR_DIR_GO))
		{
			Robot_door2_to_door4_to_loc_5();
		}else{
			Robot_door2_to_loc_5();
		}
	}else{
		Robot_door1_to_loc_5();
	}
	Robot_loc_5to_loc7();
	Robot_loc_7_to_loc_8();
	Robot_loc_8_to_HomePoint1();
    Robot_HomePoint1_to_HomePoint2();
	Robot_HomePoint2_to_Home();
	//第二轮
	Robot_Start_2();
	Robot_loc_1_to_loc_2();
	Robot_loc_2_to_loc_4();
	Robot_loc_4_to_loc_3();	
	Robot_loc_3to_door1();
	if(door_is_closed(DOOR_ID_1, DOOR_DIR_GO))
	{
		Robot_door1_to_door2();
		if(door_is_closed(DOOR_ID_2, DOOR_DIR_GO))
		{
			Robot_door2_to_door4_to_loc_5();
		}else{
			Robot_door2_to_loc_5();
		}
	}else{
		Robot_door1_to_loc_5();
	}
	Robot_loc_5to_loc7();
	Robot_loc_7_to_loc_8();
	Robot_loc_8_to_HomePoint1();
    Robot_HomePoint1_to_HomePoint2();
	Robot_HomePoint2_to_Home();
	while(1)
	{
		chassis_set_speed( 0);
		osDelay(100);
	}
}

