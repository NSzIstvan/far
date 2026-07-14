/*
 * RTE.c
 *
 *  Created on: Apr 13, 2026
 *      Author: nsist
 */

#include "RTE.h"

static bool isInit = false;
static uint8_t buffer_1 = 0;
static uint8_t buffer_2 = 0;
static uint8_t buffer_3 = 0;
static bool buffer_4 = false;
static bool buffer_5 = false;
static bool buffer_6 = false;
static bool buffer_7 = false;
static s_Light_Func_Comm buffer_8 = {0};
static uint16_t buffer_9 = 0;
static s_Blink_Func_Comm buffer_10 = {0};
static bool buffer_11 = false;
static uint8_t buffer_12 = 0;
static uint8_t buffer_13 = 0;
static uint8_t buffer_14 = 0;
static uint8_t buffer_15 = 0;
static bool buffer_16 = false;
static uint16_t buffer_17 = 0;
static uint16_t buffer_18 = 0;
static bool buffer_19 = false;
static bool buffer_20 = false;

void init()
{
	if(true != isInit)
	{
	//all Init functions from all the components
	//HAL component Init functions
		Init_Command_Control();
		Init_Sensor();
		Init_Light_Control();
		Init_Motor_Control();
		Init_Fan_Control();

	//Application components Init functions
		Init_Command_App();
		Init_Light_App();
		Init_Fan_App();
	}

	isInit = true;
}

void cyclic_task_10ms()
{
	Run_Light_App_Main_10ms();
	Run_Light_Control_Main_10ms();
}

void cyclic_task_50ms()
{
	Run_Fan_App_Main_50ms();
	Run_Fan_Control_Main_50ms();
}

void cyclic_task_20ms()
{
	if(false != isInit)
	{
		//Application Functions
		Run_Command_Control_Main_20ms();
		Run_Sensor_Main_20ms();

		Run_Command_App_Main_20ms();
		Run_Motor_Control_Main_20ms();

	}
}

uint8_t RTE_Read_Light_Control_LED_Status()
{
	return buffer_1;
}

void RTE_Write_Light_HAL_LED_Status(uint8_t val)
{
	buffer_1 = val;
}

void RTE_Call_Light_Control_To_HALL_Set_LED(Led_TypeDef LED, uint8_t status)
{
	Set_LED(LED, status);
}

uint8_t RTE_CALL_Light_Control_To_Hall_Get_LED(Led_TypeDef LED)
{
	return Get_LED(LED);
}

void RTE_Call_LightApp_LightControl_Light_Func_DRL_POS_Command(s_Light_Pixel_PWM_Command val)
{
	Set_Light_Func_POS_DRL_Command(val);
}

void RTE_Call_LightApp_LightControl_Light_Func_LowBeam_Command(uint8_t val)
{
	Set_Light_Func_LowBeam_Command(val);
}

void RTE_Call_LightApp_LightControl_Light_Func_HighBeam_Command(s_Light_Pixel_PWM_Command val)
{
	Set_Light_Func_HighBeam_Command(val1);
}

void RTE_Call_LightApp_LightControl_Light_Func_TI_Hazzard_Command(s_Light_Pixel_PWM_Command val)
{
	Set_Light_Func_TI_Hazard_Command(val1);
}

void RTE_Call_LightApp_LightControl_Light_Func_FOG_Command(uint8_t val1)
{
	Set_Light_Func_FOG_Command(val1);
}

void RTE_Write_CommandControl_CommandApp_Light_Command_Rotary_Switch_Value(uint8_t val)
{
	buffer_2 = val;
}

void RTE_Write_CommandControl_CommandApp_Leveling_Pot_Value(uint8_t val)
{
	buffer_3 = val;
}

void RTE_Write_CommandControl_CommandApp_R_Blinker_Button_Value(bool val)
{
	buffer_4 = val;
}

void RTE_Write_CommandControl_CommandApp_L_Blinker_Button_Value(bool val)
{
	buffer_5 = val;
}

void RTE_Write_CommandControl_CommandApp_Hazzard_Button_Value(bool val)
{
	buffer_6 = val;
}

void RTE_Write_CommandControl_CommandApp_HighBeamButon_Value(bool val)
{
	buffer_7 = val;
}

void RTE_Write_CommandApp_LightApp_Light_Command_Switch_Position(s_Light_Func_Comm val)
{
	buffer_8 = val;
}

uint8_t RTE_Read_CommandApp_CommandControl_Light_Command_Rotary_Switch_Value()
{
	return buffer_2;
}

uint8_t RTE_Read_CommandApp_Command_Control_Leveling_Pot_Value()
{
	return buffer_3;
}

bool RTE_Read_CommandApp_Command_Control_R_Blinker_Button_Value()
{
	return buffer_4;
}

bool RTE_Read_CommandApp_Command_Control_L_Blinker_Button_Value()
{
	return buffer_5;
}

bool RTE_Read_CommandApp_Command_Control_Hazzard_Button_Value()
{
	return buffer_6;
}

bool RTE_Read_CommandApp_Command_Control_HighBeamButon_Value()
{
	return buffer_7;
}

s_Light_Func_Comm RTE_Read_LightApp_CommandApp_Light_Command_Switch_Position()
{
	return buffer_8;
}

void RTE_Write_CommandApp_MotorApp_Leveling_Command_Value(uint16_t val)
{
	buffer_9 = val;
}

uint16_t RTE_Read_CommandApp_MotorApp_Leveling_Command_Value()
{
	return buffer_9;
}

void RTE_Write_CommandApp_LightApp_Blinker_Command_State(s_Blink_Func_Comm val)
{
	buffer_10 = val;
}

s_Blink_Func_Comm RTE_Read_LightApp_CommandApp_Blinker_Command_State()
{
	return buffer_10;
}

void RTE_Write_CommandApp_LightApp_HighBeam_Command(bool val)
{
	buffer_11 = val;
}

bool RTE_Read_LightApp_CommandApp_HighBeam_Command_State()
{
	return buffer_11;
}

void RTE_Write_Sensor_FanApp_Temperature_Read_Value(uint8_t val)
{
	buffer_12 = val;
}

uint8_t RTE_Read_FanApp_Sensor_Temperature_Read_Value()
{
	return buffer_12;
}

void RTE_Write_FanApp_LightApp_HeadLamp_Temp(uint8_t val)
{
	buffer_13 = val;
}

uint8_t RTE_Read_LightApp_FanApp_HeadLamp_Temp()
{
	return buffer_13;
}

void RTE_Write_Sensor_MotorApp_Sensor_Angle(uint8_t val)
{
	buffer_14 = val;

}

uint8_t RTE_Read_MotorApp_Sensor_Sensor_Angle()
{
	return buffer_14;
}

void RTE_Write_FanApp_FanControl_FanSpeed(uint8_t val)
{
	buffer_15 = val;
}

uint8_t RTE_Read_FanControl_FanApp_FanSpeed()
{
	return buffer_15;
}

void RTE_Call_MotorApp_MotorControl_Command_Position(uint8_t val)
{
	Set_Motor_Position(val);
}

void RTE_Write_LightControl_LightApp_InitDone(bool val)
{
	buffer_16 = val;
}

bool RTE_Read_LightControl_LightApp_InitDone()
{
	return buffer_16;
}

void RTE_Write_CommandApp_LightApp_HighBeam_Pot_Control(val)
{
	buffer_17 = val;
}

uint16_t RTE_Read_CommandApp_LightApp_HighBeam_Pot_Control()
{
	return buffer_17;
}

void RTE_Write_SensorControl_CommandApp_Ambiental_Luminosity(val)
{
	buffer_18 = val;
}

uint8_t RTE_Read_CommandApp_SensorControl_Ambiental_Luminosity()
{
	return buffer_18;
}

void RTE_Write_LightApp_MotorApp_Motor_Command_Auto(val)
{
	buffer_19 = val;
}

bool RTE_Read_LightApp_MotorApp_Motor_Command_Auto()
{
	return buffer_19;
}

void RTE_Write_LightApp_FanApp_Increase_FanSpeed_HighBeam(val)
{
	buffer_20 = val;
}

bool RTE_Read_LightApp_FanApp_Increase_FanSpeed_HighBeam()
{
	return buffer_20;
}