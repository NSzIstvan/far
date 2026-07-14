/*
 * Light_animation.h
 *
 * Created on: Jul 13, 2026
 * Author: nsist
 */

#ifndef HEADLAMP_APPLICATION_LIGHT_APP_LIGHT_APP_INCLUDE_LIGHT_ANIMATION_H_
#define HEADLAMP_APPLICATION_LIGHT_APP_LIGHT_APP_INCLUDE_LIGHT_ANIMATION_H_

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
     * Separate pixel control for DRL/POS and HighBeam.
     * These shall not be reused for the same purpose.
     */
    u_Light_Pixel drl_pos_pixels;
    u_Light_Pixel high_beam_pixels;
    u_Light_Pixel ti_hazard_pixels;

    uint8_t drl_pos_duty;
    uint8_t low_beam_duty;
    uint8_t high_beam_duty;
    uint8_t ti_hazard_duty;
    uint8_t fog_duty;

    bool animation_active;
    bool animation_finished;
} s_Light_Animation_Output;

/* ========================================================= */
/* Global Function Prototypes                               */
/* ========================================================= */

/*
 * Initializes all internal Light_animation data.
 */
void Init_Light_Animation(void);

/*
 * Starts welcome animation.
 * This shall be called by Light_App after Light_Control init is finished.
 */
void Light_Animation_Start_Welcome(void);

/*
 * Starts goodbye animation.
 * This shall be called by Light_App when OFF state is active for more than 30s.
 */
void Light_Animation_Start_Goodbye(void);

/*
 * Stops current animation and resets animation output.
 */
void Light_Animation_Stop(void);

/*
 * Cyclic 10ms runnable.
 * Calculates the current animation output.
 */
void Light_Animation_Run_10ms(void);

/*
 * Returns the current animation output.
 * Light_App will use this output and forward it to Light_Control through RTE.
 */
s_Light_Animation_Output Light_Animation_Get_Output(void);

/*
 * Returns true while welcome/goodbye animation is running.
 */
bool Light_Animation_Is_Active(void);

/*
 * Returns true when the requested animation finished.
 */
bool Light_Animation_Is_Finished(void);

#endif /* HEADLAMP_APPLICATION_LIGHT_APP_LIGHT_APP_INCLUDE_LIGHT_ANIMATION_H_ */