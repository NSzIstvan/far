/*
 * Light_animation.c
 *
 * Created on: Jul 13, 2026
 * Author: nsist
 */

#include "../include/Light_Animation.h"

#include <stdbool.h>
#include <stdint.h>

/* ========================================================= */
/* Defines                                                   */
/* ========================================================= */

#define LIGHT_ANIMATION_TASK_TIME_MS                  10U

#define LIGHT_ANIMATION_LED_NUMBER                    8U

#define LIGHT_ANIMATION_DUTY_OFF                      0U
#define LIGHT_ANIMATION_DUTY_25                       25U
#define LIGHT_ANIMATION_DUTY_50                       50U
#define LIGHT_ANIMATION_DUTY_75                       75U
#define LIGHT_ANIMATION_DUTY_100                      100U

#define LIGHT_ANIMATION_WELCOME_PULSE_TIME_MS         500U
#define LIGHT_ANIMATION_WELCOME_MAIN_TIME_MS          2000U
#define LIGHT_ANIMATION_WELCOME_TOTAL_TIME_MS         (LIGHT_ANIMATION_WELCOME_PULSE_TIME_MS + \
                                                       LIGHT_ANIMATION_WELCOME_MAIN_TIME_MS)

#define LIGHT_ANIMATION_GOODBYE_TIME_MS               2000U

#define LIGHT_ANIMATION_PULSE_STEP_NUMBER             9U

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
static s_Light_Pixel_PWM_Command Light_Animation_Get_PWM_Command_By_Duty(u_Light_Pixel pixels,
                                                                         uint8_t duty);
static s_Light_Pixel_PWM_Command Light_Animation_Get_Welcome_Main_PWM_Command(uint8_t current_led);
static s_Light_Pixel_PWM_Command Light_Animation_Get_Goodbye_PWM_Command(uint8_t current_led);
static s_Light_Pixel_PWM_Command Light_Animation_Get_Welcome_DRL_Pulse_Command(uint32_t timer_ms);
static s_Light_Pixel_PWM_Command Light_Animation_Get_Welcome_TI_Scroll_Command(uint32_t timer_ms);

static u_Light_Pixel Light_Animation_Get_All_Pixels_Off(void);
static u_Light_Pixel Light_Animation_Get_All_Pixels_On(void);
static u_Light_Pixel Light_Animation_Get_Pixels_Before_Current(uint8_t current_led);
static u_Light_Pixel Light_Animation_Get_Pixels_Until_Current(uint8_t current_led);
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
    uint32_t local_main_animation_timer_ms = 0U;
    uint8_t local_current_led = 0U;
    s_Light_Pixel_PWM_Command local_animation_command;

    light_animation_output.animation_active = true;
    light_animation_output.animation_finished = false;

    if (light_animation_timer_ms < LIGHT_ANIMATION_WELCOME_PULSE_TIME_MS)
    {
        /*
         * Welcome phase 1: first 500 ms
         *
         * TI:
         * - normal scrolling from LED0 to LED7 in 500 ms
         *
         * DRL/POS:
         * - pulse effect:
         *   0 -> 25 -> 50 -> 75 -> 100 -> 75 -> 50 -> 25 -> 0
         */
        light_animation_output.drl_pos_command =
            Light_Animation_Get_Welcome_DRL_Pulse_Command(light_animation_timer_ms);

        light_animation_output.ti_command =
            Light_Animation_Get_Welcome_TI_Scroll_Command(light_animation_timer_ms);

        light_animation_output.high_beam_command = Light_Animation_Get_PWM_Command_Off();

        light_animation_output.low_beam_duty = LIGHT_ANIMATION_DUTY_OFF;
        light_animation_output.fog_duty = LIGHT_ANIMATION_DUTY_OFF;

        light_animation_timer_ms += LIGHT_ANIMATION_TASK_TIME_MS;
    }
    else if (light_animation_timer_ms < LIGHT_ANIMATION_WELCOME_TOTAL_TIME_MS)
    {
        /*
         * Welcome phase 2: next 2000 ms
         *
         * Current animation:
         * - previous LEDs = 100%
         * - current LED   = 50%
         * - next LEDs     = OFF
         *
         * This animation is applied to both DRL/POS and TI.
         */
        local_main_animation_timer_ms =
            light_animation_timer_ms - LIGHT_ANIMATION_WELCOME_PULSE_TIME_MS;

        local_current_led =
            (uint8_t)((local_main_animation_timer_ms * LIGHT_ANIMATION_LED_NUMBER) /
                      LIGHT_ANIMATION_WELCOME_MAIN_TIME_MS);

        if (local_current_led >= LIGHT_ANIMATION_LED_NUMBER)
        {
            local_current_led = (uint8_t)(LIGHT_ANIMATION_LED_NUMBER - 1U);
        }

        local_animation_command =
            Light_Animation_Get_Welcome_Main_PWM_Command(local_current_led);

        light_animation_output.drl_pos_command = local_animation_command;
        light_animation_output.ti_command = local_animation_command;

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
        local_current_led =
            (uint8_t)((light_animation_timer_ms * LIGHT_ANIMATION_LED_NUMBER) /
                      LIGHT_ANIMATION_GOODBYE_TIME_MS);

        if (local_current_led >= LIGHT_ANIMATION_LED_NUMBER)
        {
            local_current_led = (uint8_t)(LIGHT_ANIMATION_LED_NUMBER - 1U);
        }

        /*
         * Goodbye animation:
         *
         * Reverse of the main welcome animation.
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
         * ...
         */
        local_animation_command =
            Light_Animation_Get_Goodbye_PWM_Command(local_current_led);

        light_animation_output.drl_pos_command = local_animation_command;
        light_animation_output.ti_command = local_animation_command;

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

static s_Light_Pixel_PWM_Command Light_Animation_Get_PWM_Command_By_Duty(u_Light_Pixel pixels,
                                                                         uint8_t duty)
{
    s_Light_Pixel_PWM_Command local_command;

    local_command = Light_Animation_Get_PWM_Command_Off();

    if (LIGHT_ANIMATION_DUTY_25 == duty)
    {
        local_command.pixels_25 = pixels;
    }
    else if (LIGHT_ANIMATION_DUTY_50 == duty)
    {
        local_command.pixels_50 = pixels;
    }
    else if (LIGHT_ANIMATION_DUTY_75 == duty)
    {
        local_command.pixels_75 = pixels;
    }
    else if (LIGHT_ANIMATION_DUTY_100 == duty)
    {
        local_command.pixels_100 = pixels;
    }
    else
    {
        /* Duty OFF or unsupported duty -> command remains OFF. */
    }

    return local_command;
}

static s_Light_Pixel_PWM_Command Light_Animation_Get_Welcome_Main_PWM_Command(uint8_t current_led)
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
    local_command.pixels_100 =
        Light_Animation_Get_Pixels_Before_Current(local_reverse_current_led);

    local_command.pixels_50 =
        Light_Animation_Get_Current_Pixel(local_reverse_current_led);

    return local_command;
}

static s_Light_Pixel_PWM_Command Light_Animation_Get_Welcome_DRL_Pulse_Command(uint32_t timer_ms)
{
    s_Light_Pixel_PWM_Command local_command;
    u_Light_Pixel local_all_pixels_on;
    uint8_t local_pulse_step = 0U;
    uint8_t local_duty = LIGHT_ANIMATION_DUTY_OFF;

    local_command = Light_Animation_Get_PWM_Command_Off();
    local_all_pixels_on = Light_Animation_Get_All_Pixels_On();

    local_pulse_step =
        (uint8_t)((timer_ms * LIGHT_ANIMATION_PULSE_STEP_NUMBER) /
                  LIGHT_ANIMATION_WELCOME_PULSE_TIME_MS);

    if (local_pulse_step >= LIGHT_ANIMATION_PULSE_STEP_NUMBER)
    {
        local_pulse_step = (uint8_t)(LIGHT_ANIMATION_PULSE_STEP_NUMBER - 1U);
    }

    if (0U == local_pulse_step)
    {
        local_duty = LIGHT_ANIMATION_DUTY_OFF;
    }
    else if (1U == local_pulse_step)
    {
        local_duty = LIGHT_ANIMATION_DUTY_25;
    }
    else if (2U == local_pulse_step)
    {
        local_duty = LIGHT_ANIMATION_DUTY_50;
    }
    else if (3U == local_pulse_step)
    {
        local_duty = LIGHT_ANIMATION_DUTY_75;
    }
    else if (4U == local_pulse_step)
    {
        local_duty = LIGHT_ANIMATION_DUTY_100;
    }
    else if (5U == local_pulse_step)
    {
        local_duty = LIGHT_ANIMATION_DUTY_75;
    }
    else if (6U == local_pulse_step)
    {
        local_duty = LIGHT_ANIMATION_DUTY_50;
    }
    else if (7U == local_pulse_step)
    {
        local_duty = LIGHT_ANIMATION_DUTY_25;
    }
    else
    {
        local_duty = LIGHT_ANIMATION_DUTY_OFF;
    }

    local_command = Light_Animation_Get_PWM_Command_By_Duty(local_all_pixels_on,
                                                            local_duty);

    return local_command;
}

static s_Light_Pixel_PWM_Command Light_Animation_Get_Welcome_TI_Scroll_Command(uint32_t timer_ms)
{
    s_Light_Pixel_PWM_Command local_command;
    uint8_t local_current_led = 0U;

    local_command = Light_Animation_Get_PWM_Command_Off();

    /*
     * TI scrolling in 500 ms.
     * It fills LED0 -> LED7 using the full TI group.
     */
    local_current_led =
        (uint8_t)((timer_ms * LIGHT_ANIMATION_LED_NUMBER) /
                  LIGHT_ANIMATION_WELCOME_PULSE_TIME_MS);

    if (local_current_led >= LIGHT_ANIMATION_LED_NUMBER)
    {
        local_current_led = (uint8_t)(LIGHT_ANIMATION_LED_NUMBER - 1U);
    }

    local_command.pixels_100 =
        Light_Animation_Get_Pixels_Until_Current(local_current_led);

    return local_command;
}

static u_Light_Pixel Light_Animation_Get_All_Pixels_Off(void)
{
    u_Light_Pixel local_pixels;

    local_pixels.byte = 0x00U;

    return local_pixels;
}

static u_Light_Pixel Light_Animation_Get_All_Pixels_On(void)
{
    u_Light_Pixel local_pixels;

    local_pixels.byte = 0xFFU;

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

static u_Light_Pixel Light_Animation_Get_Pixels_Until_Current(uint8_t current_led)
{
    u_Light_Pixel local_pixels;
    uint8_t local_index = 0U;

    local_pixels = Light_Animation_Get_All_Pixels_Off();

    if (current_led >= LIGHT_ANIMATION_LED_NUMBER)
    {
        current_led = (uint8_t)(LIGHT_ANIMATION_LED_NUMBER - 1U);
    }

    for (local_index = 0U; local_index <= current_led; local_index++)
    {
        local_pixels.byte |= (uint8_t)(1U << local_index);
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