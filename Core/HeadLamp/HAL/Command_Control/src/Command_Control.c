/*
 * Command_Control.c
 *
 *  Created on: Apr 27, 2026
 *      Author: nsist
 */


#include "../include/Command_Control.h"
#include "../HeadLamp/RTE/RTE.h"

uint8_t Light_Rotor_Switch_Value = 0U;
uint8_t Level_Pot_Value = 0U;
bool R_Blinker_Button_Value = 0U;
bool L_Blinker_Button_Value = 0U;
bool Hazzard_Button_Value = 0U;
bool HighBeam_Button_Value = 0U;

uint8_t Rotor_Switch_Pin = 0U;
uint8_t Leveling_Pin = 0U;
uint8_t R_Blinker_Button_Pin = 0U;
uint8_t L_Blinker_Button_Pin = 0U;
uint8_t Hazzard_Button_Pin = 0U;
uint8_t HighBeam_Button_Pin = 0U;
uint16_t HighBeam_Pot_Value = 0U;

void Get_HighBeam_Pot_Value()
{
	//read pin
}

/* Read the value for the Light_Rotor_Switch from the PIN */
void Get_Light_Switch_Value()
{
	//read pin
}

/* Read the value for the Leveling_Command from the pin*/
void Get_Leveling_Value()
{
	//read pin

}

/* Read the value for the R_Blinker_Button from the pin*/
void Get_R_Blinker_Value()
{
	//read pin

}

/* Read the value for the R_Blinker_Button from the pin*/
void Get_L_Blinker_Value()
{
	//read pin

}

/* Read the value for the Hazzard_Button from the pin*/
void Get_Hazzard_Value()
{
	//read pin

}

/* Read the value for the HighBeam_Button from the pin*/
void Get_HighBeam_Value()
{
	//read pin

}

/*Send the Light_Rotor_Switch value read from the pin */
void Write_Light_Switch_Value()
{
	RTE_Write_Light_Command_Rotary_Switch_Value(Light_Rotor_Switch_Value);
}

/*Send the Leveling value read from the pin */
void Write_Leveling_Pot_Value()
{
	RTE_Write_Leveling_Pot_Value(Level_Pot_Value);
}

void Write_R_Blinker_Button_Value()
{
	RTE_Write_R_Blinker_Button_Value(R_Blinker_Button_Value);
}

void Write_L_Blinker_Button_Value()
{
	RTE_Write_L_Blinker_Button_Value(L_Blinker_Button_Value);
}

void Write_Hazzard_Button_Value()
{
	RTE_Write_Hazzard_Button_Value(Hazzard_Button_Value);
}

void Write_HighBeam_Button_Value()
{
	RTE_Write_HighBeam_Button_Value(HighBeam_Button_Value);
}

void Write_HighBeam_Pot_Value()
{
	RTE_Write_HighBeam_Pot_Control_Value(HighBeam_Pot_Value);
}

void Init_Command_Control()
{
	/*Set the pin numbers for all the commands*/
}

void Run_Command_Control_Main_20ms()
{
	Get_Light_Switch_Value();
	Get_Leveling_Value();
	Get_L_Blinker_Value();
	Get_R_Blinker_Value();
	Get_Hazzard_Value();
	Get_HighBeam_Value();
	Get_HighBeam_Pot_Value();

	Write_Light_Switch_Value();
	Write_Leveling_Pot_Value();
	Write_L_Blinker_Button_Value();
	Write_R_Blinker_Button_Value();
	Write_Hazzard_Button_Value();
	Write_HighBeam_Button_Value();
	Write_HighBeam_Pot_Value();
}
