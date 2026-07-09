/*
 * Light_HAL.h
 *
 *  Created on: Apr 13, 2026
 *      Author: nsist
 */

#ifndef HEADLAMP_HAL_LIGHT_CONTROL_INCLUDE_LIGHT_CONTROL_H_
#define HEADLAMP_HAL_LIGHT_CONTROL_INCLUDE_LIGHT_CONTROL_H_

#include "../HeadLamp/RTE/RTE_Types.h"

void Init_Light_Control();
void Run_Light_Control_Main_10ms();

void Set_Light_Func_POS_DRL_Command(u_Light_Pixel Light_Func, uint8_t duty_cycle);
void Set_Light_Func_LowBeam_Command(uint8_t duty_cycle);
void Set_Light_Func_HighBeam_Command(u_Light_Pixel pixel_states, uint8_t duty_cycle);
void Set_Light_Func_TI_Hazard_Command(u_Light_Pixel pixel_states, uint8_t duty_cycle);
void Set_Light_Func_FOG_Command(uint8_t duty_cycle);

void Set_LED(Led_TypeDef LED, uint8_t status);
uint8_t Get_LED(Led_TypeDef LED);


#endif /* HEADLAMP_HAL_LIGHT_CONTROL_INCLUDE_LIGHT_CONTROL_H_ */
