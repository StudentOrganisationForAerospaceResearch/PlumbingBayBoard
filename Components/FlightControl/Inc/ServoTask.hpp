/**
 ******************************************************************************
 * File Name          : SOAR_SENSOR_SERVO_TASK_HPP_.hpp
 * Description        :
 ******************************************************************************
*/
#ifndef SOAR_SENSOR_SERVO_TASK_HPP_
#define SOAR_SENSOR_SERVO_TASK_HPP_
/* Includes ------------------------------------------------------------------*/
#include "Task.hpp"
#include "SystemDefines.hpp"


/* Macros/Enums ------------------------------------------------------------*/
enum PT_TASK_COMMANDS {
    SERVO_NONE = 0,
    SERVO_OPEN,// Get a new pressure transducer sample, task will be blocked for polling time
    SERVO_CLOSE,    // Send the current pressure transducer data over the Radio
};

/* Class ------------------------------------------------------------------*/
class ServoTask : public Task
{
public:
    static ServoTask& Inst() {
        static ServoTask inst;
        return inst;
    }

    void InitTask();

protected:
    static void RunTask(void* pvParams) { ServoTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();

    void Run(void* pvParams);    // Main run code

    void HandleCommand(Command& cm);
    void HandleRequestCommand(uint16_t taskCommand);

    // Sampling
    void Open();
    void Close();


private:
    ServoTask();                                        // Private constructor
    ServoTask(const ServoTask&);                    // Prevent copy-construction
    ServoTask& operator=(const ServoTask&);            // Prevent assignment
};

#endif
