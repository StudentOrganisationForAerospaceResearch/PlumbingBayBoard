/**
 ******************************************************************************
 * File Name          : FlightTask.cpp
 * Description        : Primary flight task, default task for the system.
 ******************************************************************************
*/
#include "FlightTask.hpp"
#include "GPIO.hpp"
#include "SystemDefines.hpp"

/**
 * @brief Constructor for FlightTask
 */
FlightTask::FlightTask() : Task(FLIGHT_TASK_QUEUE_DEPTH_OBJS)
{
    rsm_ = nullptr;
}

/**
 * @brief Initialize the FlightTask
 */
void FlightTask::InitTask()
{
    // Make sure the task is not already initialized
    SOAR_ASSERT(rtTaskHandle == nullptr, "Cannot initialize flight task twice");

    BaseType_t rtValue =
        xTaskCreate((TaskFunction_t)FlightTask::RunTask,
            (const char*)"FlightTask",
            (uint16_t)FLIGHT_TASK_STACK_DEPTH_WORDS,
            (void*)this,
            (UBaseType_t)FLIGHT_TASK_RTOS_PRIORITY,
            (TaskHandle_t*)&rtTaskHandle);

    SOAR_ASSERT(rtValue == pdPASS, "FlightTask::InitTask() - xTaskCreate() failed");
}

/**
 * @brief Instance Run loop for the Flight Task, runs on scheduler start as long as the task is initialized.
 * @param pvParams RTOS Passed void parameters, contains a pointer to the object instance, should not be used
 */
void FlightTask::Run(void* pvParams)
{
    uint32_t tempSecondCounter = 0; // TODO: Temporary counter, would normally be in HeartBeat task or HID Task, unless FlightTask is the HeartBeat task
    GPIO::LED1::Off();

    rsm_ = new RocketSM(RS_ABORT, false);
    Timer radioHeartbeatTimer;

    while (1) {
        

        if (radioHeartbeatTimer.GetState() == COMPLETED)
            SOAR_PRINT("Heartbeat triggered!!!!!!!!!!!!!!!!!!!!!!!!!!!!");

        osDelay(50);



        // There's effectively 3 types of tasks... 'Async' and 'Synchronous-Blocking' and 'Synchronous-Non-Blocking'
        // Asynchronous tasks don't require a fixed-delay and can simply delay using xQueueReceive, it will immedietly run the next task
        // cycle as soon as it gets an event.

        // Synchronous-Non-Blocking tasks require a fixed-delay and will require something like an RTOS timer that tracks the time till the next run cycle,
        // and will delay using xQueueReceive for the set time, but if it gets interrupted by an event will handle the event then restart a xQueueReceive with
        // the time remaining in the timer

        // Synchronous-Blocking tasks are simpler to implement, they do NOT require instant handling of queue events, and will simply delay using osDelay() and
        // poll the event queue once every cycle.

        // This task below with the display would be a 'Synchronous-Non-Blocking' we want to handle queue events instantly, but keep a fixed delay
        // Could consider a universal queue that directs and handles commands to specific tasks, and a task that handles the queue events and then calls the
        // Mappings between X command and P subscribers (tasks that are expecting it).

        // Since FlightTask is so critical to managing the system, it may make sense to make this a Async task that handles commands as they come in, and have these display commands be routed over to the DisplayTask
        // or maybe HID (Human Interface Device) task that handles both updating buzzer frequencies and LED states.
        GPIO::LED1::On();
        osDelay(500);
        GPIO::LED1::Off();
        osDelay(500);

        //Every cycle, print something out (for testing)
        SOAR_PRINT("FlightTask::Run() - [%d] Seconds\n", tempSecondCounter++);

        //Process any commands, in non-blocking mode (TODO: Change to instant-processing once complete HID/DisplayTask)
        Command cm;
        bool res = qEvtQueue->Receive(cm);
        if (res)
            rsm_->HandleCommand(cm);
    }
}
