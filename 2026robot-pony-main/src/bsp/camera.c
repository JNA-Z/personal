#include "camera.h"

uint16_t camera_start_state=0;                      //0:还未开始接收；1：开始接收
uint8_t CameraRxBuffer[10];                         //摄像头接收数组缓存
struct Camera_flag
{
	uint8_t location;                               //机器人所在位置，1为1号门，类推
	uint8_t doorClose[4];                           //门禁标志位，置1代表对应门号禁行，doorClose[0]对应1号门
}Camera;





