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

void Run_Light_App_Main_10ms()
{
	//Read all the Command_App inputs
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
