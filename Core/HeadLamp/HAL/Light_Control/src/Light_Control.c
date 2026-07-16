/*
 * Light_Control.c
 *
 * Hardware-independent light command handling and four-phase software PWM.
 */

#include "../include/Light_Control.h"
#include "../../../RTE/RTE.h"

#include <string.h>

#define LIGHT_CONTROL_OUTPUT_OFF              (0U)
#define LIGHT_CONTROL_ALL_PIXELS_OFF          (0x00U)

/* Four equal PWM phases provide 0%, 25%, 50%, 75% and 100% pixel duty levels. */
#define LIGHT_CONTROL_PWM_PHASE_25             (0U)
#define LIGHT_CONTROL_PWM_PHASE_50             (1U)
#define LIGHT_CONTROL_PWM_PHASE_75             (2U)
#define LIGHT_CONTROL_PWM_PHASE_100            (3U)

typedef struct
{
    s_Light_Pixel_PWM_Command requested_command;
    s_Light_Pixel_PWM_Command active_command;
    bool update_pending;
} Light_Control_PixelGroupStateType;

typedef struct
{
    Light_Control_PixelGroupStateType pixel_group[LIGHT_CONTROL_GROUP_COUNT];
    uint8_t low_beam_duty;
    uint8_t fog_light_duty;
    uint8_t pwm_phase;
    bool init_done;
} Light_Control_StateType;

static Light_Control_StateType Light_Control_State;

/** Limits a percentage command to the valid physical output range of 0...100 percent. */
static uint8_t Light_Control_Clamp_Duty(uint8_t duty_cycle);

/** Copies pending pixel commands to the active PWM command set at a PWM-frame boundary. */
static void Light_Control_Activate_Pending_Commands(void);

/** Builds the eight-bit output mask required for one group during the selected PWM phase. */
static uint8_t Light_Control_Build_Phase_Mask(
    const s_Light_Pixel_PWM_Command *command,
    uint8_t phase);

/** Sends one eight-bit pixel mask to the selected TPIC6595 chain and latches its outputs. */
static void Light_Control_Write_Shift_Register(
    Light_Control_GroupType group,
    uint8_t output_mask);

/** Enables or disables the high-side supply of one multi-pixel light group. */
static void Light_Control_Set_Group_Enable(
    Light_Control_GroupType group,
    bool enable);

/** Applies a 0...100 percent command to the dedicated low-beam PWM output. */
static void Light_Control_Write_Low_Beam_PWM(uint8_t duty_cycle);

/** Applies a 0...100 percent command to the dedicated fog-light PWM output. */
static void Light_Control_Write_Fog_Light_PWM(uint8_t duty_cycle);

/** Refreshes all three pixel groups for the current software-PWM phase. */
static void Light_Control_Refresh_Pixel_Outputs(void);

static void Light_Control_Set_Pixel_Command(Light_Control_GroupType group,
    s_Light_Pixel_PWM_Command pixel_duties);

/** Limits a percentage command to the valid physical output range of 0...100 percent. */
static uint8_t Light_Control_Clamp_Duty(uint8_t duty_cycle)
{
    if (duty_cycle > LIGHT_CONTROL_DUTY_MAX_PERCENT)
    {
        duty_cycle = LIGHT_CONTROL_DUTY_MAX_PERCENT;
    }

    return duty_cycle;
}

/** Copies pending pixel commands to the active PWM command set at a PWM-frame boundary. */
static void Light_Control_Activate_Pending_Commands(void)
{
    uint8_t group_index;

    taskENTER_CRITICAL();

    for (group_index = 0U;
         group_index < (uint8_t)LIGHT_CONTROL_GROUP_COUNT;
         group_index++)
    {
        if (Light_Control_State.pixel_group[group_index].update_pending != false)
        {
            Light_Control_State.pixel_group[group_index].active_command =
                Light_Control_State.pixel_group[group_index].requested_command;
            Light_Control_State.pixel_group[group_index].update_pending = false;
        }
    }

    taskEXIT_CRITICAL();
}

/** Builds the eight-bit output mask required for one group during the selected PWM phase. */
static uint8_t Light_Control_Build_Phase_Mask(
    const s_Light_Pixel_PWM_Command *command,
    uint8_t phase)
{
    uint8_t output_mask = command->pixels_100.byte;

    if (phase <= LIGHT_CONTROL_PWM_PHASE_75)
    {
        output_mask |= command->pixels_75.byte;
    }

    if (phase <= LIGHT_CONTROL_PWM_PHASE_50)
    {
        output_mask |= command->pixels_50.byte;
    }

    if (phase == LIGHT_CONTROL_PWM_PHASE_25)
    {
        output_mask |= command->pixels_25.byte;
    }

    return output_mask;
}

/** Sends one eight-bit pixel mask to the selected TPIC6595 chain and latches its outputs. */
static void Light_Control_Write_Shift_Register(
    Light_Control_GroupType group,
    uint8_t output_mask)
{
    (void)group;
    (void)output_mask;

    /* TO-DO PIN HANDLING NEEDED: Select the requested TPIC6595, shift output_mask via SPI/GPIO, then pulse RCK. */
}

/** Enables or disables the high-side supply of one multi-pixel light group. */
static void Light_Control_Set_Group_Enable(
    Light_Control_GroupType group,
    bool enable)
{
    (void)group;
    (void)enable;

    /* TO-DO PIN HANDLING NEEDED: Drive the PMOS/group-enable pin using the required active polarity. */
}

/** Applies a 0...100 percent command to the dedicated low-beam PWM output. */
static void Light_Control_Write_Low_Beam_PWM(uint8_t duty_cycle)
{
    (void)duty_cycle;

    /* TO-DO PIN HANDLING NEEDED: Write the low-beam timer compare value or low-beam GPIO state. */
}

/** Applies a 0...100 percent command to the dedicated fog-light PWM output. */
static void Light_Control_Write_Fog_Light_PWM(uint8_t duty_cycle)
{
    (void)duty_cycle;

    /* TO-DO PIN HANDLING NEEDED: Write the fog-light timer compare value or fog-light GPIO state. */
}

/**
 * Returns true when at least one pixel in the command has a non-zero duty.
 */
static bool Light_Control_Is_Group_Command_Active(
    const s_Light_Pixel_PWM_Command *command)
{
    return
        (command->pixels_25.byte  != 0U) ||
        (command->pixels_50.byte  != 0U) ||
        (command->pixels_75.byte  != 0U) ||
        (command->pixels_100.byte != 0U);
}

/** Refreshes all three pixel groups for the current software-PWM phase. */
static void Light_Control_Refresh_Pixel_Outputs(void)
{
    uint8_t group_index;

    for (group_index = 0U;
         group_index < (uint8_t)LIGHT_CONTROL_GROUP_COUNT;
         group_index++)
    {
        const s_Light_Pixel_PWM_Command *active_command =
            &Light_Control_State.pixel_group[group_index].active_command;

        const uint8_t output_mask =
            Light_Control_Build_Phase_Mask(active_command,
                Light_Control_State.pwm_phase);

        Light_Control_Set_Group_Enable(
            (Light_Control_GroupType)group_index,
            Light_Control_Is_Group_Command_Active(active_command));

        Light_Control_Write_Shift_Register(
            (Light_Control_GroupType)group_index,output_mask);
    }
}

/** Initializes the light-control state and leaves every physical light output in its safe OFF state. */
void Init_Light_Control(void)
{
    (void)memset(&Light_Control_State, 0, sizeof(Light_Control_State));

    /* TO-DO PIN HANDLING NEEDED: Initialize SPI/GPIO/timer peripherals and configure every light output to its safe OFF state. */

    Light_Control_All_Outputs_Off();
    Light_Control_State.init_done = true;
    RTE_Write_Light_Control_Init_Done(true);
}

/** Publishes the initialization state and performs low-frequency supervision of the light-control module. */
void Run_Light_Control_Main_10ms(void)
{
    RTE_Write_Light_Control_Init_Done(Light_Control_State.init_done);
}

/** Executes one software-PWM phase and refreshes the three 8-channel light groups. Call every 1 ms. */
void Run_Light_Control_PWM_Main_1ms(void)
{
    if (Light_Control_State.init_done == false)
    {
        return;
    }

    if (Light_Control_State.pwm_phase == LIGHT_CONTROL_PWM_PHASE_25)
    {
        Light_Control_Activate_Pending_Commands();
    }

    Light_Control_Refresh_Pixel_Outputs();

    Light_Control_State.pwm_phase++;
    if (Light_Control_State.pwm_phase >= LIGHT_CONTROL_PWM_PHASE_COUNT)
    {
        Light_Control_State.pwm_phase = LIGHT_CONTROL_PWM_PHASE_25;
    }
}

static void Light_Control_Set_Pixel_Command(
    Light_Control_GroupType group,
    s_Light_Pixel_PWM_Command pixel_duties)
{
    taskENTER_CRITICAL();

    Light_Control_State.pixel_group[group].requested_command =
        pixel_duties;

    Light_Control_State.pixel_group[group].update_pending = true;

    taskEXIT_CRITICAL();
}

/** Stores the requested per-pixel PWM command for the combined position/DRL light group. */
void Set_Light_Func_POS_DRL_Command(s_Light_Pixel_PWM_Command pixel_duties)
{
    Light_Control_Set_Pixel_Command(LIGHT_CONTROL_GROUP_POS_DRL, pixel_duties);
}

/** Stores and applies the requested low-beam duty cycle in the range 0...100 percent. */
void Set_Light_Func_LowBeam_Command(uint8_t duty_cycle)
{
    Light_Control_State.low_beam_duty = Light_Control_Clamp_Duty(duty_cycle);
    Light_Control_Write_Low_Beam_PWM(Light_Control_State.low_beam_duty);
}

/** Stores the requested per-pixel PWM command for the high-beam light group. */
void Set_Light_Func_HighBeam_Command(s_Light_Pixel_PWM_Command pixel_duties)
{
    Light_Control_Set_Pixel_Command(LIGHT_CONTROL_GROUP_HIGH_BEAM, pixel_duties);
}

/** Stores the requested per-pixel PWM command for turn-indicator and hazard operation. */
void Set_Light_Func_TI_Hazard_Command(s_Light_Pixel_PWM_Command pixel_duties)
{
    Light_Control_Set_Pixel_Command(LIGHT_CONTROL_GROUP_TURN_INDICATOR, pixel_duties);
}

/** Stores and applies the requested fog-light duty cycle in the range 0...100 percent. */
void Set_Light_Func_FOG_Command(uint8_t duty_cycle)
{
    Light_Control_State.fog_light_duty = Light_Control_Clamp_Duty(duty_cycle);
    Light_Control_Write_Fog_Light_PWM(Light_Control_State.fog_light_duty);
}

/** Forces all controlled light outputs OFF and clears all pending light commands. */
void Light_Control_All_Outputs_Off(void)
{
    uint8_t group_index;

    for (group_index = 0U;
         group_index < (uint8_t)LIGHT_CONTROL_GROUP_COUNT;
         group_index++)
    {
        (void)memset(
            &Light_Control_State.pixel_group[group_index],
            0,
            sizeof(Light_Control_State.pixel_group[group_index]));

        Light_Control_Write_Shift_Register(
            (Light_Control_GroupType)group_index,
            LIGHT_CONTROL_ALL_PIXELS_OFF);
        Light_Control_Set_Group_Enable(
            (Light_Control_GroupType)group_index,
            false);
    }

    Light_Control_State.low_beam_duty = LIGHT_CONTROL_OUTPUT_OFF;
    Light_Control_State.fog_light_duty = LIGHT_CONTROL_OUTPUT_OFF;
    Light_Control_State.pwm_phase = LIGHT_CONTROL_PWM_PHASE_25;

    Light_Control_Write_Low_Beam_PWM(LIGHT_CONTROL_OUTPUT_OFF);
    Light_Control_Write_Fog_Light_PWM(LIGHT_CONTROL_OUTPUT_OFF);
}

/** Returns true after Light Control has completed its initialization sequence. */
bool Light_Control_Is_Init_Done(void)
{
    return Light_Control_State.init_done;
}