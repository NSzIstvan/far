/*
 * Motor_App.c
 *
 * Created on: Jul 14, 2026
 * Author: nsist
 */

#include "../include/Motor_App.h"
#include "../HeadLamp/RTE/RTE.h"

#include <stdbool.h>
#include <stdint.h>

/* ========================================================= */
/* Defines                                                   */
/* ========================================================= */

#define MOTOR_APP_POSITION_MIN                  0U
#define MOTOR_APP_POSITION_MAX                  63U
#define MOTOR_APP_POSITION_CENTER               32U

#define MOTOR_APP_FULL_TRAVEL_STEPS             64U
#define MOTOR_APP_CENTERING_STEPS               32U

#define MOTOR_APP_NO_STEPS                      0U

#define MOTOR_APP_DIRECTION_DOWN                0U
#define MOTOR_APP_DIRECTION_UP                  1U

#define MOTOR_APP_MANUAL_INPUT_MAX_VALUE        63U
#define MOTOR_APP_SENSOR_INPUT_MAX_VALUE        63U

#define MOTOR_APP_STUCK_TIMEOUT_MS              2000U
#define MOTOR_APP_CYCLIC_TASK_PERIOD_MS         20U
#define MOTOR_APP_NUMBER_RETRIES                3U

/* ========================================================= */
/* Local Types                                               */
/* ========================================================= */

typedef enum
{
    MOTOR_APP_REF_TO_MIN_REQ = 0U,
    MOTOR_APP_REF_TO_MIN_WAIT,
    MOTOR_APP_REF_TO_MAX_REQ,
    MOTOR_APP_REF_TO_MAX_WAIT,
    MOTOR_APP_REF_TO_CENTER_REQ,
    MOTOR_APP_REF_TO_CENTER_WAIT,
    MOTOR_APP_NORMAL_STATE
} e_Motor_App_State;

typedef enum
{
    MOTOR_APP_MANUAL_MODE = 0U,
    MOTOR_APP_AUTO_MODE
} e_Motor_App_Mode;

typedef struct
{
    /*
     * Manual leveling value from Command_Control.
     *
     * Expected range:
     * 0  -> one mechanical end
     * 63 -> opposite mechanical end
     */
    uint8_t manual_level_value;

    /*
     * Auto leveling value from Sensor_Control.
     *
     * Expected range:
     * 0  -> one mechanical end
     * 63 -> opposite mechanical end
     */
    uint8_t sensor_level_value;

    /*
     * Auto request from Light_App.
     *
     * false -> manual motor control from potentiometer
     * true  -> auto motor control from inclination sensors
     */
    bool auto_motor_request;
} s_Motor_App_Input_Data;

typedef struct
{
    bool move_request;

    uint8_t direction;
    uint8_t steps;

    uint8_t actual_position;
    uint8_t target_position;
    uint8_t pending_target_position;

    e_Motor_App_Mode control_mode;
} s_Motor_App_Output_Data;

/* ========================================================= */
/* Local Data                                                */
/* ========================================================= */

static s_Motor_App_Input_Data motor_app_input_data;
static s_Motor_App_Output_Data motor_app_output_data;

static e_Motor_App_State motor_app_state = MOTOR_APP_REF_TO_MIN_REQ;
static uint16_t motor_stuck_timer = 0;
static uint8_t motor_app_number_of_retries = 0;

static bool motor_app_is_initialized = false;
static bool motor_app_waiting_for_normal_movement = false;

/* ========================================================= */
/* Local Function Prototypes                                 */
/* ========================================================= */

static void Motor_App_Reset_Input_Data(void);
static void Motor_App_Reset_Output_Data(void);
static void Motor_App_Reset_Internal_Data(void);

static void Motor_App_Read_Input_Data(void);
static void Motor_App_Calculate(void);
static void Motor_App_Transmit_Output_Data(void);

static void Motor_App_Calculate_Referentiation(void);
static void Motor_App_Calculate_Normal_Mode(void);

static void Motor_App_Set_Move_Request(uint8_t direction, uint8_t steps);
static void Motor_App_Clear_Move_Request(void);

static uint8_t Motor_App_Get_Target_Position(void);
static uint8_t Motor_App_Limit_Position(uint8_t position);
static uint8_t Motor_App_Calculate_Steps(uint8_t actual_position,
                                         uint8_t target_position);
static uint8_t Motor_App_Calculate_Direction(uint8_t actual_position,
                                             uint8_t target_position);

static bool Motor_App_Is_Motor_Movement_Finished(void);

/* ========================================================= */
/* Local Functions                                           */
/* ========================================================= */

static void Motor_App_Reset_Input_Data(void)
{
    motor_app_input_data.manual_level_value = MOTOR_APP_POSITION_CENTER;
    motor_app_input_data.sensor_level_value = MOTOR_APP_POSITION_CENTER;
    motor_app_input_data.auto_motor_request = false;
}

static void Motor_App_Reset_Output_Data(void)
{
    motor_app_output_data.move_request = false;

    motor_app_output_data.direction = MOTOR_APP_DIRECTION_DOWN;
    motor_app_output_data.steps = MOTOR_APP_NO_STEPS;

    /*
     * At power-up, the real position is unknown.
     * We initialize the software position to center only as a safe default.
     * It becomes valid only after referentiation is finished.
     */
    motor_app_output_data.actual_position = MOTOR_APP_POSITION_CENTER;
    motor_app_output_data.target_position = MOTOR_APP_POSITION_CENTER;
    motor_app_output_data.pending_target_position = MOTOR_APP_POSITION_CENTER;

    motor_app_output_data.control_mode = MOTOR_APP_MANUAL_MODE;
}

static void Motor_App_Reset_Internal_Data(void)
{
    motor_app_state = MOTOR_APP_REF_TO_MIN_REQ;

    motor_app_is_initialized = false;
    motor_app_waiting_for_normal_movement = false;
    motor_stuck_timer = 0;
}

static void Motor_App_Read_Input_Data(void)
{
    /*
     * Command_Control -> Motor_App
     *
     * Manual leveling potentiometer.
     * Requirement says Command_Control should map this on a 64-step scale.
     */
    motor_app_input_data.manual_level_value = RTE_Read_Leveling_Pot_Value();

    /*
     * Sensor_Control -> Motor_App
     *
     * Inclination / auto leveling value.
     */
    motor_app_input_data.sensor_level_value = RTE_Read_Leveling_Sensor_Angle();

    /*
     * Light_App -> Motor_App
     *
     * Auto control request.
     */
    motor_app_input_data.auto_motor_request = RTE_Read_Motor_Command_Auto();
}

static void Motor_App_Calculate(void)
{
    Motor_App_Clear_Move_Request();

    if (MOTOR_APP_NORMAL_STATE != motor_app_state)
    {
        Motor_App_Calculate_Referentiation();
    }
    else
    {
        Motor_App_Calculate_Normal_Mode();
    }
}

static void Motor_App_Calculate_Referentiation(void)
{
    /*
     * Referentiation logic:
     *
     * 1. Move 64 steps DOWN  -> force one mechanical end.
     * 2. Move 64 steps UP    -> force the opposite mechanical end.
     * 3. Move 32 steps DOWN  -> go back to center.
     *
     * After this sequence, software can trust:
     * actual_position = MOTOR_APP_POSITION_CENTER.
     */
    static e_Motor_App_State local_motor_app_state = MOTOR_APP_REF_TO_MIN_REQ;

    switch (motor_app_state)
    {
        case MOTOR_APP_REF_TO_MIN_REQ:
        {
            if (true == Motor_App_Is_Motor_Movement_Finished())
            {
                Motor_App_Set_Move_Request(MOTOR_APP_DIRECTION_DOWN,
                                           MOTOR_APP_FULL_TRAVEL_STEPS);

                motor_app_state = MOTOR_APP_REF_TO_MIN_WAIT;
            }
            break;
        }

        case MOTOR_APP_REF_TO_MIN_WAIT:
        {
            if (true == Motor_App_Is_Motor_Movement_Finished())
            {
                motor_app_output_data.actual_position = MOTOR_APP_POSITION_MIN;
                motor_app_state = MOTOR_APP_REF_TO_MAX_REQ;
            }
            break;
        }

        case MOTOR_APP_REF_TO_MAX_REQ:
        {
            if (true == Motor_App_Is_Motor_Movement_Finished())
            {
                Motor_App_Set_Move_Request(MOTOR_APP_DIRECTION_UP,
                                           MOTOR_APP_FULL_TRAVEL_STEPS);

                motor_app_state = MOTOR_APP_REF_TO_MAX_WAIT;
            }
            break;
        }

        case MOTOR_APP_REF_TO_MAX_WAIT:
        {
            if (true == Motor_App_Is_Motor_Movement_Finished())
            {
                motor_app_output_data.actual_position = MOTOR_APP_POSITION_MAX;
                motor_app_state = MOTOR_APP_REF_TO_CENTER_REQ;
            }
            break;
        }

        case MOTOR_APP_REF_TO_CENTER_REQ:
        {
            if (true == Motor_App_Is_Motor_Movement_Finished())
            {
                Motor_App_Set_Move_Request(MOTOR_APP_DIRECTION_DOWN,
                                           MOTOR_APP_CENTERING_STEPS);

                motor_app_state = MOTOR_APP_REF_TO_CENTER_WAIT;
            }
            break;
        }

        case MOTOR_APP_REF_TO_CENTER_WAIT:
        {
            if (true == Motor_App_Is_Motor_Movement_Finished())
            {
                motor_app_output_data.actual_position = MOTOR_APP_POSITION_CENTER;
                motor_app_output_data.target_position = MOTOR_APP_POSITION_CENTER;
                motor_app_output_data.pending_target_position = MOTOR_APP_POSITION_CENTER;

                motor_app_state = MOTOR_APP_NORMAL_STATE;
            }
            break;
        }

        case MOTOR_APP_NORMAL_STATE:
        default:
        {
            motor_app_state = MOTOR_APP_NORMAL_STATE;
            break;
        }
    }

    if (MOTOR_APP_NORMAL_STATE != motor_app_state)
    {
        if (motor_app_state == local_motor_app_state)
        {
            motor_stuck_timer += MOTOR_APP_CYCLIC_TASK_PERIOD_MS;
        }
        else
        {
            motor_stuck_timer = 0;       
        }
    }
    else
    {
        motor_stuck_timer = 0;
    }

    local_motor_app_state = motor_app_state;
}

static void Motor_App_Calculate_Normal_Mode(void)
{
    uint8_t local_target_position = MOTOR_APP_POSITION_CENTER;
    uint8_t local_steps = MOTOR_APP_NO_STEPS;
    uint8_t local_direction = MOTOR_APP_DIRECTION_DOWN;

    if (true == motor_app_waiting_for_normal_movement)
    {
        if (true == Motor_App_Is_Motor_Movement_Finished())
        {
            motor_app_output_data.actual_position =
                motor_app_output_data.pending_target_position;

            motor_app_waiting_for_normal_movement = false;
        }
    }
    else
    {
        if (true == Motor_App_Is_Motor_Movement_Finished())
        {
            local_target_position = Motor_App_Get_Target_Position();

            motor_app_output_data.target_position = local_target_position;

            local_steps =
                Motor_App_Calculate_Steps(motor_app_output_data.actual_position,
                                          motor_app_output_data.target_position);

            if (MOTOR_APP_NO_STEPS != local_steps)
            {
                local_direction =
                    Motor_App_Calculate_Direction(motor_app_output_data.actual_position,
                                                  motor_app_output_data.target_position);

                Motor_App_Set_Move_Request(local_direction, local_steps);

                motor_app_output_data.pending_target_position =
                    motor_app_output_data.target_position;

                motor_app_waiting_for_normal_movement = true;
            }
        }
    }
}

static void Motor_App_Transmit_Output_Data(void)
{
    if (true == motor_app_output_data.move_request)
    {
        /*
         * New expected RTE interface:
         *
         * Motor_App -> Motor_Control
         * direction + steps
         */
        RTE_Call_Move_Motor_Steps(motor_app_output_data.direction,
                                  motor_app_output_data.steps);
    }
}

static void Motor_App_Set_Move_Request(uint8_t direction, uint8_t steps)
{
    motor_app_output_data.move_request = true;
    motor_app_output_data.direction = direction;
    motor_app_output_data.steps = steps;
}

static void Motor_App_Clear_Move_Request(void)
{
    motor_app_output_data.move_request = false;
    motor_app_output_data.direction = MOTOR_APP_DIRECTION_DOWN;
    motor_app_output_data.steps = MOTOR_APP_NO_STEPS;
}

static uint8_t Motor_App_Get_Target_Position(void)
{
    uint8_t local_target_position = MOTOR_APP_POSITION_CENTER;

    if (true == motor_app_input_data.auto_motor_request)
    {
        motor_app_output_data.control_mode = MOTOR_APP_AUTO_MODE;

        local_target_position =
            Motor_App_Limit_Position(motor_app_input_data.sensor_level_value);
    }
    else
    {
        motor_app_output_data.control_mode = MOTOR_APP_MANUAL_MODE;

        local_target_position =
            Motor_App_Limit_Position(motor_app_input_data.manual_level_value);
    }

    return local_target_position;
}

static uint8_t Motor_App_Limit_Position(uint8_t position)
{
    uint8_t local_position = position;

    if (local_position > MOTOR_APP_POSITION_MAX)
    {
        local_position = MOTOR_APP_POSITION_MAX;
    }
    else
    {
        /* Position is already valid. */
    }

    return local_position;
}

static uint8_t Motor_App_Calculate_Steps(uint8_t actual_position,
                                         uint8_t target_position)
{
    uint8_t local_steps = MOTOR_APP_NO_STEPS;

    if (target_position >= actual_position)
    {
        local_steps = (uint8_t)(target_position - actual_position);
    }
    else
    {
        local_steps = (uint8_t)(actual_position - target_position);
    }

    return local_steps;
}

static uint8_t Motor_App_Calculate_Direction(uint8_t actual_position,
                                             uint8_t target_position)
{
    uint8_t local_direction = MOTOR_APP_DIRECTION_DOWN;

    if (target_position >= actual_position)
    {
        local_direction = MOTOR_APP_DIRECTION_UP;
    }
    else
    {
        local_direction = MOTOR_APP_DIRECTION_DOWN;
    }

    return local_direction;
}

static bool Motor_App_Is_Motor_Movement_Finished(void)
{
    bool local_movement_finished = false;

    /*
     * New expected RTE interface:
     *
     * Motor_Control -> Motor_App
     * true  -> Motor_Control is idle / previous movement finished
     * false -> Motor_Control is still moving
     */
    local_movement_finished = RTE_Read_Motor_Movement_Finished();

    return local_movement_finished;
}

/* ========================================================= */
/* Global Functions                                          */
/* ========================================================= */

void Init_Motor_App(void)
{
    Motor_App_Reset_Input_Data();
    Motor_App_Reset_Output_Data();
    Motor_App_Reset_Internal_Data();

    motor_app_number_of_retries = 0;

    motor_app_is_initialized = true;
}

void Run_Motor_App_Main_20ms(void)
{
    if (false == motor_app_is_initialized)
    {
        Init_Motor_App();
    }
    else
    {   
        if (motor_stuck_timer >= MOTOR_APP_STUCK_TIMEOUT_MS)
        {
            Motor_App_Reset_Internal_Data();
            motor_app_number_of_retries++;
        }

        if (motor_app_number_of_retries < MOTOR_APP_NUMBER_RETRIES)
        {
            Motor_App_Read_Input_Data();
            Motor_App_Calculate();
            Motor_App_Transmit_Output_Data();
        }
    }
}
