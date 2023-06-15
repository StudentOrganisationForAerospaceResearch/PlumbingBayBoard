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

#include "TelemetryMessage.hpp"
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
    case THERMOCOUPLE_REQUEST_NEW_SAMPLE: //Sample TC and store in class fields
    	SampleThermocouple();
        break;
    case THERMOCOUPLE_REQUEST_TRANSMIT: //Sending data to PI
        TransmitProtocolThermoData();
        break;
    case THERMOCOUPLE_REQUEST_DEBUG: //Output TC data
        ThermocoupleDebugPrint();
        break;
    default:
        SOAR_PRINT("UARTTask - Received Unsupported REQUEST_COMMAND {%d}\n", taskCommand);
        break;
    }
}


/**
 * @brief Transmits a protocol barometer data sample
 */
void ThermocoupleTask::TransmitProtocolThermoData()
{
    SOAR_PRINT("Thermocouple Task Transmit...\n");
    //ThermocoupleDebugPrint();

    Proto::TelemetryMessage msg;
    msg.set_source(Proto::Node::NODE_PBB);
    msg.set_target(Proto::Node::NODE_DMB);
    msg.set_message_id((uint32_t)Proto::MessageID::MSG_TELEMETRY);
    Proto::PBBTemp tempData;
	tempData.set_ib_temperature(temperature1);
	tempData.set_pv_temperature(temperature2);
	msg.set_temppbb(tempData);

    EmbeddedProto::WriteBufferFixedSize<DEFAULT_PROTOCOL_WRITE_BUFFER_SIZE> writeBuffer;
    msg.serialize(writeBuffer);

    // Send the thermocouple data
    PBBProtocolTask::SendProtobufMessage(writeBuffer, Proto::MessageID::MSG_TELEMETRY);
}


/**
 * @brief display any error messages and the temperature
 */
void ThermocoupleTask::ThermocoupleDebugPrint()
{
	uint8_t Error1= dataBuffer1[3]&0x07;
	uint8_t Error2= dataBuffer2[3]&0x07;

	//thermo 1 print
	if(dataBuffer1[1] & 0x01)
	{								  // Returns Error Number
		if(Error1 & 0x01){
			SOAR_PRINT("Thermocouple 1 is not connected\n");
		}
		if(Error1 & 0x02){
			SOAR_PRINT("Thermocouple 1 is shorted to GND\n");
		}
		if(Error1 & 0x04){
			SOAR_PRINT("Thermocouple 1 is shorted to VCC\n");
		}
		SOAR_PRINT("\n");
	}
	else
	{
		SOAR_PRINT("Thermocouple 1 is reading %d.%d C \n\n" , temperature1/100, temperature1%100);
	}

	//thermo 2 print
	if(dataBuffer2[1] & 0x01)
	{								  // Returns Error Number
		if(Error2 & 0x01){
			SOAR_PRINT("Thermocouple 2 is not connected\n");
		}
		if(Error2 & 0x02){
			SOAR_PRINT("Thermocouple 2 is shorted to GND\n");
		}
		if(Error2 & 0x04){
			SOAR_PRINT("Thermocouple 2 is shorted to VCC\n");
		}
		SOAR_PRINT("\n");
	}
	else
	{
		SOAR_PRINT("Thermocouple 2 is reading %d.%d C \n\n", temperature2/100, temperature2%100);
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


		SOAR_PRINT("\n-- Sample Thermocouple Data --\n");

		uint8_t tempDataBuffer5[5] = {0};
		//See Above bit mem-map

		//Read ---------------------------------------------------------------------------------------
		HAL_GPIO_WritePin(TC1_CS__GPIO_Port, TC1_CS__Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(TC2_CS__GPIO_Port, TC2_CS__Pin, GPIO_PIN_SET);


	    //Read From Thermocouple 1 first
		HAL_GPIO_WritePin(TC1_CS__GPIO_Port, TC1_CS__Pin, GPIO_PIN_RESET); //begin read with CS pin low
		HAL_Delay(10);
		HAL_SPI_Receive(SystemHandles::SPI_Thermocouple1, tempDataBuffer5, 5, 1000); //Fill the data buffer with data from TC1
		HAL_Delay(10);
		HAL_GPIO_WritePin(TC2_CS__GPIO_Port, TC2_CS__Pin, GPIO_PIN_SET); //end read with setting CS pin to high again

		for(int i = 0; i<5; i++){
			if(i!=4)
			dataBuffer1[i] = tempDataBuffer5[i];\
		}

		int Temp1=0;

		if(!(dataBuffer1[1] & 0x01)){ //if there is not an error with TC1 compute temperature
			if((dataBuffer1[0]&(0x80))>>7==1)  // Negative Temperature
			{
				Temp1 = (dataBuffer1[0] << 6) | (dataBuffer1[1] >> 2);
				Temp1&=0b01111111111111;
				Temp1^=0b01111111111111;
				temperature1 = (((double)-Temp1 / 4)*100-320);
			}
			else  // Positive Temperature
			{
				Temp1 = (dataBuffer1[0] << 6) | (dataBuffer1[1] >> 2);
				temperature1 = (((double)Temp1 / 4)*100-320);
			}
		}
		else
		{
			temperature1 = 999; //there is an error detected with TC1
		}



		tempDataBuffer5[5] = {0}; //reset for TC2 reading
		//See Above bit mem-map

		//Read ---------------------------------------------------------------------------------------
		HAL_GPIO_WritePin(TC1_CS__GPIO_Port, TC1_CS__Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(TC2_CS__GPIO_Port, TC2_CS__Pin, GPIO_PIN_SET);

		//Read From Thermocouple 1 first
		HAL_GPIO_WritePin(TC1_CS__GPIO_Port, TC1_CS__Pin, GPIO_PIN_RESET); //begin read with CS pin low
		HAL_Delay(10);
		HAL_SPI_Receive(SystemHandles::SPI_Thermocouple1, tempDataBuffer5, 5, 1000); //Fill the data buffer with data from TC1
		HAL_Delay(10);
		HAL_GPIO_WritePin(TC2_CS__GPIO_Port, TC2_CS__Pin, GPIO_PIN_SET); //end read with setting CS pin to high again

		for(int i = 0; i<5; i++){
			if(i!=4)
			dataBuffer2[i] = tempDataBuffer5[i];
		}

		int Temp2=0;

		if(!(dataBuffer2[1] & 0x01)){ //if there is not an error with TC1 compute temperature
			if((dataBuffer2[0]&(0x80))>>7==1)  // Negative Temperature
			{
				Temp2 = (dataBuffer2[0] << 6) | (dataBuffer2[1] >> 2);
				Temp2&=0b01111111111111;
				Temp2^=0b01111111111111;
				temperature2 = (((double)-Temp2 / 4)*100-320);
			}
			else  // Positive Temperature
			{
				Temp2 = (dataBuffer2[0] << 6) | (dataBuffer2[1] >> 2);
				temperature2 = (((double)Temp2 / 4)*100-320);
			}
		}
		else
		{
			temperature2 = 999; //there is an error detected with TC2
		}
	}
















