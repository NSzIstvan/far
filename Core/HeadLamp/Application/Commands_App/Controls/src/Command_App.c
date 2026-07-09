/*
 * Command_App.c
 *
 *  Created on: Apr 27, 2026
 *      Author: nsist
 */
#include "../include/Command_App.h"
#include "../HeadLamp/RTE/RTE.h"

#define ON_STATE 1U
#define OFF_STATE 0U

typedef struct
{
	s_Light_Func_Comm Light_Func_Switch_Position;
	s_Blink_Func_Comm Blinker_Func;
	bool HB_State;
	uint16_t Leveling_Position;
} s_HL_Light_Func_Commands;

typedef enum
{
	Off_Value = 0,
	Position_Value,
	Auto_Value,
	LowBeam_Value,
	FogLight_Value
} enum_Light_Pos_Rot_Value;

typedef enum
{
	Blinker_L_Pos = 0,
	Blinker_R_Pos,
	Blinker_Hazzard,
	No_Blinker
} enum_Blinker_Positions;

s_HL_Light_Func_Commands HL_Light_Func_Commands = {OFF_STATE};
uint8_t old_Light_Rotary_Switch_Position = 0U;
s_Blink_Func_Comm old_Blink_Func = {OFF_STATE};

void Get_All_Func();
void Send_All_Func();
void Map_New_Light_Func_Switch_From_Rotor_Switch(uint8_t light_pos_param);
void Map_New_Blink_Commands(s_Blink_Func_Comm blinker_comm_param, enum_Blinker_Positions blinker_func);
void Reset_Light_Func_Commands();
void Reset_Blink_Func_Commands();
enum_Blinker_Positions Compare_Blinker_Func_Commands(s_Blink_Func_Comm s1, s_Blink_Func_Comm s2);

void Send_All_Func()
{
	RTE_Write_Light_Command_Switch_Position(HL_Light_Func_Commands.Light_Func_Switch_Position);
	RTE_Write_Blinker_Command_State(HL_Light_Func_Commands.Blinker_Func);
	RTE_Write_High_Beam_Command_State(HL_Light_Func_Commands.HB_State);
	RTE_Write_Leveling_Command_Value(HL_Light_Func_Commands.Leveling_Position);

}

void Get_All_Func()
{
	uint8_t new_Light_Rotary_Switch_Position = RTE_Read_Light_Command_Rotary_Switch_Value();
	if(new_Light_Rotary_Switch_Position != old_Light_Rotary_Switch_Position)
	{
		//reset all the commands get before
		Reset_Light_Func_Commands();
		Map_New_Light_Func_Switch_From_Rotor_Switch(new_Light_Rotary_Switch_Position);
		old_Light_Rotary_Switch_Position = new_Light_Rotary_Switch_Position;
	}

	s_Blink_Func_Comm new_Blink_Func = (s_Blink_Func_Comm){RTE_Read_R_Blinker_Button_Value(), RTE_Read_L_Blinker_Button_Value(), RTE_Read_Hazzard_Button_Value()} ;
	enum_Blinker_Positions new_Blinker_Func_Comparison = Compare_Blinker_Func_Commands(new_Blink_Func, old_Blink_Func);
	if(No_Blinker > new_Blinker_Func_Comparison)
	{
		Reset_Blink_Func_Commands();
		Map_New_Blink_Commands(new_Blink_Func, new_Blinker_Func_Comparison);
		old_Blink_Func = new_Blink_Func;
	}

	HL_Light_Func_Commands.HB_State = RTE_Read_HighBeam_Button_Value();
	HL_Light_Func_Commands.Leveling_Position = RTE_Read_Leveling_Pot_Value();
}

void Map_New_Light_Func_Switch_From_Rotor_Switch(uint8_t light_pos_param)
{
	switch((enum_Light_Pos_Rot_Value)light_pos_param)
	{
	case Position_Value:
		HL_Light_Func_Commands.Light_Func_Switch_Position.Position_State = ON_STATE;
	break;
	case Auto_Value:
		HL_Light_Func_Commands.Light_Func_Switch_Position.Auto_State = ON_STATE;
	break;
	case LowBeam_Value:
		HL_Light_Func_Commands.Light_Func_Switch_Position.LowBeam_State = ON_STATE;
	break;
	case FogLight_Value:
		HL_Light_Func_Commands.Light_Func_Switch_Position.FogLight_State = ON_STATE;
	break;
	case Off_Value:
		HL_Light_Func_Commands.Light_Func_Switch_Position.Off_State = ON_STATE;
		break;
	default:
		break;
	}
}

void Map_New_Blink_Commands(s_Blink_Func_Comm blinker_comm_param, enum_Blinker_Positions blinker_func)
{
	switch(blinker_func)
	{
	case Blinker_L_Pos:
		HL_Light_Func_Commands.Blinker_Func.L_Blink = blinker_comm_param.L_Blink;
		break;
	case Blinker_R_Pos:
		HL_Light_Func_Commands.Blinker_Func.R_Blink = blinker_comm_param.R_Blink;
		break;
	case Blinker_Hazzard:
		HL_Light_Func_Commands.Blinker_Func.Hazzard_Blink = blinker_comm_param.Hazzard_Blink;
		break;
	default:
		break;
	}
}

enum_Blinker_Positions Compare_Blinker_Func_Commands(s_Blink_Func_Comm s1, s_Blink_Func_Comm s2)
{
	uint8_t ret = No_Blinker;
	if(s1.L_Blink != s2.L_Blink)
	{
		ret = Blinker_L_Pos;
	}
	if(s1.R_Blink != s2.R_Blink)
	{
		ret = Blinker_R_Pos;
	}
	if(s1.Hazzard_Blink !=  s2.Hazzard_Blink)
	{
		ret = Blinker_Hazzard;
	}
	return ret;
}

void Reset_Light_Func_Commands()
{
	HL_Light_Func_Commands.Light_Func_Switch_Position = (s_Light_Func_Comm){OFF_STATE, OFF_STATE, OFF_STATE, OFF_STATE, OFF_STATE};
}

void Reset_Blink_Func_Commands()
{
	HL_Light_Func_Commands.Blinker_Func = (s_Blink_Func_Comm){OFF_STATE, OFF_STATE, OFF_STATE};
}

void Init_Command_App()
{
	Reset_Light_Func_Commands();
	Reset_Blink_Func_Commands();
	HL_Light_Func_Commands.HB_State = OFF_STATE;
	HL_Light_Func_Commands.Leveling_Position = 0xFFFF;
}

void Run_Command_App_Main_20ms()
{
	Get_All_Func();
	Send_All_Func();
}
