/*
 * RTE_Types.h
 *
 *  Created on: Apr 28, 2026
 *      Author: nsist
 */

#ifndef HEADLAMP_RTE_RTE_TYPES_H_
#define HEADLAMP_RTE_RTE_TYPES_H_

#include <stdbool.h>
#include "cmsis_os2.h"
#include "stm32h7xx_nucleo.h"

/* TYPEDEFS*/
typedef struct
{
	bool Off_State;
	bool Position_State;
	bool DRL_State;
	bool Auto_State;
	bool LowBeam_State;
	bool FogLight_State;
} s_Light_Func_Comm;

typedef struct
{
	bool L_Blink;
	bool R_Blink;
	bool Hazzard_Blink;
} s_Blink_Func_Comm;

typedef union{
	uint8_t byte;
	struct
	{
		uint8_t LED0:1;
		uint8_t LED1:1;
		uint8_t LED2:1;
		uint8_t LED3:1;
		uint8_t LED4:1;
		uint8_t LED5:1;
		uint8_t LED6:1;
		uint8_t LED7:1;
	}bit;
}u_Light_Pixel;

typedef struct
{
    u_Light_Pixel pixels_25;
    u_Light_Pixel pixels_50;
    u_Light_Pixel pixels_75;
    u_Light_Pixel pixels_100;
} s_Light_Pixel_PWM_Command;

#endif /* HEADLAMP_RTE_RTE_TYPES_H_ */
