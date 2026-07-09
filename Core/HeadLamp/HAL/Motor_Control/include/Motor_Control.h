/*
 * Motor_Control.h
 *
 *  Created on: Apr 28, 2026
 *      Author: nsist
 */

#ifndef HEADLAMP_HAL_MOTOR_CONTROL_INCLUDE_MOTOR_CONTROL_H_
#define HEADLAMP_HAL_MOTOR_CONTROL_INCLUDE_MOTOR_CONTROL_H_

#include "../HeadLamp/RTE/RTE_Types.h"

void Init_Motor_Control();
void Run_Motor_Control_Main_20ms();
void Set_Motor_Position(uint8_t pos);

#endif /* HEADLAMP_HAL_MOTOR_CONTROL_INCLUDE_MOTOR_CONTROL_H_ */
