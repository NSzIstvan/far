/*
 * Fan_App.c
 *
 * Created on: Apr 28, 2026
 * Author: nsist
 */

#include "../include/Fan_App.h"
#include "../HeadLamp/RTE/RTE.h"

#include <stdbool.h>
#include <stdint.h>

/* ========================================================= */
/* Defines                                                   */
/* ========================================================= */

#define FAN_APP_FAN_SPEED_0                         0U
#define FAN_APP_FAN_SPEED_25                        25U
#define FAN_APP_FAN_SPEED_50                        50U
#define FAN_APP_FAN_SPEED_75                        75U
#define FAN_APP_FAN_SPEED_100                       100U

/*
 * Temperature is expected from Sensor_Control already mapped to [0, 150].
 */
#define FAN_APP_TEMP_MIN_VALUE                  0U
#define FAN_APP_TEMP_MAX_VALUE                  150U

/*
 * Temperature thresholds for the 5 fan speed steps.
 *
 * 0   - 29  -> 0%
 * 30  - 59  -> 25%
 * 60  - 89  -> 50%
 * 90  - 119 -> 75%
 * 120 - 150 -> 100%
 */
#define FAN_APP_TEMP_THRESHOLD_25_PERCENT        30U
#define FAN_APP_TEMP_THRESHOLD_50_PERCENT        60U
#define FAN_APP_TEMP_THRESHOLD_75_PERCENT        90U
#define FAN_APP_TEMP_THRESHOLD_100_PERCENT       120U

/*
 * When HighBeam was active for more than 10s, Light_App sends a request
 * to increase the fan speed. Fan_App increases the temperature-based
 * fan speed with one step.
 */
#define FAN_APP_HIGH_BEAM_SPEED_INCREASE_STEP   25U

/* ========================================================= */
/* Local Types                                               */
/* ========================================================= */

typedef struct
{
    /*
     * Temperature from Sensor_Control.
     *
     * Expected range:
     * 0   -> cold
     * 150 -> very hot
     */
    uint8_t temperature_value;

    /*
     * Request from Light_App.
     *
     * true  -> increase fan speed with one step
     * false -> fan speed is based only on temperature
     */
    bool increase_fan_speed_high_beam_request;
} s_Fan_App_Input_Data;

typedef struct
{
    uint8_t temperature_based_fan_speed;
    uint8_t requested_fan_speed;
} s_Fan_App_Output_Data;

/* ========================================================= */
/* Global Data                                               */
/* ========================================================= */

/*
 * Kept global because the old Fan_App already had this variable and it can be
 * useful for debug/watch window.
 */
uint8_t actual_fan_speed = FAN_APP_FAN_SPEED_0;

/* ========================================================= */
/* Local Data                                                */
/* ========================================================= */

static s_Fan_App_Input_Data fan_app_input_data;
static s_Fan_App_Output_Data fan_app_output_data;

static bool fan_app_is_initialized = false;

/* ========================================================= */
/* Local Function Prototypes                                 */
/* ========================================================= */

static void Fan_App_Reset_Input_Data(void);
static void Fan_App_Reset_Output_Data(void);
static void Fan_App_Reset_Internal_Data(void);

static void Fan_App_Read_Input_Data(void);
static void Fan_App_Calculate(void);
static void Fan_App_Transmit_Output_Data(void);

static uint8_t Fan_App_Limit_Temperature_Value(uint8_t temperature_value);
static uint8_t Fan_App_Calculate_Speed_From_Temperature(uint8_t temperature_value);
static uint8_t Fan_App_Increase_Speed_One_Step(uint8_t fan_speed);

/* ========================================================= */
/* Local Functions                                           */
/* ========================================================= */

static void Fan_App_Reset_Input_Data(void)
{
    fan_app_input_data.temperature_value = FAN_APP_TEMP_MIN_VALUE;
    fan_app_input_data.increase_fan_speed_high_beam_request = false;
}

static void Fan_App_Reset_Output_Data(void)
{
    fan_app_output_data.temperature_based_fan_speed = FAN_APP_FAN_SPEED_0;
    fan_app_output_data.requested_fan_speed = FAN_APP_FAN_SPEED_0;
}

static void Fan_App_Reset_Internal_Data(void)
{
    actual_fan_speed = FAN_APP_FAN_SPEED_0;
    fan_app_is_initialized = false;
}

static void Fan_App_Read_Input_Data(void)
{
    /*
     * Sensor_Control -> Fan_App:
     * temperature mapped to [0, 150].
     */
    fan_app_input_data.temperature_value = RTE_Read_Sensor_Ambiental_Temp();

    /*
     * Light_App -> Fan_App:
     * increase fan speed request when HighBeam is active for more than 10s.
     */
    fan_app_input_data.increase_fan_speed_high_beam_request =
        RTE_Read_Increase_FanSpeed_HighBeam();
}

static void Fan_App_Calculate(void)
{
    uint8_t local_temperature_value = FAN_APP_TEMP_MIN_VALUE;

    local_temperature_value =
        Fan_App_Limit_Temperature_Value(fan_app_input_data.temperature_value);

    fan_app_output_data.temperature_based_fan_speed =
        Fan_App_Calculate_Speed_From_Temperature(local_temperature_value);

    fan_app_output_data.requested_fan_speed =
        fan_app_output_data.temperature_based_fan_speed;

    if (true == fan_app_input_data.increase_fan_speed_high_beam_request)
    {
        fan_app_output_data.requested_fan_speed =
            Fan_App_Increase_Speed_One_Step(fan_app_output_data.requested_fan_speed);
    }

    actual_fan_speed = fan_app_output_data.requested_fan_speed;
}

static void Fan_App_Transmit_Output_Data(void)
{
    /*
     * Fan_App -> Fan_Control:
     * requested speed is one of: 0, 25, 50, 75, 100.
     */
    RTE_Write_FanSpeed(fan_app_output_data.requested_fan_speed);
}

static uint8_t Fan_App_Limit_Temperature_Value(uint8_t temperature_value)
{
    uint8_t local_temperature_value = temperature_value;

    if (local_temperature_value >= FAN_APP_TEMP_MAX_VALUE)
    {
        local_temperature_value = FAN_APP_TEMP_MAX_VALUE;
    }
    else
    {
        /* Temperature is already in range. */
    }

    return local_temperature_value;
}

static uint8_t Fan_App_Calculate_Speed_From_Temperature(uint8_t temperature_value)
{
    uint8_t local_fan_speed = FAN_APP_FAN_SPEED_0;

    if (temperature_value >= FAN_APP_TEMP_THRESHOLD_100_PERCENT)
    {
        local_fan_speed = FAN_APP_FAN_SPEED_100;
    }
    else if (temperature_value >= FAN_APP_TEMP_THRESHOLD_75_PERCENT)
    {
        local_fan_speed = FAN_APP_FAN_SPEED_75;
    }
    else if (temperature_value >= FAN_APP_TEMP_THRESHOLD_50_PERCENT)
    {
        local_fan_speed = FAN_APP_FAN_SPEED_50;
    }
    else if (temperature_value >= FAN_APP_TEMP_THRESHOLD_25_PERCENT)
    {
        local_fan_speed = FAN_APP_FAN_SPEED_25;
    }
    else
    {
        local_fan_speed = FAN_APP_FAN_SPEED_0;
    }

    return local_fan_speed;
}

static uint8_t Fan_App_Increase_Speed_One_Step(uint8_t fan_speed)
{
    uint8_t local_fan_speed = fan_speed;

	if(local_fan_speed > FAN_APP_FAN_SPEED_100)
	{
		local_fan_speed = FAN_APP_FAN_SPEED_100;
	}
	else
	{
		local_fan_speed += FAN_APP_HIGH_BEAM_SPEED_INCREASE_STEP;
	}

    return local_fan_speed;
}

/* ========================================================= */
/* Global Functions                                          */
/* ========================================================= */

void Init_Fan_App(void)
{
    Fan_App_Reset_Input_Data();
    Fan_App_Reset_Output_Data();
    Fan_App_Reset_Internal_Data();

    fan_app_is_initialized = true;

    Fan_App_Transmit_Output_Data();
}

void Run_Fan_App_Main_50ms(void)
{
    if (false == fan_app_is_initialized)
    {
        Init_Fan_App();
    }

    if (true == fan_app_is_initialized)
    {
        Fan_App_Read_Input_Data();
        Fan_App_Calculate();
        Fan_App_Transmit_Output_Data();
    }
}