/*
 * Sensor.c
 *
 *  Created on: Apr 28, 2026
 *      Author: nsist
 */


#include "../HeadLamp/RTE/RTE.h"
#include "../include/Sensor.h"

uint8_t temperature_value_read = 0;
uint8_t angle_value_read = 0;
uint16_t HighBeam_Pot_Value = 0U;
uint16_t HighBeam_Pot_Pin = 0U;

uint8_t Read_Temp_Sensor_Value();
uint8_t Read_Angle_Sensor_Value();
uint8_t Map_Temp_Sensor_Value(uint8_t map_temp);
uint8_t Map_Angle_Sensor_Value(uint8_t map_angle);


void Get_HighBeam_Pot_Value(void)
{
    /* read ADC pin */
}

void Write_HighBeam_Pot_Value(void)
{
    RTE_Write_HighBeam_Pot_Control_Value(HighBeam_Pot_Value);
}

void Init_Sensor()
{
	temperature_value_read = 0;
	angle_value_read = 0;
	HighBeam_Pot_Value = 0U;
	HighBeam_Pot_Pin = 0U;
}

void Run_Sensor_Main_20ms()
{
	uint8_t new_temperature_value_read = Read_Temp_Sensor_Value();
	if (new_temperature_value_read != temperature_value_read)
	{
		temperature_value_read = new_temperature_value_read;
		RTE_Write_Sensor_Ambiental_Temp((uint8_t)Map_Temp_Sensor_Value(temperature_value_read));
	}

	uint8_t new_angle_value_read = Read_Angle_Sensor_Value();
	if(new_angle_value_read != angle_value_read)
	{
		angle_value_read = new_angle_value_read;
		RTE_Write_Leveling_Sensor_Angle((uint8_t)Map_Angle_Sensor_Value(angle_value_read));
	}
	Get_HighBeam_Pot_Value();
	Write_HighBeam_Pot_Value();
}

/* Read temperature sensor value from the PIN*/
uint8_t Read_Temp_Sensor_Value()
{
	uint8_t ret = 0;

	return ret;
}

/* Read angle sensor value from the PIN*/
uint8_t Read_Angle_Sensor_Value()
{
	uint8_t ret = 0;

		return ret;
}

uint8_t Map_Temp_Sensor_Value(uint8_t map_temp)
{
	uint8_t ret = 0;

		return ret;
}

uint8_t Map_Angle_Sensor_Value(uint8_t map_angle)
{
	uint8_t ret = 0;

		return ret;
}


