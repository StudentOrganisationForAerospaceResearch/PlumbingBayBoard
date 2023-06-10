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
#include "PBBProtocolTask.hpp"

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
        SampleThermocouple();
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

	uint8_t dataBuffer1[4] = {0};
	uint8_t dataBuffer5[5] = {0};
	//See Above bit mem-map


	uint8_t Error1=0;// Thermocouple Connection acknowledge Flag
	uint32_t sign1=0;
	int Temp1=0;

	//Read ---------------------------------------------------------------------------------------
	HAL_GPIO_WritePin(TC1_CS__GPIO_Port, TC1_CS__Pin, GPIO_PIN_SET);


    //Read From Thermocouple 1 first
	HAL_GPIO_WritePin(TC1_CS__GPIO_Port, TC1_CS__Pin, GPIO_PIN_RESET); //begin read with CS pin low
	HAL_Delay(10);
	HAL_SPI_Receive(SystemHandles::SPI_Thermocouple1, dataBuffer5, 5, 1000); //Fill the data buffer with data from TC1
	HAL_Delay(10);
	HAL_GPIO_WritePin(TC1_CS__GPIO_Port, TC1_CS__Pin, GPIO_PIN_SET); //end read with setting CS pin to high again

	SOAR_PRINT("------------1-------------\n");

	for(int i = 0; i<4; i++){
		dataBuffer1[i] = dataBuffer5[i+1];
		SOAR_PRINT("databufferTC1[%d] are: %d\n",i, dataBuffer1[i]);
	}
	SOAR_PRINT("\n");


	double temp_debug_1 = 0;

	Error1=dataBuffer1[3]&0x07;								  // Error Detection
	sign1=(dataBuffer1[0]&(0x80))>>7;							  // Sign Bit calculation

	if(dataBuffer1[3] & 0x07){								  // Returns Error Number
		SOAR_PRINT("THERE IS AN ERROR !!!!!!!");
		temp_debug_1 = (-1*(dataBuffer1[3] & 0x07));
	}
	else if(sign1==1){									  // Negative Temperature
		Temp1 = (dataBuffer1[0] << 6) | (dataBuffer1[1] >> 2);
		Temp1&=0b01111111111111;
		Temp1^=0b01111111111111;
		temp_debug_1 = (double)-Temp1/4;
	}
	else												  // Positive Temperature
	{
		Temp1 = (dataBuffer1[0] << 6) | (dataBuffer1[1] >> 2);
		temp_debug_1 = ((double)Temp1 / 4);
	}

	temp_debug_1 = temp_debug_1*100-320; //room temp should've been 22.5, it was reading ambient 25.7
	SOAR_PRINT(
				"\t-- The new Temp as big number say its read by TC1 is %d \n"
				, (int)temp_debug_1);

	SOAR_PRINT("\t-- The new Temp say its read by TC1 is %d.%d C \n"
			"-------------------------\n", (int)temp_debug_1/100, (uint8_t)(int)temp_debug_1%100);




	uint8_t dataBuffer2[4] = {0};
	dataBuffer5[5] = {0};
	//See Above bit mem-map

	uint8_t Error2=0;// Thermocouple Connection acknowledge Flag
	uint32_t sign2=0;
	int Temp2=0;

	//Read ---------------------------------------------------------------------------------------
	HAL_GPIO_WritePin(TC2_CS__GPIO_Port, TC2_CS__Pin, GPIO_PIN_SET);

	//Read From Thermocouple 1 first
	HAL_GPIO_WritePin(TC2_CS__GPIO_Port, TC2_CS__Pin, GPIO_PIN_RESET); //begin read with CS pin low
	HAL_Delay(10);
	HAL_SPI_Receive(SystemHandles::SPI_Thermocouple2, dataBuffer5, 5, 1000); //Fill the data buffer with data from TC1
	HAL_Delay(10);
	HAL_GPIO_WritePin(TC2_CS__GPIO_Port, TC2_CS__Pin, GPIO_PIN_SET); //end read with setting CS pin to high again

	SOAR_PRINT("------------2-------------\n");

	for(int i = 0; i<4; i++){
		dataBuffer2[i] = dataBuffer5[i+1];
		SOAR_PRINT("databufferTC2[%d] are: %d\n",i, dataBuffer2[i]);
	}
	SOAR_PRINT("\n");


	double temp_debug_2 = 0;

	Error2=dataBuffer2[3]&0x07;								  // Error Detection
	sign2=(dataBuffer2[0]&(0x80))>>7;							  // Sign Bit calculation

	if(dataBuffer2[3] & 0x07){								  // Returns Error Number
		SOAR_PRINT("THERE IS AN ERROR !!!!!!!");
		temp_debug_2 = (-1*(dataBuffer2[3] & 0x07));
	}
	else if(sign2==1){									  // Negative Temperature
		Temp2 = (dataBuffer2[0] << 6) | (dataBuffer2[1] >> 2);
		Temp2&=0b01111111111111;
		Temp2^=0b01111111111111;
		temp_debug_2 = (double)-Temp2/4;
	}
	else												  // Positive Temperature
	{
		Temp2 = (dataBuffer2[0] << 6) | (dataBuffer2[1] >> 2);
		temp_debug_2 = ((double)Temp2 / 4);
	}

	temp_debug_2 = temp_debug_2*100-320; //room temp should've been 22.5, it was reading ambient 25.7
	SOAR_PRINT(
				"\t-- The new Temp as big number say its read by TC2 is %d \n"
				, (int)temp_debug_2);

	SOAR_PRINT("\t-- The new Temp say its read by TC2 is %d.%d C \n"
			"-------------------------\n", (int)temp_debug_2/100, (uint8_t)(int)temp_debug_2%100);

}


/**
 * @brief Transmits a protocol barometer data sample
 */
void ThermocoupleTask::TransmitThermocouple()
{
    SOAR_PRINT("Thermocouple State Transmit...\n");

    Proto::TelemetryMessage msg;
	msg.set_source(Proto::Node::NODE_PBB);
	msg.set_target(Proto::Node::NODE_DMB);
//	msg.set_message_id((uint32_t)Proto::MessageID::MSG_TELEMETRY);
	Proto::PBBTemp tempData;
	tempData.set_ib_temperature(TC1_Temp_Data);
	tempData.set_pv_temperature(TC1_Internal_Temp_Data);
	msg.set_temppbb(tempData);

	EmbeddedProto::WriteBufferFixedSize<DEFAULT_PROTOCOL_WRITE_BUFFER_SIZE> writeBuffer;
	msg.serialize(writeBuffer);

    // Send the barometer data
    PBBProtocolTask::SendProtobufMessage(writeBuffer, Proto::MessageID::MSG_TELEMETRY);
}
















