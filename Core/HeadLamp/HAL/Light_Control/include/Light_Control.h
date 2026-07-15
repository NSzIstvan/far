/*
 * Light_Control.h
 *
 * Hardware abstraction for all headlamp light outputs.
 */

#ifndef HEADLAMP_HAL_LIGHT_CONTROL_INCLUDE_LIGHT_CONTROL_H_
#define HEADLAMP_HAL_LIGHT_CONTROL_INCLUDE_LIGHT_CONTROL_H_

#include <stdbool.h>
#include <stdint.h>

#include "../../../RTE/RTE_Types.h"

#define LIGHT_CONTROL_PIXEL_COUNT              (8U)
#define LIGHT_CONTROL_PWM_PHASE_COUNT          (4U)
#define LIGHT_CONTROL_DUTY_MIN_PERCENT         (0U)
#define LIGHT_CONTROL_DUTY_MAX_PERCENT         (100U)

typedef enum
{
    LIGHT_CONTROL_GROUP_POS_DRL = 0,
    LIGHT_CONTROL_GROUP_HIGH_BEAM,
    LIGHT_CONTROL_GROUP_TURN_INDICATOR,
    LIGHT_CONTROL_GROUP_COUNT
} Light_Control_GroupType;

/** Initializes the light-control state and leaves every physical light output in its safe OFF state. */
void Init_Light_Control(void);

/** Publishes the initialization state and performs low-frequency supervision of the light-control module. */
void Run_Light_Control_Main_10ms(void);

/** Executes one software-PWM phase and refreshes the three 8-channel light groups. Call every 1 ms. */
void Run_Light_Control_PWM_Main_1ms(void);

/** Stores the requested per-pixel PWM command for the combined position/DRL light group. */
void Set_Light_Func_POS_DRL_Command(s_Light_Pixel_PWM_Command pixel_duties);

/** Stores and applies the requested low-beam duty cycle in the range 0...100 percent. */
void Set_Light_Func_LowBeam_Command(uint8_t duty_cycle);

/** Stores the requested per-pixel PWM command for the high-beam light group. */
void Set_Light_Func_HighBeam_Command(s_Light_Pixel_PWM_Command pixel_duties);

/** Stores the requested per-pixel PWM command for turn-indicator and hazard operation. */
void Set_Light_Func_TI_Hazard_Command(s_Light_Pixel_PWM_Command pixel_duties);

/** Stores and applies the requested fog-light duty cycle in the range 0...100 percent. */
void Set_Light_Func_FOG_Command(uint8_t duty_cycle);

/** Forces all controlled light outputs OFF and clears all pending light commands. */
void Light_Control_All_Outputs_Off(void);

/** Returns true after Light Control has completed its initialization sequence. */
bool Light_Control_Is_Init_Done(void);

#endif /* HEADLAMP_HAL_LIGHT_CONTROL_INCLUDE_LIGHT_CONTROL_H_ */