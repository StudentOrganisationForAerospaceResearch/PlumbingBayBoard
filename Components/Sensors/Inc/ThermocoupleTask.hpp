/**
 ******************************************************************************
 * File Name          : Thermocouple.hpp
 * Description        :
 ******************************************************************************
*/
#ifndef SOAR_SENSOR_THERMOCOUPLE_TASK_HPP_
#define SOAR_SENSOR_THERMOCOUPLE_TASK_HPP_

/* Includes ------------------------------------------------------------------*/

#include "Task.hpp"
#include "SystemDefines.hpp"


/* Macros/Enums ------------------------------------------------------------*/
enum THERMOCOUPLE_TASK_COMMANDS {
	THERMOCOUPLE = 0,
	THERMOCOUPLE_REQUEST_NEW_SAMPLE,	// Get a new temperature sample
	THERMOCOUPLE_REQUEST_TRANSMIT,
	THERMOCOUPLE_REQUEST_DEBUG       	// Send the current temperature data over the Debug UART
};


//class

class ThermocoupleTask : public Task
{
public:
    static ThermocoupleTask& Inst() {
        static ThermocoupleTask inst;
        return inst;
    }

    void InitTask();

protected:
    static void RunTask(void* pvParams) { ThermocoupleTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();
    //ETHAN NOTE: NOT SURE WHAT THE pvParams ARE FOR

    void Run(void* pvParams);    // Main run code

    void HandleCommand(Command& cm);
    void HandleRequestCommand(uint16_t taskCommand);

    // Sampling
    void SampleThermocouple();
    void ConvertTempuatureData();
    uint16_t ReadCalibrationCoefficients(uint8_t PROM_READ_CMD);
    void TransmitThermocouple();
    //ETHAN NOTE: UNSURE IF I NEEDEDTHIS READCALIBRATION METHOD


    // Data Fields ----------------------------------------
    uint16_t TC1_Temp_Data;
	//bits 15 and 14 are not used
	//bit 13 is the sign bit (1 = negative)
	//bits 12 to 0 store the temperature amount (2^10) to (2^-2)

	uint16_t TC1_Internal_Temp_Data;
	//bits 15 to 12 are not used
	//bit 11 is the sign bit
	//bits 10 to 0 store the internal temperature amount (2^6) to (2^-4)

	uint8_t TC1_faultBits;
	//bits 7 to 4 are unused
	//bit 3 (D16) is the Fault declaring bit (if high there is a fault)
	//bit 2 (D2) is SCV Fault
	//bit 1 (D1) is SCG Fault
	//bit 0 (D0) is No Connection Fault
	//for more info about D pins see bit mem-map
	//in ThermocoupleTask::SampleThermocouple() in ThermocoupleTask.cpp

	uint16_t TC2_Temp_Data;
	uint16_t TC2_Internal_Temp_Data;
	uint8_t TC2_faultBits;


private:
    ThermocoupleTask();                                        // Private constructor
    ThermocoupleTask(const ThermocoupleTask&);                    // Prevent copy-construction
    ThermocoupleTask& operator=(const ThermocoupleTask&);            // Prevent assignment
};

#endif		//SOAR_SENSOR_THERMOCOUPLE_TASK_HPP_
