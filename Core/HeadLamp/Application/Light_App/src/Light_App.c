/*
 * Light_App.c
 *
 * Created on: Apr 13, 2026
 * Author: nsist
 */

#include "../HeadLamp/RTE/RTE.h"
#include "../include/Light_App.h"
#include "../include/Light_animation.h"
#include "../include/Light_scrolling_ind.h"

#include <stdbool.h>
#include <stdint.h>

/* ========================================================= */
/* Defines                                                   */
/* ========================================================= */

#define LIGHT_APP_OFF_STATE                     0U
#define LIGHT_APP_ON_STATE                      1U

#define LIGHT_APP_TASK_TIME_MS                  10U

#define LIGHT_APP_DUTY_OFF                      0U
#define LIGHT_APP_DUTY_LOW_BEAM_ON              100U
#define LIGHT_APP_DUTY_FOG_ON                   100U

#define LIGHT_APP_POS_DRL_LED_NUMBER            8U
#define LIGHT_APP_HIGH_BEAM_LED_NUMBER          8U
#define LIGHT_APP_TI_LED_NUMBER                 8U

#define LIGHT_APP_OFF_TIMEOUT_MS                30000U
#define LIGHT_APP_HIGH_BEAM_FAN_TIME_MS         10000U

#define LIGHT_APP_LUMINOSITY_DARK_LEVEL         30U
#define LIGHT_APP_LUMINOSITY_LIGHT_LEVEL        70U

#define LIGHT_APP_HIGH_BEAM_POT_MAX_VALUE       4095U
#define LIGHT_APP_HIGH_BEAM_PAIR_NUMBER         4U

/* ========================================================= */
/* Local Types                                               */
/* ========================================================= */

typedef union
{
    struct
    {
        uint8_t drl_light : 1;
        uint8_t pos_light : 1;
        uint8_t low_light : 1;
        uint8_t fog_light : 1;
    } bits;

    uint8_t all_lights;
} u_Light_Functions;

typedef union
{
    struct
    {
        uint8_t l_ti : 1;
        uint8_t r_ti : 1;
        uint8_t haz  : 1;
    } bits;

    uint8_t all_lights;
} u_Blink_Light_Functions;

typedef enum
{
    AUTO_SHAPE_DRL = 0U,
    AUTO_SHAPE_LOW_BEAM,
    AUTO_SHAPE_HIGH_BEAM
} e_Auto_Shape_Req;

typedef struct
{
    s_Light_Func_Comm light_command;
    s_Blink_Func_Comm blink_command;

    bool high_beam_command;

    /*
     * Expected range:
     * 0   -> very dark
     * 100 -> very bright
     */
    uint8_t luminosity_value;

    /*
     * Expected range:
     * 0    -> first pair of high beam LEDs
     * 4095 -> last pair of high beam LEDs
     */
    uint16_t high_beam_shape_pot_value;
} s_Light_App_Input_Data;

typedef struct
{
    s_Light_Pixel_PWM_Command drl_pos_command;
    s_Light_Pixel_PWM_Command high_beam_command;
    s_Light_Pixel_PWM_Command ti_command;

    uint8_t low_beam_duty;
    uint8_t fog_duty;

    /*
     * TODO:
     * Send this to Fan_App through RTE when the interface is added.
     */
    bool fan_speed_increase_request;
    bool motor_auto_control_request;
} s_Light_App_Output_Data;

/* ========================================================= */
/* Global Application Data                                   */
/* ========================================================= */

u_Light_Functions light_functions = { .all_lights = LIGHT_APP_OFF_STATE };
u_Blink_Light_Functions blink_functions = { .all_lights = LIGHT_APP_OFF_STATE };

bool high_beam_function = false;
bool lights_auto_function = false;

e_Auto_Shape_Req auto_shape_req = AUTO_SHAPE_DRL;

/* ========================================================= */
/* Local Data                                                */
/* ========================================================= */

static s_Light_App_Input_Data light_app_input_data;
static s_Light_App_Output_Data light_app_output_data;

static bool light_app_is_initialized = false;
static bool welcome_animation_started = false;
static bool goodbye_animation_was_executed = false;

static uint32_t off_timer_ms = 0U;
static uint32_t high_beam_timer_ms = 0U;

/* ========================================================= */
/* Local Function Prototypes                                 */
/* ========================================================= */

static void Light_App_Reset_Input_Data(void);
static void Light_App_Reset_Output_Data(void);
static void Light_App_Reset_Internal_Data(void);

static void Light_App_Read_Input_Data(void);
static void Light_App_Map_Input_Commands(void);

static void Light_App_Calculate(void);
static bool Light_App_Calculate_Animation(void);
static void Light_App_Calculate_Normal_Shape(void);
static void Light_App_Calculate_Auto_Shape(void);
static void Light_App_Calculate_Turn_Indicator(void);

static void Light_App_Handle_Off_Timer(void);
static void Light_App_Handle_High_Beam_Timer(void);

static void Light_App_Transmit_Output_Data(void);

static s_Light_Pixel_PWM_Command Light_App_Get_PWM_Command_Off(void);
static s_Light_Pixel_PWM_Command Light_App_Get_PWM_Command_100(u_Light_Pixel pixels);
static s_Light_Pixel_PWM_Command Light_App_Get_HighBeam_Command_From_Pot(uint16_t pot_value);

static u_Light_Pixel Light_App_Get_All_Pixels_Off(void);
static u_Light_Pixel Light_App_Get_All_Pixels_On(void);
static u_Light_Pixel Light_App_Get_HighBeam_Pair_Pixels(uint8_t pair_index);

/* ========================================================= */
/* Local Functions                                           */
/* ========================================================= */

static void Light_App_Reset_Input_Data(void)
{
    light_app_input_data.light_command.Off_State = false;
    light_app_input_data.light_command.Position_State = false;
    light_app_input_data.light_command.DRL_State = false;
    light_app_input_data.light_command.Auto_State = false;
    light_app_input_data.light_command.LowBeam_State = false;
    light_app_input_data.light_command.FogLight_State = false;

    light_app_input_data.blink_command.L_Blink = false;
    light_app_input_data.blink_command.R_Blink = false;
    light_app_input_data.blink_command.Hazzard_Blink = false;

    light_app_input_data.high_beam_command = false;

    light_app_input_data.luminosity_value = 100U;
    light_app_input_data.high_beam_shape_pot_value = 0U;
}

static void Light_App_Reset_Output_Data(void)
{
    light_app_output_data.drl_pos_command = Light_App_Get_PWM_Command_Off();
    light_app_output_data.high_beam_command = Light_App_Get_PWM_Command_Off();
    light_app_output_data.ti_command = Light_App_Get_PWM_Command_Off();

    light_app_output_data.low_beam_duty = LIGHT_APP_DUTY_OFF;
    light_app_output_data.fog_duty = LIGHT_APP_DUTY_OFF;

    light_app_output_data.fan_speed_increase_request = false;
    light_app_output_data.motor_auto_control_request = false;
}

static void Light_App_Reset_Internal_Data(void)
{
    light_functions.all_lights = LIGHT_APP_OFF_STATE;
    blink_functions.all_lights = LIGHT_APP_OFF_STATE;

    high_beam_function = false;
    lights_auto_function = false;
    auto_shape_req = AUTO_SHAPE_DRL;

    welcome_animation_started = false;
    goodbye_animation_was_executed = false;

    off_timer_ms = 0U;
    high_beam_timer_ms = 0U;
}

static void Light_App_Read_Input_Data(void)
{
    uint8_t local_luminosity_sensor_value = RTE_Read_Ambiental_Luminosity();
    uint16_t local_high_beam_shape_pot_value = RTE_Read_HighBeam_Pot_Control_Level();

    light_app_input_data.light_command = RTE_Read_Light_Command_Switch_Position();
    light_app_input_data.blink_command = RTE_Read_Blinker_Command_State();
    light_app_input_data.high_beam_command = RTE_Read_HighBeam_Command_State();

    light_app_input_data.luminosity_value = local_luminosity_sensor_value;
    light_app_input_data.high_beam_shape_pot_value = local_high_beam_shape_pot_value;
}

static void Light_App_Map_Input_Commands(void)
{
    light_functions.all_lights = LIGHT_APP_OFF_STATE;
    blink_functions.all_lights = LIGHT_APP_OFF_STATE;

    lights_auto_function = false;

    if (true == light_app_input_data.light_command.Off_State)
    {
        light_functions.all_lights = LIGHT_APP_OFF_STATE;
        lights_auto_function = false;
    }
    else if (true == light_app_input_data.light_command.Auto_State)
    {
        light_functions.all_lights = LIGHT_APP_OFF_STATE;
        lights_auto_function = true;
    }
    else
    {
        light_functions.bits.pos_light = light_app_input_data.light_command.Position_State;
        light_functions.bits.drl_light = light_app_input_data.light_command.DRL_State;
        light_functions.bits.low_light = light_app_input_data.light_command.LowBeam_State;
        light_functions.bits.fog_light = light_app_input_data.light_command.FogLight_State;
    }

    blink_functions.bits.l_ti = light_app_input_data.blink_command.L_Blink;
    blink_functions.bits.r_ti = light_app_input_data.blink_command.R_Blink;
    blink_functions.bits.haz = light_app_input_data.blink_command.Hazzard_Blink;

    high_beam_function = light_app_input_data.high_beam_command;
}

static void Light_App_Calculate(void)
{
    bool local_animation_active = false;

    Light_App_Reset_Output_Data();
    Light_App_Map_Input_Commands();

    local_animation_active = Light_App_Calculate_Animation();

    if (false == local_animation_active)
    {
        if (true == lights_auto_function)
        {
            Light_App_Calculate_Auto_Shape();
        }
        else
        {
            Light_App_Calculate_Normal_Shape();
        }

        Light_App_Calculate_Turn_Indicator();

        Light_App_Handle_Off_Timer();
        Light_App_Handle_High_Beam_Timer();
    }
}

static bool Light_App_Calculate_Animation(void)
{
    bool local_animation_active = false;

    if (true == Light_Animation_Is_Active())
    {
        s_Light_Animation_Output local_animation_output;

        Light_Animation_Run_10ms();
        local_animation_output = Light_Animation_Get_Output();

        light_app_output_data.drl_pos_command = local_animation_output.drl_pos_command;
        light_app_output_data.high_beam_command = local_animation_output.high_beam_command;
        light_app_output_data.ti_command = local_animation_output.ti_command;

        light_app_output_data.low_beam_duty = local_animation_output.low_beam_duty;
        light_app_output_data.fog_duty = local_animation_output.fog_duty;

        local_animation_active = true;
    }

    return local_animation_active;
}

static void Light_App_Calculate_Normal_Shape(void)
{
    u_Light_Pixel local_all_pixels_on = Light_App_Get_All_Pixels_On();

    /*
     * Normal shape priority:
     * 1. OFF
     * 2. FOG      -> POS + LowBeam + Fog
     * 3. LOW      -> POS + LowBeam
     * 4. DRL      -> DRL
     * 5. POS      -> POS
     *
     * HighBeam is a separate LED group and separate command.
     */

    if (true == light_app_input_data.light_command.Off_State)
    {
        /* Outputs already reset to OFF. */
    }
    else if (LIGHT_APP_ON_STATE == light_functions.bits.fog_light)
    {
        light_app_output_data.drl_pos_command = Light_App_Get_PWM_Command_100(local_all_pixels_on);
        light_app_output_data.low_beam_duty = LIGHT_APP_DUTY_LOW_BEAM_ON;
        light_app_output_data.fog_duty = LIGHT_APP_DUTY_FOG_ON;
    }
    else if (LIGHT_APP_ON_STATE == light_functions.bits.low_light)
    {
        light_app_output_data.drl_pos_command = Light_App_Get_PWM_Command_100(local_all_pixels_on);
        light_app_output_data.low_beam_duty = LIGHT_APP_DUTY_LOW_BEAM_ON;
    }
    else if (LIGHT_APP_ON_STATE == light_functions.bits.drl_light)
    {
        light_app_output_data.drl_pos_command = Light_App_Get_PWM_Command_100(local_all_pixels_on);
    }
    else if (LIGHT_APP_ON_STATE == light_functions.bits.pos_light)
    {
        /*
         * POS is put on 25% mask.
         * DRL is put on 100% mask.
         *
         * If later you want POS at 50% or 75%, only this assignment changes.
         */
        light_app_output_data.drl_pos_command.pixels_25 = local_all_pixels_on;
    }
    else
    {
        /* Outputs already reset to OFF. */
    }

    if ((true == high_beam_function) &&
        (false == light_app_input_data.light_command.Off_State))
    {
        /*
         * Normal HighBeam: all high beam LEDs ON at 100%.
         * Dynamic 2-by-2 movement is used in Auto shape.
         */
        light_app_output_data.high_beam_command = Light_App_Get_PWM_Command_100(local_all_pixels_on);
    }
}

static void Light_App_Calculate_Auto_Shape(void)
{
    u_Light_Pixel local_all_pixels_on = Light_App_Get_All_Pixels_On();

    light_app_output_data.motor_auto_control_request = true;

    /*
     *
     * Current logic:
     * bright environment -> DRL
     * medium environment -> POS + LowBeam
     * dark environment   -> POS + LowBeam
     * dark + HighBeam    -> POS + LowBeam + dynamic HighBeam pair
     */

    if (light_app_input_data.luminosity_value >= LIGHT_APP_LUMINOSITY_LIGHT_LEVEL)
    {
        auto_shape_req = AUTO_SHAPE_DRL;
    }
    else if (light_app_input_data.luminosity_value >= LIGHT_APP_LUMINOSITY_DARK_LEVEL)
    {
        auto_shape_req = AUTO_SHAPE_LOW_BEAM;
    }
    else
    {
        if (true == high_beam_function)
        {
            auto_shape_req = AUTO_SHAPE_HIGH_BEAM;
        }
        else
        {
            auto_shape_req = AUTO_SHAPE_LOW_BEAM;
        }
    }

    if (AUTO_SHAPE_DRL == auto_shape_req)
    {
        light_app_output_data.drl_pos_command = Light_App_Get_PWM_Command_100(local_all_pixels_on);
    }
    else if (AUTO_SHAPE_LOW_BEAM == auto_shape_req)
    {
        light_app_output_data.drl_pos_command.pixels_25 = local_all_pixels_on;
        light_app_output_data.low_beam_duty = LIGHT_APP_DUTY_LOW_BEAM_ON;
    }
    else if (AUTO_SHAPE_HIGH_BEAM == auto_shape_req)
    {
        light_app_output_data.drl_pos_command.pixels_25 = local_all_pixels_on;
        light_app_output_data.low_beam_duty = LIGHT_APP_DUTY_LOW_BEAM_ON;

        light_app_output_data.high_beam_command =
            Light_App_Get_HighBeam_Command_From_Pot(light_app_input_data.high_beam_shape_pot_value);
    }
    else
    {
        /* Do nothing */
    }
}

static void Light_App_Calculate_Turn_Indicator(void)
{
    bool local_left_indicator_requested = false;

    /*
     * Current project decision:
     * - only left turn indicator is implemented
     * - right turn indicator is NOT mapped yet
     * - no left/right split inside the TI LED group
     *
     * TODO:
     * R_Blink shall be added when Parameter file functionality is available.
     */

    if ((LIGHT_APP_ON_STATE == blink_functions.bits.l_ti) ||
        (LIGHT_APP_ON_STATE == blink_functions.bits.haz))
    {
        local_left_indicator_requested = true;
    }

    if (true == local_left_indicator_requested)
    {
        Light_Scrolling_Ind_Run_10ms();
        light_app_output_data.ti_command = Light_Scrolling_Ind_Get_Output();
    }
    else
    {
        Light_Scrolling_Ind_Stop();
        light_app_output_data.ti_command = Light_App_Get_PWM_Command_Off();
    }
}

static void Light_App_Handle_Off_Timer(void)
{
    bool local_all_lights_are_off = false;

    if ((true == light_app_input_data.light_command.Off_State) &&
        (false == high_beam_function) &&
        (LIGHT_APP_OFF_STATE == blink_functions.all_lights))
    {
        local_all_lights_are_off = true;
    }

    if (true == local_all_lights_are_off)
    {
        if (off_timer_ms < LIGHT_APP_OFF_TIMEOUT_MS)
        {
            off_timer_ms += LIGHT_APP_TASK_TIME_MS;
        }
        else
        {
            if (false == goodbye_animation_was_executed)
            {
                goodbye_animation_was_executed = true;
                off_timer_ms = 0U;

                Light_Animation_Start_Goodbye();
            }
        }
    }
    else
    {
        off_timer_ms = 0U;
        goodbye_animation_was_executed = false;
    }
}

static void Light_App_Handle_High_Beam_Timer(void)
{
    if (LIGHT_APP_OFF_STATE != light_app_output_data.high_beam_command.pixels_100.byte)
    {
        if (high_beam_timer_ms < LIGHT_APP_HIGH_BEAM_FAN_TIME_MS)
        {
            high_beam_timer_ms += LIGHT_APP_TASK_TIME_MS;
        }
        else
        {
            light_app_output_data.fan_speed_increase_request = true;
        }
    }
    else
    {
        high_beam_timer_ms = 0U;
        light_app_output_data.fan_speed_increase_request = false;
    }

    /*
     * TODO:
     * Send fan_speed_increase_request to Fan_App through RTE when
     * Light_App -> Fan_App interface is added.
     */
}

static void Light_App_Transmit_Output_Data(void)
{
    Light_Functionality_POS_DRL_Command(light_app_output_data.drl_pos_command);
    Light_Functionality_Low_Beam_Command(light_app_output_data.low_beam_duty);
    Light_Functionality_High_Beam_Command(light_app_output_data.high_beam_command);
    Light_Functionality_TI_Hazzard_Command(light_app_output_data.ti_command);
    Light_Functionality_FOG_Command(light_app_output_data.fog_duty);
    RTE_Write_Motor_Command_Auto(light_app_output_data.motor_auto_control_request);
    RTE_Write_Increase_FanSpeed_HighBeam(light_app_output_data.fan_speed_increase_request);
}

static s_Light_Pixel_PWM_Command Light_App_Get_PWM_Command_Off(void)
{
    s_Light_Pixel_PWM_Command local_command;

    local_command.pixels_25.byte = 0x00U;
    local_command.pixels_50.byte = 0x00U;
    local_command.pixels_75.byte = 0x00U;
    local_command.pixels_100.byte = 0x00U;

    return local_command;
}

static s_Light_Pixel_PWM_Command Light_App_Get_PWM_Command_100(u_Light_Pixel pixels)
{
    s_Light_Pixel_PWM_Command local_command;

    local_command = Light_App_Get_PWM_Command_Off();

    local_command.pixels_100 = pixels;

    return local_command;
}

static s_Light_Pixel_PWM_Command Light_App_Get_HighBeam_Command_From_Pot(uint16_t pot_value)
{
    s_Light_Pixel_PWM_Command local_command;
    uint8_t local_pair_index = 0U;
    uint16_t local_step_size = 0U;

    local_command = Light_App_Get_PWM_Command_Off();

    local_step_size = (uint16_t)((LIGHT_APP_HIGH_BEAM_POT_MAX_VALUE + 1U) /
                                 LIGHT_APP_HIGH_BEAM_PAIR_NUMBER);

    if (0U != local_step_size)
    {
        local_pair_index = (uint8_t)(pot_value / local_step_size);
    }

    if (local_pair_index >= LIGHT_APP_HIGH_BEAM_PAIR_NUMBER)
    {
        local_pair_index = (uint8_t)(LIGHT_APP_HIGH_BEAM_PAIR_NUMBER - 1U);
    }

    /*
     * HighBeam movement 2 by 2:
     *
     * pair 0 -> LED0 + LED1
     * pair 1 -> LED2 + LED3
     * pair 2 -> LED4 + LED5
     * pair 3 -> LED6 + LED7
     */
    local_command.pixels_100 = Light_App_Get_HighBeam_Pair_Pixels(local_pair_index);

    return local_command;
}

static u_Light_Pixel Light_App_Get_All_Pixels_Off(void)
{
    u_Light_Pixel local_pixels;

    local_pixels.byte = 0x00U;

    return local_pixels;
}

static u_Light_Pixel Light_App_Get_All_Pixels_On(void)
{
    u_Light_Pixel local_pixels;

    local_pixels.byte = 0xFFU;

    return local_pixels;
}

static u_Light_Pixel Light_App_Get_HighBeam_Pair_Pixels(uint8_t pair_index)
{
    u_Light_Pixel local_pixels;

    local_pixels = Light_App_Get_All_Pixels_Off();

    if (0U == pair_index)
    {
        local_pixels.bit.LED0 = LIGHT_APP_ON_STATE;
        local_pixels.bit.LED1 = LIGHT_APP_ON_STATE;
    }
    else if (1U == pair_index)
    {
        local_pixels.bit.LED2 = LIGHT_APP_ON_STATE;
        local_pixels.bit.LED3 = LIGHT_APP_ON_STATE;
    }
    else if (2U == pair_index)
    {
        local_pixels.bit.LED4 = LIGHT_APP_ON_STATE;
        local_pixels.bit.LED5 = LIGHT_APP_ON_STATE;
    }
    else if (3U == pair_index)
    {
        local_pixels.bit.LED6 = LIGHT_APP_ON_STATE;
        local_pixels.bit.LED7 = LIGHT_APP_ON_STATE;
    }
    else
    {
        /* Do nothing */
    }

    return local_pixels;
}

/* ========================================================= */
/* Global Functions                                          */
/* ========================================================= */

void Init_Light_App(void)
{
    if (false == RTE_Read_Light_Control_Init_Done())
    {
        light_app_is_initialized = false;
    }
    else
    {
        Light_App_Reset_Input_Data();
        Light_App_Reset_Output_Data();
        Light_App_Reset_Internal_Data();

        Init_Light_Animation();
        Init_Light_Scrolling_Ind();

        Light_Animation_Start_Welcome();
        welcome_animation_started = true;

        light_app_is_initialized = true;
    }
}

void Run_Light_App_Main_10ms(void)
{
    if (false == light_app_is_initialized)
    {
        Init_Light_App();
    }

    if (true == light_app_is_initialized)
    {
        Light_App_Read_Input_Data();
        Light_App_Calculate();
        Light_App_Transmit_Output_Data();
    }
}