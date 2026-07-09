/*
 * Motor_Contro.c
 *
 *  Created on: Apr 28, 2026
 *      Author: nsist
 */

#include "../HeadLamp/RTE/RTE.h"
#include "../include/Motor_Control.h"

uint16_t current_motor_pos = 0u;
uint16_t requested_motor_pos = 0u;

void Init_Motor_Control()
{
	current_motor_pos = 0u;
	requested_motor_pos = 0u;
}

void Run_Motor_Control_Main_20ms()
{

}

void Set_Motor_Position(uint8_t pos)
{
	requested_motor_pos = pos;
}
