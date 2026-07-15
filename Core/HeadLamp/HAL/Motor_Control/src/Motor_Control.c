/*
 * Motor_Contro.c
 *
 *  Created on: Apr 28, 2026
 *      Author: nsist
 */

#include "../HeadLamp/RTE/RTE.h"
#include "../include/Motor_Control.h"

#define MOTOR_CONTROL_DIRECTION_DOWN                0U
#define MOTOR_CONTROL_DIRECTION_UP                  1U

uint8_t current_motor_pos = 0u;
uint8_t requested_motor_pos = 0u;
uint8_t motor_direction = MOTOR_CONTROL_DIRECTION_DOWN;

void Init_Motor_Control()
{
	current_motor_pos = 0u;
	requested_motor_pos = 0u;
	motor_direction = MOTOR_CONTROL_DIRECTION_DOWN;
	RTE_Write_Motor_Movement_Finished(true);
}

void Run_Motor_Control_Main_20ms()
{

}

void Move_Motor_Steps(uint8_t direction, uint8_t steps)
{
	motor_direction = direction;
    requested_motor_pos = steps;
    RTE_Write_Motor_Movement_Finished(false);
}
