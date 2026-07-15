/*
 * Light_Scrolling_Ind.h
 *
 * Created on: Jul 14, 2026
 * Author: nsist
 */

#ifndef HEADLAMP_APPLICATION_LIGHT_APP_LIGHT_APP_INCLUDE_Light_Scrolling_Ind.h_
#define HEADLAMP_APPLICATION_LIGHT_APP_LIGHT_APP_INCLUDE_Light_Scrolling_Ind.h_

#include <stdbool.h>
#include <stdint.h>

#include "../HeadLamp/RTE/RTE_Types.h"

/* ========================================================= */
/* Global Function Prototypes                               */
/* ========================================================= */

/*
 * Initializes all internal scrolling indicator data.
 */
void Init_Light_Scrolling_Ind(void);

/*
 * Cyclic 10ms runnable.
 *
 * Calculates the scrolling turn indicator output.
 * The output shall be read by Light_App using Light_Scrolling_Ind_Get_Output().
 */
void Light_Scrolling_Ind_Run_10ms(void);

/*
 * Stops the scrolling indicator and resets the output to OFF.
 */
void Light_Scrolling_Ind_Stop(void);

/*
 * Returns the current scrolling indicator output.
 *
 * Current implementation:
 * - only one indicator direction is implemented
 * - it uses the full TI LED group
 * - right indicator split is left as TODO for Parameter file functionality
 */
s_Light_Pixel_PWM_Command Light_Scrolling_Ind_Get_Output(void);

/*
 * Returns true while the scrolling indicator is active.
 */
bool Light_Scrolling_Ind_Is_Active(void);

#endif /* HEADLAMP_APPLICATION_LIGHT_APP_LIGHT_APP_INCLUDE_Light_Scrolling_Ind.h_ */