/*
 * Light_HAL.c
 *
 *  Created on: Apr 13, 2026
 *      Author: nsist
 */
#include "cmsis_os2.h"
#include "stm32h7xx_nucleo.h"
#include "../include/Light_Control.h"
#include "../HeadLamp/RTE/RTE.h"

#define POS_DRL_TIM (&htim3)
#define POS_DRL_CH (TIM_CHANNEL_3)
#define LB_TIM (&htim8)
#define LB_CH (TIM_CHANNEL_2)
#define HB_TIM (&htim8)
#define HB_CH (TIM_CHANNEL_2)
#define TI_HA_TIM (&htim8)
#define TI_HA_CH (TIM_CHANNEL_2)
#define FOG_TIM (&htim3)
#define FOG_CH (TIM_CHANNEL_3)
#define MAP_DUTY_CYCLE(x) (x*10)
#define OUTPUT_LED(x1, x2, x3) __HAL_TIM_SET_COMPARE(x1, x2, MAP_DUTY_CYCLE(x3))

typedef struct
{
	TIM_HandleTypeDef light_func_TIM;
	uint8_t light_func_CHTIM;
	uint8_t duty_cycle;
}s_Light_Func_Desc;

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim8;

s_Light_Func_Desc light_func[7] = {0};
bool isInitDone = false;


/*
*	Need these lines for testing
void Set_LED(Led_TypeDef LED, uint8_t status)
{
	if (status == 1)
	{
		BSP_LED_On(LED);
	}
	else
	{
		BSP_LED_Off(LED);
	}

}
uint8_t Get_LED(Led_TypeDef LED)
{
	uint8_t state;

	if(BSP_LED_GetState(LED) == GPIO_PIN_RESET)
	{
		state = 0;
	}
	else
	{
		state = 1;
	}

	return state;
}
*/

/* Set the LEDs for the POS_DRL_FOG functionalities  */
void Set_Light_Func_POS_DRL_Command(s_Light_Pixel_PWM_Command pixel_duties)
{
}

/* Set the LEDs for the Low Beam functionality */
void Set_Light_Func_LowBeam_Command(uint8_t duty_cycle)
{
}

/* Set the LEDs for the High Beam functionality */
void Set_Light_Func_HighBeam_Command(s_Light_Pixel_PWM_Command pixel_duties)
{
}

/* Set the LEDs for the Turn Indicator and Hazard functionalities */
void Set_Light_Func_TI_Hazard_Command(s_Light_Pixel_PWM_Command pixel_duties)
{
}

/* Set the LEDs for the FOG functionality */
void Set_Light_Func_FOG_Command(uint8_t duty_cycle)
{
}

void Init_Light_Control()
{
//	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
//	HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_2);
	isInitDone = true;
}

void Run_Light_Control_Main_10ms()
{
	RTE_Write_Light_Control_Init_Done(isInitDone);
}
