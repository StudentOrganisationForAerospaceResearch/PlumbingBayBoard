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
#include "Data.h"
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
    uint16_t ReadCalibrationCoefficients(uint8_t PROM_READ_CMD);
    //ETHAN NOTE: UNSURE IF I NEEDEDTHIS READCALIBRATION METHOD

    // Data
    double thermcoup1temp;
    double thermcoup2temp;


private:
    ThermocoupleTask();                                        // Private constructor
    ThermocoupleTask(const ThermocoupleTask&);                    // Prevent copy-construction
    ThermocoupleTask& operator=(const ThermocoupleTask&);            // Prevent assignment
};

#endif		//SOAR_SENSOR_THERMOCOUPLE_TASK_HPP_
