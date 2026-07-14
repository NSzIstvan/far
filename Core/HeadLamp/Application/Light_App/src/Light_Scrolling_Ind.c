/*
 * Light_scrolling_ind.c
 *
 * Created on: Jul 14, 2026
 * Author: nsist
 */

#include "../include/Light_scrolling_ind.h"

#include <stdbool.h>
#include <stdint.h>

/* ========================================================= */
/* Defines                                                   */
/* ========================================================= */

#define LIGHT_SCROLLING_IND_TASK_TIME_MS       10U

#define LIGHT_SCROLLING_IND_LED_NUMBER         8U

/*
 * Requirement:
 * scrolling indicator shall turn ON one LED at a time.
 *
 * The step time is calculated as:
 * 500ms / number of LEDs that are still OFF.
 */
#define LIGHT_SCROLLING_IND_BASE_TIME_MS       500U

#define LIGHT_SCROLLING_IND_FIRST_LED          0U
#define LIGHT_SCROLLING_IND_LAST_LED           (LIGHT_SCROLLING_IND_LED_NUMBER - 1U)

/* ========================================================= */
/* Local Data                                                */
/* ========================================================= */

static s_Light_Pixel_PWM_Command light_scrolling_ind_output;

static uint32_t light_scrolling_ind_timer_ms = 0U;
static uint16_t light_scrolling_ind_step_time_ms = 0U;

static uint8_t light_scrolling_ind_current_led = 0U;

static bool light_scrolling_ind_active = false;

/* ========================================================= */
/* Local Function Prototypes                                 */
/* ========================================================= */

static void Light_Scrolling_Ind_Reset_Output(void);
static void Light_Scrolling_Ind_Reset_Internal_Data(void);

static void Light_Scrolling_Ind_Calculate(void);
static void Light_Scrolling_Ind_Update_Step_Time(void);

static s_Light_Pixel_PWM_Command Light_Scrolling_Ind_Get_PWM_Command_Off(void);
static u_Light_Pixel Light_Scrolling_Ind_Get_Pixels_Until_Current(uint8_t current_led);

/* ========================================================= */
/* Local Functions                                           */
/* ========================================================= */

static void Light_Scrolling_Ind_Reset_Output(void)
{
    light_scrolling_ind_output = Light_Scrolling_Ind_Get_PWM_Command_Off();
}

static void Light_Scrolling_Ind_Reset_Internal_Data(void)
{
    light_scrolling_ind_timer_ms = 0U;
    light_scrolling_ind_step_time_ms = 0U;

    light_scrolling_ind_current_led = LIGHT_SCROLLING_IND_FIRST_LED;

    light_scrolling_ind_active = false;
}

static void Light_Scrolling_Ind_Calculate(void)
{
    Light_Scrolling_Ind_Update_Step_Time();

    /*
     * Scrolling indicator behavior:
     *
     * Step 0:
     * LED0 = 100%
     *
     * Step 1:
     * LED0 = 100%
     * LED1 = 100%
     *
     * Step 2:
     * LED0 = 100%
     * LED1 = 100%
     * LED2 = 100%
     *
     * ...
     *
     * After LED7 is ON, the sequence restarts from LED0.
     */
    light_scrolling_ind_output = Light_Scrolling_Ind_Get_PWM_Command_Off();
    light_scrolling_ind_output.pixels_100 =
        Light_Scrolling_Ind_Get_Pixels_Until_Current(light_scrolling_ind_current_led);

    light_scrolling_ind_timer_ms += LIGHT_SCROLLING_IND_TASK_TIME_MS;

    if (light_scrolling_ind_timer_ms >= light_scrolling_ind_step_time_ms)
    {
        light_scrolling_ind_timer_ms = 0U;

        if (light_scrolling_ind_current_led < LIGHT_SCROLLING_IND_LAST_LED)
        {
            light_scrolling_ind_current_led++;
        }
        else
        {
            light_scrolling_ind_current_led = LIGHT_SCROLLING_IND_FIRST_LED;
        }
    }
}

static void Light_Scrolling_Ind_Update_Step_Time(void)
{
    uint8_t local_off_led_number = 0U;

    /*
     * Number of LEDs still OFF:
     *
     * current_led = 0 -> LED0 is active, 7 LEDs are still OFF
     * current_led = 1 -> LED0..LED1 active, 6 LEDs are still OFF
     * ...
     * current_led = 7 -> all LEDs active, 0 LEDs are OFF
     *
     * To avoid division by zero when all LEDs are ON, use 1 as minimum.
     */
    if (light_scrolling_ind_current_led < LIGHT_SCROLLING_IND_LAST_LED)
    {
        local_off_led_number =
            (uint8_t)(LIGHT_SCROLLING_IND_LAST_LED - light_scrolling_ind_current_led);
    }
    else
    {
        local_off_led_number = 1U;
    }

    light_scrolling_ind_step_time_ms =
        (uint16_t)(LIGHT_SCROLLING_IND_BASE_TIME_MS / local_off_led_number);

    if (light_scrolling_ind_step_time_ms < LIGHT_SCROLLING_IND_TASK_TIME_MS)
    {
        light_scrolling_ind_step_time_ms = LIGHT_SCROLLING_IND_TASK_TIME_MS;
    }
}

static s_Light_Pixel_PWM_Command Light_Scrolling_Ind_Get_PWM_Command_Off(void)
{
    s_Light_Pixel_PWM_Command local_command;

    local_command.pixels_25.byte = 0x00U;
    local_command.pixels_50.byte = 0x00U;
    local_command.pixels_75.byte = 0x00U;
    local_command.pixels_100.byte = 0x00U;

    return local_command;
}

static u_Light_Pixel Light_Scrolling_Ind_Get_Pixels_Until_Current(uint8_t current_led)
{
    u_Light_Pixel local_pixels;
    uint8_t local_index = 0U;

    local_pixels.byte = 0x00U;

    if (current_led >= LIGHT_SCROLLING_IND_LED_NUMBER)
    {
        current_led = LIGHT_SCROLLING_IND_LAST_LED;
    }

    for (local_index = 0U; local_index <= current_led; local_index++)
    {
        local_pixels.byte |= (uint8_t)(1U << local_index);
    }

    return local_pixels;
}

/* ========================================================= */
/* Global Functions                                          */
/* ========================================================= */

void Init_Light_Scrolling_Ind(void)
{
    Light_Scrolling_Ind_Reset_Output();
    Light_Scrolling_Ind_Reset_Internal_Data();
}

void Light_Scrolling_Ind_Run_10ms(void)
{
    if (false == light_scrolling_ind_active)
    {
        light_scrolling_ind_active = true;

        light_scrolling_ind_timer_ms = 0U;
        light_scrolling_ind_current_led = LIGHT_SCROLLING_IND_FIRST_LED;
    }

    Light_Scrolling_Ind_Calculate();
}

void Light_Scrolling_Ind_Stop(void)
{
    Light_Scrolling_Ind_Reset_Output();
    Light_Scrolling_Ind_Reset_Internal_Data();
}

s_Light_Pixel_PWM_Command Light_Scrolling_Ind_Get_Output(void)
{
    return light_scrolling_ind_output;
}

bool Light_Scrolling_Ind_Is_Active(void)
{
    return light_scrolling_ind_active;
}