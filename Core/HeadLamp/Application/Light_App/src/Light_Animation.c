/*
 * Light_animation.c
 *
 * Created on: Jul 13, 2026
 * Author: nsist
 */

#include "../include/Light_animation.h"

#include <stdbool.h>
#include <stdint.h>

/* ========================================================= */
/* Defines                                                   */
/* ========================================================= */

#define LIGHT_ANIMATION_TASK_TIME_MS             10U

#define LIGHT_ANIMATION_LED_NUMBER               8U

#define LIGHT_ANIMATION_DUTY_OFF                 0U

#define LIGHT_ANIMATION_WELCOME_TIME_MS          2000U
#define LIGHT_ANIMATION_GOODBYE_TIME_MS          2000U

#define LIGHT_ANIMATION_WELCOME_STEP_TIME_MS     (LIGHT_ANIMATION_WELCOME_TIME_MS / LIGHT_ANIMATION_LED_NUMBER)
#define LIGHT_ANIMATION_GOODBYE_STEP_TIME_MS     (LIGHT_ANIMATION_GOODBYE_TIME_MS / LIGHT_ANIMATION_LED_NUMBER)

/* ========================================================= */
/* Local Types                                               */
/* ========================================================= */

typedef enum
{
    LIGHT_ANIMATION_IDLE_STATE = 0U,
    LIGHT_ANIMATION_WELCOME_STATE,
    LIGHT_ANIMATION_GOODBYE_STATE
} e_Light_Animation_State;

/* ========================================================= */
/* Local Data                                                */
/* ========================================================= */

static e_Light_Animation_State light_animation_state = LIGHT_ANIMATION_IDLE_STATE;

static s_Light_Animation_Output light_animation_output;

static uint32_t light_animation_timer_ms = 0U;

/* ========================================================= */
/* Local Function Prototypes                                 */
/* ========================================================= */

static void Light_Animation_Reset_Output(void);
static void Light_Animation_Set_Idle_State(void);

static void Light_Animation_Calculate_Welcome(void);
static void Light_Animation_Calculate_Goodbye(void);

static s_Light_Pixel_PWM_Command Light_Animation_Get_PWM_Command_Off(void);
static s_Light_Pixel_PWM_Command Light_Animation_Get_Welcome_PWM_Command(uint8_t current_led);
static s_Light_Pixel_PWM_Command Light_Animation_Get_Goodbye_PWM_Command(uint8_t current_led);

static u_Light_Pixel Light_Animation_Get_All_Pixels_Off(void);
static u_Light_Pixel Light_Animation_Get_Pixels_Before_Current(uint8_t current_led);
static u_Light_Pixel Light_Animation_Get_Pixels_After_Current(uint8_t current_led);
static u_Light_Pixel Light_Animation_Get_Current_Pixel(uint8_t current_led);

/* ========================================================= */
/* Local Functions                                           */
/* ========================================================= */

static void Light_Animation_Reset_Output(void)
{
    light_animation_output.drl_pos_command = Light_Animation_Get_PWM_Command_Off();
    light_animation_output.high_beam_command = Light_Animation_Get_PWM_Command_Off();
    light_animation_output.ti_command = Light_Animation_Get_PWM_Command_Off();

    light_animation_output.low_beam_duty = LIGHT_ANIMATION_DUTY_OFF;
    light_animation_output.fog_duty = LIGHT_ANIMATION_DUTY_OFF;

    light_animation_output.animation_active = false;
}

static void Light_Animation_Set_Idle_State(void)
{
    light_animation_state = LIGHT_ANIMATION_IDLE_STATE;
    light_animation_timer_ms = 0U;

    Light_Animation_Reset_Output();

    light_animation_output.animation_active = false;
    light_animation_output.animation_finished = true;
}

static void Light_Animation_Calculate_Welcome(void)
{
    uint8_t local_current_led = 0U;
    s_Light_Pixel_PWM_Command local_animation_command;

    light_animation_output.animation_active = true;
    light_animation_output.animation_finished = false;

    if (light_animation_timer_ms < LIGHT_ANIMATION_WELCOME_TIME_MS)
    {
        local_current_led = (uint8_t)(light_animation_timer_ms / LIGHT_ANIMATION_WELCOME_STEP_TIME_MS);

        if (local_current_led >= LIGHT_ANIMATION_LED_NUMBER)
        {
            local_current_led = (uint8_t)(LIGHT_ANIMATION_LED_NUMBER - 1U);
        }

        /*
         * Welcome animation:
         *
         * Step 0:
         * LED0 = 50%
         *
         * Step 1:
         * LED0 = 100%
         * LED1 = 50%
         *
         * Step 2:
         * LED0 = 100%
         * LED1 = 100%
         * LED2 = 50%
         *
         * ...
         */
        local_animation_command = Light_Animation_Get_Welcome_PWM_Command(local_current_led);

        light_animation_output.drl_pos_command = local_animation_command;
        light_animation_output.ti_command = local_animation_command;

        /*
         * HighBeam is not used in welcome animation.
         * It has a separate LED group and is controlled by Light_App normal/auto shape.
         */
        light_animation_output.high_beam_command = Light_Animation_Get_PWM_Command_Off();

        light_animation_output.low_beam_duty = LIGHT_ANIMATION_DUTY_OFF;
        light_animation_output.fog_duty = LIGHT_ANIMATION_DUTY_OFF;

        light_animation_timer_ms += LIGHT_ANIMATION_TASK_TIME_MS;
    }
    else
    {
        Light_Animation_Set_Idle_State();
    }
}

static void Light_Animation_Calculate_Goodbye(void)
{
    uint8_t local_current_led = 0U;
    s_Light_Pixel_PWM_Command local_animation_command;

    light_animation_output.animation_active = true;
    light_animation_output.animation_finished = false;

    if (light_animation_timer_ms < LIGHT_ANIMATION_GOODBYE_TIME_MS)
    {
        local_current_led = (uint8_t)(light_animation_timer_ms / LIGHT_ANIMATION_GOODBYE_STEP_TIME_MS);

        if (local_current_led >= LIGHT_ANIMATION_LED_NUMBER)
        {
            local_current_led = (uint8_t)(LIGHT_ANIMATION_LED_NUMBER - 1U);
        }

        /*
         * Goodbye animation:
         *
         * Reverse of welcome.
         *
         * Step 0:
         * LED0 ... LED6 = 100%
         * LED7          = 50%
         *
         * Step 1:
         * LED0 ... LED5 = 100%
         * LED6          = 50%
         * LED7          = OFF
         *
         * Step 2:
         * LED0 ... LED4 = 100%
         * LED5          = 50%
         * LED6 ... LED7 = OFF
         *
         * ...
         */
        local_animation_command = Light_Animation_Get_Goodbye_PWM_Command(local_current_led);

        light_animation_output.drl_pos_command = local_animation_command;
        light_animation_output.ti_command = local_animation_command;

        /*
         * HighBeam is not used in goodbye animation.
         */
        light_animation_output.high_beam_command = Light_Animation_Get_PWM_Command_Off();

        light_animation_output.low_beam_duty = LIGHT_ANIMATION_DUTY_OFF;
        light_animation_output.fog_duty = LIGHT_ANIMATION_DUTY_OFF;

        light_animation_timer_ms += LIGHT_ANIMATION_TASK_TIME_MS;
    }
    else
    {
        Light_Animation_Set_Idle_State();
    }
}

static s_Light_Pixel_PWM_Command Light_Animation_Get_PWM_Command_Off(void)
{
    s_Light_Pixel_PWM_Command local_command;

    local_command.pixels_25.byte = 0x00U;
    local_command.pixels_50.byte = 0x00U;
    local_command.pixels_75.byte = 0x00U;
    local_command.pixels_100.byte = 0x00U;

    return local_command;
}

static s_Light_Pixel_PWM_Command Light_Animation_Get_Welcome_PWM_Command(uint8_t current_led)
{
    s_Light_Pixel_PWM_Command local_command;

    local_command = Light_Animation_Get_PWM_Command_Off();

    /*
     * Previous LEDs are 100%.
     * Current LED is 50%.
     * Next LEDs are OFF.
     */
    local_command.pixels_100 = Light_Animation_Get_Pixels_Before_Current(current_led);
    local_command.pixels_50 = Light_Animation_Get_Current_Pixel(current_led);

    return local_command;
}

static s_Light_Pixel_PWM_Command Light_Animation_Get_Goodbye_PWM_Command(uint8_t current_led)
{
    s_Light_Pixel_PWM_Command local_command;
    uint8_t local_reverse_current_led = 0U;

    local_command = Light_Animation_Get_PWM_Command_Off();

    if (current_led >= LIGHT_ANIMATION_LED_NUMBER)
    {
        local_reverse_current_led = 0U;
    }
    else
    {
        local_reverse_current_led = (uint8_t)((LIGHT_ANIMATION_LED_NUMBER - 1U) - current_led);
    }

    /*
     * LEDs before reverse current are 100%.
     * Reverse current LED is 50%.
     * LEDs after reverse current are OFF.
     */
    local_command.pixels_100 = Light_Animation_Get_Pixels_Before_Current(local_reverse_current_led);
    local_command.pixels_50 = Light_Animation_Get_Current_Pixel(local_reverse_current_led);

    return local_command;
}

static u_Light_Pixel Light_Animation_Get_All_Pixels_Off(void)
{
    u_Light_Pixel local_pixels;

    local_pixels.byte = 0x00U;

    return local_pixels;
}

static u_Light_Pixel Light_Animation_Get_Pixels_Before_Current(uint8_t current_led)
{
    u_Light_Pixel local_pixels;
    uint8_t local_index = 0U;

    local_pixels = Light_Animation_Get_All_Pixels_Off();

    if (current_led > LIGHT_ANIMATION_LED_NUMBER)
    {
        current_led = LIGHT_ANIMATION_LED_NUMBER;
    }

    for (local_index = 0U; local_index < current_led; local_index++)
    {
        local_pixels.byte |= (uint8_t)(1U << local_index);
    }

    return local_pixels;
}

static u_Light_Pixel Light_Animation_Get_Pixels_After_Current(uint8_t current_led)
{
    u_Light_Pixel local_pixels;
    uint8_t local_index = 0U;

    local_pixels = Light_Animation_Get_All_Pixels_Off();

    if (current_led < LIGHT_ANIMATION_LED_NUMBER)
    {
        for (local_index = (uint8_t)(current_led + 1U);
             local_index < LIGHT_ANIMATION_LED_NUMBER;
             local_index++)
        {
            local_pixels.byte |= (uint8_t)(1U << local_index);
        }
    }

    return local_pixels;
}

static u_Light_Pixel Light_Animation_Get_Current_Pixel(uint8_t current_led)
{
    u_Light_Pixel local_pixels;

    local_pixels = Light_Animation_Get_All_Pixels_Off();

    if (current_led < LIGHT_ANIMATION_LED_NUMBER)
    {
        local_pixels.byte = (uint8_t)(1U << current_led);
    }

    return local_pixels;
}

/* ========================================================= */
/* Global Functions                                          */
/* ========================================================= */

void Init_Light_Animation(void)
{
    light_animation_state = LIGHT_ANIMATION_IDLE_STATE;
    light_animation_timer_ms = 0U;

    Light_Animation_Reset_Output();

    light_animation_output.animation_active = false;
    light_animation_output.animation_finished = false;
}

void Light_Animation_Start_Welcome(void)
{
    light_animation_state = LIGHT_ANIMATION_WELCOME_STATE;
    light_animation_timer_ms = 0U;

    Light_Animation_Reset_Output();

    light_animation_output.animation_active = true;
    light_animation_output.animation_finished = false;
}

void Light_Animation_Start_Goodbye(void)
{
    light_animation_state = LIGHT_ANIMATION_GOODBYE_STATE;
    light_animation_timer_ms = 0U;

    Light_Animation_Reset_Output();

    light_animation_output.animation_active = true;
    light_animation_output.animation_finished = false;
}

void Light_Animation_Stop(void)
{
    light_animation_state = LIGHT_ANIMATION_IDLE_STATE;
    light_animation_timer_ms = 0U;

    Light_Animation_Reset_Output();

    light_animation_output.animation_active = false;
    light_animation_output.animation_finished = false;
}

void Light_Animation_Run_10ms(void)
{
    Light_Animation_Reset_Output();

    if (LIGHT_ANIMATION_WELCOME_STATE == light_animation_state)
    {
        Light_Animation_Calculate_Welcome();
    }
    else if (LIGHT_ANIMATION_GOODBYE_STATE == light_animation_state)
    {
        Light_Animation_Calculate_Goodbye();
    }
    else
    {
        light_animation_output.animation_active = false;
    }
}

s_Light_Animation_Output Light_Animation_Get_Output(void)
{
    return light_animation_output;
}

bool Light_Animation_Is_Active(void)
{
    return light_animation_output.animation_active;
}

bool Light_Animation_Is_Finished(void)
{
    return light_animation_output.animation_finished;
}