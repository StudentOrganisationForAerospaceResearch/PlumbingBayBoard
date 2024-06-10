/**
  ******************************************************************************
  * File Name          : PressureTransducerTask.cpp
  *
  *    Source Info           : Based on Andromeda V3.31 Implementation
  *                         Andromeda_V3.31_Legacy/Core/Src/ReadBarometer.c
  *
  * Description        : This file contains constants and functions designed to
  *                      obtain accurate pressure and temperature readings from
  *                      the MS5607-02BA03 barometer on the flight board. A
  *                      thread task is included that will constantly loop,
  *                      reading and updating the passed BarometerData struct.
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "PressureTransducerTask.hpp"
#include "FastLogManager.hpp"
#include "main.h"
#include "Data.h"
#include "DebugTask.hpp"
#include "Task.hpp"
#include <time.h>
#include "PBBProtocolTask.hpp"


/* Macros --------------------------------------------------------------------*/

/* Structs -------------------------------------------------------------------*/

/* Constants -----------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/

/* Prototypes ----------------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/
/**
 * @brief Default constructor, sets and sets up storage for member variables
 */
PressureTransducerTask::PressureTransducerTask() : Task(TASK_PRESSURE_TRANSDUCER_QUEUE_DEPTH_OBJS)
{
}

/**
 * @brief Creates a task for the FreeRTOS Scheduler
 */
void PressureTransducerTask::InitTask()
{
    // Make sure the task is not already initialized
    SOAR_ASSERT(rtTaskHandle == nullptr, "Cannot initialize PT task twice");

    // Start the task
    BaseType_t rtValue =
        xTaskCreate((TaskFunction_t)PressureTransducerTask::RunTask,
            (const char*)"PTTask",
            (uint16_t)TASK_PRESSURE_TRANSDUCER_STACK_DEPTH_WORDS,
            (void*)this,
            (UBaseType_t)TASK_PRESSURE_TRANSDUCER_PRIORITY,
            (TaskHandle_t*)&rtTaskHandle);

    //Ensure creation succeded
    SOAR_ASSERT(rtValue == pdPASS, "PressureTransducerTask::InitTask() - xTaskCreate() failed");
}

/**
 * @brief PresssureTransducerTask run loop
 * @param pvParams Currently unused task context
 */
void PressureTransducerTask::Run(void * pvParams)
{
    Command cm;
    while (1) {
        // Run the FastLogManager
        FastLogManager::Inst().Run();

        // Poll for a command and handle it if received
        if(qEvtQueue->Receive(cm)) {
            HandleCommand(cm);
        }
    }
}

/**
 * @brief Handles a command
 * @param cm Command reference to handle
 */
void PressureTransducerTask::HandleCommand(Command& cm)
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
        SOAR_PRINT("PressureTransducerTASK - Received Unsupported Command {%d}\n", cm.GetCommand());
        break;
    }

    //No matter what we happens, we must reset allocated data
    cm.Reset();
}

/**
 * @brief Handles a Request Command
 * @param taskCommand The command to handle
 */
void PressureTransducerTask::HandleRequestCommand(uint16_t taskCommand)
{
    //Switch for task specific command within DATA_COMMAND
    switch (taskCommand) {
    case PT_REQUEST_NEW_SAMPLE:
        SamplePressureTransducer();
        break;
    case PT_REQUEST_TRANSMIT:
    	TransmitProtocolPressureData();
        break;
    default:
        SOAR_PRINT("UARTTask - Received Unsupported REQUEST_COMMAND {%d}\n", taskCommand);
        break;
    }
}

