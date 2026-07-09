/*
 * Light.c
 *
 *  Created on: Apr 13, 2026
 *      Author: nsist
 */
#include "../HeadLamp/RTE/RTE.h"
#include "../include/Light_App.h"

uint8_t POS_test = 0;
uint8_t FOG_test = 0;
bool seting_test = false;

#define LIGHT_ON 1U
#define LIGHT_OFF 0U

typedef union
{
	struct{
	uint8_t drl_light:1;
	uint8_t pos_light:1;
	uint8_t low_light:1;
	uint8_t high_light:1;
	uint8_t fog_light:1;
	} bits;
	uint8_t all_lights;
}u_Light_Functions;

typedef union
{
	struct {
		uint8_t l_ti : 1;
		uint8_t r_ti : 1;
		uint8_t haz : 1;
	}bits;
	uint8_t all_lights;
}u_Blink_Light_Functions;


u_Light_Functions light_functions = { .all_lights = LIGHT_OFF};
u_Blink_Light_Functions blink_functions = { .all_lights = LIGHT_OFF };
bool high_beam_function = LIGHT_OFF;
bool lights_auto = false;

void Toggle_LED(Led_TypeDef LED)
{
	if (RTE_Call_Get_LED(LED) == 1)
	{
		RTE_Call_Set_LED(LED, 0);
	}
	else
	{
		RTE_Call_Set_LED(LED, 1);
	}
}

void LED_operation()
{
	static uint32_t tick = 0U;

	  if ((tick % 1000) == 0)
	  {
		  Toggle_LED(LED_GREEN);
	  }
	  else if ((tick % 1500) == 0U)
	  {
		  Toggle_LED(LED_RED);
	  }

	  if(tick > 12000)
	  {
		  tick = 0;
	  }

	  tick+=20;
}

static void Get_Light_Functions_Mapped(s_Light_Func_Comm light_funcs)
{
	static s_Light_Func_Comm command_light_function = { LIGHT_OFF };
	if (light_funcs != command_light_function)
	{
		if (true == light_funcs.Off_State) 
		{
			light_functions.all_lights = LIGHT_OFF;
		}
		else if (true == light_funcs.Auto_State)
		{
			lights_auto = true;
		}
		else
		{
			light_functions.bits.drl_light = light_funcs.DRL_State;
			light_functions.bits.pos_light = light_funcs.Position_State;
			light_functions.bits.low_light = light_funcs.LowBeam_State;
			light_functions.bits.fog_light = light_funcs.FogLight_State;
		}

		command_light_function = light_funcs;
	}
}

static void Get_Blink_Functions_Mapped(s_Blink_Func_Comm blink_funcs)
{
	static s_Blink_Func_Comm old_blink_funcs = { LIGHT_OFF };

	if (old_blink_funcs != blink_funcs)
	{
		blink_functions.bits.l_ti = blink_funcs.L_Blink;
		blink_functions.bits.r_ti = blink_funcs.R_Blink;
		blink_functions.bits.haz = blink_funcs.Hazzard_Blink;

		old_blink_funcs = blink_funcs;
	}
}

static void Set_Blink_Functions_Off()
{

}

static void Set_Light_Functions_Off()
{

}

void Run_Light_App_Main_10ms()
{
	//Read all the Command_App inputs
	// 
	//pos,drl,lb,auto,off
	s_Light_Func_Comm local_light_functions = RTE_Read_Light_Command_Switch_Position();
	Get_Light_Functions_Mapped(local_light_functions);
	//ti,haz
	u_Blink_Light_Functions local_blinker_functions = RTE_Read_Blinker_Command_State();
	Get_Blink_Functions_Mapped(local_blinker_functions);
	//high beam
	high_beam_function = RTE_Read_HighBeam_Command_State();


	//Analyze and write to Light_Control the light functions

	if (light_functions.all_lights != LIGHT_OFF)
	{
		if (lights_auto)
		{
			//go and check the light sensor and set the shape to on
			// 
			//check the high_beam if active -> dynamic HB
		}
		else
		{
			//set all the light shapes according to their command
			//pos

			//drl

			//low beam

			//fog

			//high beam
		}
	}
	else
	{
		Set_Light_Functions_Off();
		
	}

	if (blink_functions.all_lights != LIGHT_OFF)
	{
		//set all the blinking functions shapes according to their command
		//left

		//right

		//hazard
	}
	else
	{
		Set_Blink_Functions_Off();
	}

	u_Light_Pixel test_pixel = {0};
	if(seting_test == true)
	{
	Light_Functionality_POS_DRL_Command(test_pixel, POS_test);
	Light_Functionality_FOG_Command(FOG_test);
	seting_test = false;
	}
}

void Init_Light_App()
{
}
