/**
  ******************************************************************************
  * File Name          : BarometerTask.cpp
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
#include "ThermocoupleTask.hpp"
#include "main.h"
#include "Data.h"
#include "DebugTask.hpp"
#include "Task.hpp"


/* Macros --------------------------------------------------------------------*/

/* Structs -------------------------------------------------------------------*/

/* Constants -----------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/

/* Prototypes ----------------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**
 * @brief Default constructor
 */
ThermocoupleTask::ThermocoupleTask() : Task(TASK_THERMOCOUPLE_QUEUE_DEPTH_OBJS)
{
	//Data is stored locally in object not using MALOC
}

/**
 * @brief Creates a task for the FreeRTOS Scheduler
 */
void ThermocoupleTask::InitTask() //ETHAN NOTE: IDK IF WE ARE STILL DOING THIS
{
    // Make sure the task is not already initialized
    SOAR_ASSERT(rtTaskHandle == nullptr, "Cannot initialize Thermocouple task twice");

    // Start the task
    BaseType_t rtValue =
        xTaskCreate((TaskFunction_t)ThermocoupleTask::RunTask,
            (const char*)"ThermocoupleTask",
            (uint16_t)TASK_THERMOCOUPLE_STACK_DEPTH_WORDS,
            (void*)this,
            (UBaseType_t)TASK_THERMOCOUPLE_PRIORITY,
            (TaskHandle_t*)&rtTaskHandle);

    //Ensure creation succeeded
    SOAR_ASSERT(rtValue == pdPASS, "ThermocoupleTask::InitTask() - xTaskCreate() failed");
}

/**
 * @brief ThermocoupleTask run loop
 * @param pvParams Currently unused task context
 */
void ThermocoupleTask::Run(void * pvParams)
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
void ThermocoupleTask::HandleCommand(Command& cm)
{

	//ETHAN NOTE: IDK WHAT TODO IS ABOUT
    //TODO: Since this task will stall for a few milliseconds, we may need a way to eat the whole queue (combine similar eg. REQUEST commands and eat to WDG command etc)
    //TODO: Maybe a HandleEvtQueue instead that takes in the whole queue and eats the whole thing in order of non-blocking to blocking

    //Switch for the GLOBAL_COMMAND
    switch (cm.GetCommand()) {
    case REQUEST_COMMAND: {
        HandleRequestCommand(cm.GetTaskCommand()); //Sends task specific request command to task request handler
    }
    case TASK_SPECIFIC_COMMAND: {
        break; //No task specific commands need
    }
    default:
        SOAR_PRINT("ThermocoupleTask - Received Unsupported Command {%d}\n", cm.GetCommand());
        break;
    }

    //No matter what we happens, we must reset allocated data
    cm.Reset();
}

/**
 * @brief Handles a Request Command
 * @param taskCommand The command to handle
 */
void ThermocoupleTask::HandleRequestCommand(uint16_t taskCommand)
{
    //Switch for task specific command within DATA_COMMAND
    switch (taskCommand) {
    case THERMOCOUPLE_REQUEST_NEW_SAMPLE:
    	SampleThermocouple();
        break;
    case THERMOCOUPLE_REQUEST_TRANSMIT: //This is where we will actually be sending data
        SOAR_PRINT("Stubbed: Thermocouple task transmit not implemented\n");
        break;
    case THERMOCOUPLE_REQUEST_DEBUG: //Temporary data debug sender
        SOAR_PRINT("\t-- Barometer Data --\n");
        break;s
    default:
        SOAR_PRINT("UARTTask - Received Unsupported REQUEST_COMMAND {%d}\n", taskCommand);
        break;
    }
}

/**
 * @brief This method receives the voltage reading through spi from the thermocouple readings
 */

void ThermocoupleTask::SampleThermocouple()
{





}









