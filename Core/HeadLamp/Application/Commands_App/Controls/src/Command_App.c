/*
 * Command_App.c
 *
 * Created on: Apr 27, 2026
 * Author: nsist
 */

#include "../include/Command_App.h"
#include "../HeadLamp/RTE/RTE.h"

#include <stdbool.h>
#include <stdint.h>

/* ========================================================= */
/* Defines                                                   */
/* ========================================================= */

#define COMMAND_APP_OFF_STATE                    (false)
#define COMMAND_APP_ON_STATE                     (true)

#define COMMAND_APP_DEFAULT_HB_POT_VALUE         (0U)

/* ========================================================= */
/* Local Types                                               */
/* ========================================================= */

typedef enum
{
    COMMAND_APP_LIGHT_OFF_VALUE = 0U,
    COMMAND_APP_LIGHT_POSITION_VALUE,
    COMMAND_APP_LIGHT_LOW_BEAM_VALUE,
    COMMAND_APP_LIGHT_AUTO_VALUE,
    COMMAND_APP_LIGHT_FOG_VALUE
} e_Command_App_Light_Rotary_Value;

typedef struct
{
    uint8_t light_rotary_switch_position;

    bool high_beam_button_state;

    bool left_blinker_button_state;
    bool right_blinker_button_state;
    bool hazzard_button_state;

    uint16_t high_beam_pot_value;
} s_Command_App_Input_Data;

typedef struct
{
    s_Light_Func_Comm light_func_switch_position;
    s_Blink_Func_Comm blinker_func;

    bool high_beam_state;

    uint16_t high_beam_pot_control;
} s_Command_App_Output_Data;

/* ========================================================= */
/* Local Data                                                */
/* ========================================================= */

static s_Command_App_Input_Data command_app_input_data;
static s_Command_App_Output_Data command_app_output_data;

/* ========================================================= */
/* Local Function Prototypes                                 */
/* ========================================================= */

static void Command_App_Reset_Input_Data(void);
static void Command_App_Reset_Output_Data(void);

static void Command_App_Read_Input_Data(void);
static void Command_App_Calculate(void);
static void Command_App_Transmit_Output_Data(void);

static void Command_App_Calculate_Light_Functions(void);
static void Command_App_Calculate_Blinker_Functions(void);
static void Command_App_Calculate_High_Beam_Function(void);
static void Command_App_Calculate_High_Beam_Pot_Command(void);

/* ========================================================= */
/* Local Functions                                           */
/* ========================================================= */

static void Command_App_Reset_Input_Data(void)
{
    command_app_input_data.light_rotary_switch_position = COMMAND_APP_LIGHT_OFF_VALUE;

    command_app_input_data.high_beam_button_state = COMMAND_APP_OFF_STATE;

    command_app_input_data.left_blinker_button_state = COMMAND_APP_OFF_STATE;
    command_app_input_data.right_blinker_button_state = COMMAND_APP_OFF_STATE;
    command_app_input_data.hazzard_button_state = COMMAND_APP_OFF_STATE;

    command_app_input_data.high_beam_pot_value = COMMAND_APP_DEFAULT_HB_POT_VALUE;
}

static void Command_App_Reset_Output_Data(void)
{
    command_app_output_data.light_func_switch_position.Off_State = COMMAND_APP_ON_STATE;
    command_app_output_data.light_func_switch_position.Position_State = COMMAND_APP_OFF_STATE;
    command_app_output_data.light_func_switch_position.DRL_State = COMMAND_APP_OFF_STATE;
    command_app_output_data.light_func_switch_position.Auto_State = COMMAND_APP_OFF_STATE;
    command_app_output_data.light_func_switch_position.LowBeam_State = COMMAND_APP_OFF_STATE;
    command_app_output_data.light_func_switch_position.FogLight_State = COMMAND_APP_OFF_STATE;
	
    command_app_output_data.high_beam_state = COMMAND_APP_OFF_STATE;

    command_app_output_data.blinker_func.L_Blink = COMMAND_APP_OFF_STATE;
    command_app_output_data.blinker_func.R_Blink = COMMAND_APP_OFF_STATE;
    command_app_output_data.blinker_func.Hazzard_Blink = COMMAND_APP_OFF_STATE;

    command_app_output_data.high_beam_pot_control = COMMAND_APP_DEFAULT_HB_POT_VALUE;
}

static void Command_App_Read_Input_Data(void)
{
    command_app_input_data.light_rotary_switch_position =
        RTE_Read_Light_Command_Rotary_Switch_Value();

    command_app_input_data.high_beam_button_state =
        RTE_Read_HighBeam_Button_Value();

    command_app_input_data.left_blinker_button_state =
        RTE_Read_L_Blinker_Button_Value();

    command_app_input_data.right_blinker_button_state =
        RTE_Read_R_Blinker_Button_Value();

    command_app_input_data.hazzard_button_state =
        RTE_Read_Hazzard_Button_Value();

    command_app_input_data.high_beam_pot_value = RTE_Read_HighBeam_Pot_Control_Level();
}

static void Command_App_Calculate(void)
{
    Command_App_Reset_Output_Data();

    Command_App_Calculate_Light_Functions();
    Command_App_Calculate_Blinker_Functions();
    Command_App_Calculate_High_Beam_Function();
    Command_App_Calculate_High_Beam_Pot_Command();
}

static void Command_App_Transmit_Output_Data(void)
{
    RTE_Write_Light_Command_Switch_Position(command_app_output_data.light_func_switch_position);

    RTE_Write_Blinker_Command_State(command_app_output_data.blinker_func);

    RTE_Write_High_Beam_Command_State(command_app_output_data.high_beam_state);

    RTE_Write_HighBeam_Pot_Control(command_app_output_data.high_beam_pot_control);
}

static void Command_App_Calculate_Light_Functions(void)
{
    /*
     * Rotary switch mapping according to requirements:
     *
     * 0 -> OFF
     * 1 -> POS
     * 2 -> LOW BEAM
     * 3 -> AUTO
     * 4 -> FOG
     *
     * DRL is not directly mapped from Command_App because the current
     * requirement lists only: off, pos, lowbeam, auto, fog.
     */
    switch ((e_Command_App_Light_Rotary_Value)command_app_input_data.light_rotary_switch_position)
    {
        case COMMAND_APP_LIGHT_POSITION_VALUE:
        {
            command_app_output_data.light_func_switch_position.Off_State = COMMAND_APP_OFF_STATE;
            command_app_output_data.light_func_switch_position.Position_State = COMMAND_APP_ON_STATE;
            break;
        }

        case COMMAND_APP_LIGHT_LOW_BEAM_VALUE:
        {
            command_app_output_data.light_func_switch_position.Off_State = COMMAND_APP_OFF_STATE;
            command_app_output_data.light_func_switch_position.LowBeam_State = COMMAND_APP_ON_STATE;
            break;
        }

        case COMMAND_APP_LIGHT_AUTO_VALUE:
        {
            command_app_output_data.light_func_switch_position.Off_State = COMMAND_APP_OFF_STATE;
            command_app_output_data.light_func_switch_position.Auto_State = COMMAND_APP_ON_STATE;
            break;
        }

        case COMMAND_APP_LIGHT_FOG_VALUE:
        {
            command_app_output_data.light_func_switch_position.Off_State = COMMAND_APP_OFF_STATE;
            command_app_output_data.light_func_switch_position.FogLight_State = COMMAND_APP_ON_STATE;
            break;
        }

        case COMMAND_APP_LIGHT_OFF_VALUE:
        default:
        {
            command_app_output_data.light_func_switch_position.Off_State = COMMAND_APP_ON_STATE;
            break;
        }
    }
}

static void Command_App_Calculate_Blinker_Functions(void)
{
    /*
     * Priority:
     * 1. Hazzard
     * 2. Left indicator
     * 3. Right indicator
     */
    if (COMMAND_APP_ON_STATE == command_app_input_data.hazzard_button_state)
    {
        command_app_output_data.blinker_func.Hazzard_Blink = COMMAND_APP_ON_STATE;
    }
    else if (COMMAND_APP_ON_STATE == command_app_input_data.left_blinker_button_state)
    {
        command_app_output_data.blinker_func.L_Blink = COMMAND_APP_ON_STATE;
    }
    else if (COMMAND_APP_ON_STATE == command_app_input_data.right_blinker_button_state)
    {
        command_app_output_data.blinker_func.R_Blink = COMMAND_APP_ON_STATE;
    }
    else
    {
        /* Blinker output remains OFF. */
    }
}

static void Command_App_Calculate_High_Beam_Function(void)
{
    command_app_output_data.high_beam_state =
        command_app_input_data.high_beam_button_state;
}

static void Command_App_Calculate_High_Beam_Pot_Command(void)
{
    /*
     * HighBeam potentiometer value is forwarded to Light_App.
     * Light_App will use this value in Auto shape to move HighBeam LEDs 2 by 2.
     */
    command_app_output_data.high_beam_pot_control =
        command_app_input_data.high_beam_pot_value;
}

/* ========================================================= */
/* Global Functions                                          */
/* ========================================================= */

void Init_Command_App(void)
{
    Command_App_Reset_Input_Data();
    Command_App_Reset_Output_Data();

    Command_App_Transmit_Output_Data();
}

void Run_Command_App_Main_20ms(void)
{
    Command_App_Read_Input_Data();
    Command_App_Calculate();
    Command_App_Transmit_Output_Data();
}