/*
 * Fan_App.c
 *
 *  Created on: Apr 28, 2026
 *      Author: nsist
 */

#include "../include/Fan_App.h"
#include "../HeadLamp/RTE/RTE.h"

uint8_t actual_fan_speed = 0u;

void Init_Fan_App()
{
	actual_fan_speed = 0;
}

void Run_Fan_App_Main_50ms()
{
	RTE_Write_FanSpeed(actual_fan_speed);
}
