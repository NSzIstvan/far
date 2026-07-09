/*
 * Fan_Control.c
 *
 *  Created on: Apr 29, 2026
 *      Author: nsist
 */

#include "../HeadLamp/RTE/RTE.h"
#include "../include/Fan_Control.h"

uint8_t requested_fan_speed = 0;

uint8_t Get_Reqested_Fan_Speed();
void Set_Output_Fan_Speed(uint8_t speed);



uint8_t Get_Reqested_Fan_Speed()
{
	return RTE_Read_FanSpeed();
}

void Set_Output_Fan_Speed(uint8_t speed)
{
	//set Pin value = speed
}

void Init_Fan_Control()
{
	requested_fan_speed = 0;
}

void Run_Fan_Control_Main_50ms()
{
	requested_fan_speed = Get_Reqested_Fan_Speed();
}
