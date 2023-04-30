/**
 ******************************************************************************
 * File Name          : ServoTask.cpp
 * Description        : Primary Watchdog task, default task for the system.
 ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "ServoTask.hpp"
#include "main.h"
#include "DebugTask.hpp"
#include "Task.hpp"
#include "GPIO.hpp"


/* Macros --------------------------------------------------------------------*/

/* Structs -------------------------------------------------------------------*/

/* Constants -----------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/

/* Prototypes ----------------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/
/**
 * @brief Default constructor, sets and sets up storage for member variables
 */
ServoTask::ServoTask() : Task(TASK_SERVO_QUEUE_DEPTH_OBJS)
{
}

/**
 * @brief Creates a task for the FreeRTOS Scheduler
 */
void ServoTask::InitTask()
{
    // Make sure the task is not already initialized
    SOAR_ASSERT(rtTaskHandle == nullptr, "Cannot initialize Servo task twice");

    // Start the task
    BaseType_t rtValue =
        xTaskCreate((TaskFunction_t)ServoTask::RunTask,
            (const char*)"ServoTask",
            (uint16_t)TASK_SERVO_STACK_DEPTH_WORDS,
            (void*)this,
            (UBaseType_t)TASK_SERVO_PRIORITY,
            (TaskHandle_t*)&rtTaskHandle);

    //Ensure creation succeded
    SOAR_ASSERT(rtValue == pdPASS, "ServoTask::InitTask() - xTaskCreate() failed");
}

/**
 * @brief PresssureTransducerTask run loop
 * @param pvParams Currently unused task context
 */
void ServoTask::Run(void * pvParams)
{
    while (1) {
        Command cm;

        //Wait forever for a command
        qEvtQueue->ReceiveWait(cm);

        //Process the command
        HandleCommand(cm);
    }
}

/**
 * @brief Handles a command
 * @param cm Command reference to handle
 */
void ServoTask::HandleCommand(Command& cm)
{
    //TODO: Since this task will stall for a few milliseconds, we may need a way to eat the whole queue (combine similar eg. REQUEST commands and eat to WDG command etc)
    //TODO: Maybe a HandleEvtQueue instead that takes in the whole queue and eats the whole thing in order of non-blocking to blocking

    //Switch for the GLOBAL_COMMAND
    switch (cm.GetCommand()) {
    case REQUEST_COMMAND: {
        HandleRequestCommand(cm.GetTaskCommand());
    }
    case TASK_SPECIFIC_COMMAND: {
        break;
    }
    default:
        SOAR_PRINT("ServoTASK - Received Unsupported Command {%d}\n", cm.GetCommand());
        break;
    }

    //No matter what we happens, we must reset allocated data
    cm.Reset();
}

/**
 * @brief Handles a Request Command
 * @param taskCommand The command to handle
 */
void ServoTask::HandleRequestCommand(uint16_t taskCommand)
{
    //Switch for task specific command within DATA_COMMAND
    switch (taskCommand) {
    case SERVO_OPEN:
    	SOAR_PRINT("Servo Open");
        Open();
        break;
    case SERVO_CLOSE:
    	SOAR_PRINT("Servo Closed");
    	Close();
        break;
    default:
        SOAR_PRINT("UARTTask - Received Unsupported REQUEST_COMMAND {%d}\n", taskCommand);
        break;
    }
}


void ServoTask::Open()
{
	HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_4);
	htim8.Instance->CCR4 = 103;
	while (1) {
	  htim8.Instance->CCR4 = 50; //55
	  osDelay(10000);
	  htim8.Instance->CCR4 = 103;
	  osDelay(10000);
	}
}


void ServoTask::Close()
{
	HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_4);
	htim8.Instance->CCR4 = 103;
	while (1) {
	  htim8.Instance->CCR4 = 50; //55
	  osDelay(10000);
	  htim8.Instance->CCR4 = 103;
	  osDelay(10000);
	}
}
