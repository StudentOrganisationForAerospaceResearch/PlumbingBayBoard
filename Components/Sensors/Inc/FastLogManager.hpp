/**
 ******************************************************************************
 * File Name          : FastLogManager.hpp
 * Description        : FastLogManager manages the fast log state machine
 ******************************************************************************
*/
#ifndef SOAR_FAST_LOG_MANAGER_HPP_
#define SOAR_FAST_LOG_MANAGER_HPP_

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <etl/circular_buffer.h>
#include <FreeRTOS.h>
#include "SystemDefines.hpp"


/* Macros/Enums ------------------------------------------------------------*/
constexpr uint16_t PT_SLOW_RATE = 4; // 4 Hz
constexpr uint16_t LOG_BUFFER_SIZE = 10000; // 10000 samples (10s at 1kHz)
constexpr uint16_t SEND_RATE = 125; // 125 Hz

constexpr uint16_t AUTO_PT_SLOW_PERIOD = 1000 / PT_SLOW_RATE;
constexpr uint16_t AUTO_PT_SEND_PERIOD = 1000 / SEND_RATE;
/* Structs ------------------------------------------------------------------*/
struct PressureLog {
    int32_t pvPressure;
    int32_t ibPressure;
};

/* Class ------------------------------------------------------------------*/
class FastLogManager
{
public:
    static FastLogManager& Inst() {
        static FastLogManager inst;
        return inst;
    }

    void Run();

    // Externally managed commands
    void TransitionPend();
    void TransitionStart();
    void TransitionReset();

    // Get the data
    PressureLog GetLastLog() { return data; }
    void PrintLastLog() {
        SOAR_PRINT("|PT| PV Pressure [1] (PSI): %d.%d\r\n", data.pvPressure / 1000, data.pvPressure % 1000);
        SOAR_PRINT("|PT| IB Pressure [2] (PSI): %d.%d\r\n", data.ibPressure / 1000, data.ibPressure % 1000);
    }

private:
    FastLogManager();

    enum State {
        IDLE, // Regular Logging Rate (5 Hz)
        PEND, // 1000 Hz logging into the circular buffer
        START, // Take T-1s of data, and 9s of data
        SEND,  // Send the data over the telemetry
    };

    void TransitionSend(); // Transition to the send state

    void TransmitLogData(PressureLog& pl, uint32_t timestamp); // Transmits one log data packet

    void SamplePressureTransducer(); // Samples the pressure transducer
    void TransmitProtocolPressureData(); // Transmits the pressure transducer data

protected:
    State state;

    etl::circular_buffer<PressureLog, LOG_BUFFER_SIZE> pressureLogBuffer;

    uint32_t msTick;
    uint32_t msStartTick;

    TickType_t xLastWakeTime;

    uint32_t sendIdx;
    PressureLog data;
};


#endif // SOAR_SENSOR_THERMOCOUPLE_TASK_HPP_
