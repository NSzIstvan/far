/*
 * Light_Animation.h
 *
 * Created on: Jul 13, 2026
 * Author: nsist
 */

#ifndef HEADLAMP_APPLICATION_LIGHT_APP_LIGHT_APP_INCLUDE_Light_Animation.h_
#define HEADLAMP_APPLICATION_LIGHT_APP_LIGHT_APP_INCLUDE_Light_Animation.h_

#include <stdbool.h>
#include <stdint.h>

#include "../HeadLamp/RTE/RTE_Types.h"

/* ========================================================= */
/* Types                                                     */
/* ========================================================= */

typedef enum
{
    LIGHT_ANIMATION_NO_REQ = 0U,
    LIGHT_ANIMATION_WELCOME_REQ,
    LIGHT_ANIMATION_GOODBYE_REQ
} e_Light_Animation_Req;

typedef struct
{
    /*
     * Pixel PWM commands for LED groups controlled by shift registers.
     *
     * Each command contains:
     * - pixels_25
     * - pixels_50
     * - pixels_75
     * - pixels_100
     */
    s_Light_Pixel_PWM_Command drl_pos_command;
    s_Light_Pixel_PWM_Command high_beam_command;
    s_Light_Pixel_PWM_Command ti_command;

    /*
     * LowBeam and Fog are simple channels, not 8-pixel groups.
     */
    uint8_t low_beam_duty;
    uint8_t fog_duty;

    bool animation_active;
    bool animation_finished;
} s_Light_Animation_Output;

/* ========================================================= */
/* Global Function Prototypes                               */
/* ========================================================= */

void Init_Light_Animation(void);

void Light_Animation_Start_Welcome(void);
void Light_Animation_Start_Goodbye(void);

void Light_Animation_Stop(void);

void Light_Animation_Run_10ms(void);

s_Light_Animation_Output Light_Animation_Get_Output(void);

bool Light_Animation_Is_Active(void);
bool Light_Animation_Is_Finished(void);

#endif /* HEADLAMP_APPLICATION_LIGHT_APP_LIGHT_APP_INCLUDE_Light_Animation.h_ */