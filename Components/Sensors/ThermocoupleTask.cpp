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
#include "DebugTask.hpp"
#include "Task.hpp"


/* Macros --------------------------------------------------------------------*/

/* Structs -------------------------------------------------------------------*/

/* Constants -----------------------------------------------------------------*/

/* Values should not be modified, non-const due to HAL and C++ strictness) ---*/
constexpr int CMD_TIMEOUT = 150; //TODO: Taken from IMU not sure if it needs to be different

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
void ThermocoupleTask::InitTask() //TODO: ETHAN NOTE: IDK IF WE ARE STILL DOING THIS
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
        SOAR_PRINT("\t-- Thermocouple Data --\n");
        ConvertTempuatureData();
        break;
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
	/*DATA FROM MAX31855KASA+T ------------------------------------------------------

	32 bits Memory Map

		D31-D18 : Thermocoupler Temperature Data

			D31 : Sign bit

			D30-D18 : Temperature Value (2's complement) from 2^10 to 2^-2

		D17 : Reserved Bit

		D16 : Fault (if high shows fault is detected, more specific fault messages at D2 - D0)

		D15-D4 :  Internal Temperature Data (reference junction temperature)

			D15 : Sign bit

			D14-D4 : Temperature Value (2's complement) from 2^6 to 2^-4

		D3 : Reserved

		D2-D0 : Fault Detection Bits

			D2 : SCV Fault (displays high if TC shorts to Vcc)

			D1 : SCG Fault (displays high if TC shorts to GND)

			D0 : Thermocouple has no Connection (displays high)

	*///------------------------------------------------------------------------------


	//Storable Data ------------------------------------------------------------------------------

	uint8_t dataBuffer[4];
	//See Above bit mem-map

	//Read ---------------------------------------------------------------------------------------

    //Read From Thermocouple 1 first
	HAL_GPIO_WritePin(TC1_CS__GPIO_Port, TC1_CS__Pin, GPIO_PIN_RESET); //begin read with CS pin low
	HAL_SPI_Receive(SystemHandles::SPI_Thermocouple1, &dataBuffer[0], 4, CMD_TIMEOUT); //Fill the data buffer with data from TC1
	HAL_GPIO_WritePin(TC1_CS__GPIO_Port, TC1_CS__Pin, GPIO_PIN_SET); //end read with setting CS pin to high again


	TC1_Temp_Data = 0xffff & ((dataBuffer[3] << 6) | (dataBuffer[2] >> 2));

	TC1_Internal_Temp_Data = 0xffff & ((dataBuffer[1] << 4) | (dataBuffer[0] >> 4));

	TC1_faultBits = 0xff & (((2 << 4) - 1 ) & ((dataBuffer[2] << 3) & (2 << 1)| ((dataBuffer[0]) & ((2<<2)-1))));
	// (dataBuffer[2] << 3) : this puts D16 from the SPI into D3 of the faultBits variable
	// &(2 << 1) : prunes unwanted surrounding highs
	// | ((dataBuffer[0]) & ((2<<2)-1)) : this or adds only the last 3 bits from the dataBuffer
	// (2 << 4) - 1 : acts as a filter to ensure bits 7-4 are 0


	//Read From Thermocouple 1 first
	HAL_GPIO_WritePin(TC2_CS__GPIO_Port, TC2_CS__Pin, GPIO_PIN_RESET); //begin read with CS pin low
	HAL_SPI_Receive(SystemHandles::SPI_Thermocouple1, &dataBuffer[0], 4, CMD_TIMEOUT); //Fill the data buffer with data from TC1
	HAL_GPIO_WritePin(TC2_CS__GPIO_Port, TC2_CS__Pin, GPIO_PIN_SET); //end read with setting CS pin to high again


	TC2_Temp_Data = 0xffff & ((dataBuffer[3] << 6) | (dataBuffer[2] >> 2));

	TC2_Internal_Temp_Data = 0xffff & ((dataBuffer[1] << 4) | (dataBuffer[0] >> 4));

	TC2_faultBits = 0xff & (((2 << 4) - 1 ) & ((dataBuffer[2] << 3) & (2 << 1)| ((dataBuffer[0]) & ((2<<2)-1))));

}

void ThermocoupleTask::ConvertTempuatureData()
{
	double TC1_temperature;
	double TC2_temperature;

	uint16_t dataBits = TC1_Temp_Data & 0x1fff; //filters out only the data bits
	bool signBit = (TC1_Temp_Data & 0x2000) == 0x2000; //true if sign bit is on

	//Calculate the actual temperature value using two's complement if needed
	if(!signBit){
		TC1_temperature = (double)dataBits/4.0;
	} else {
		dataBits = (~dataBits) + 1;
		TC1_temperature = -1 * (double)dataBits/4.0;
	}

	SOAR_PRINT("\t-- The temp read by TC1 is %f --\n", TC1_temperature);



	 dataBits = TC2_Temp_Data & 0x1fff;
	 signBit = (TC2_Temp_Data & 0x2000) == 0x2000;


	if(!signBit){
			TC2_temperature = (double)dataBits/4.0;
		} else {
			dataBits = (~dataBits) + 1;
			TC2_temperature = -1 * (double)dataBits/4.0;
		}

	SOAR_PRINT("\t-- The temp read by TC2 is %f --\n", TC2_temperature);
}




















