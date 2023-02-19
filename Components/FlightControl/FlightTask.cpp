/**
 ******************************************************************************
 * File Name          : FlightTask.cpp
 * Description        : Primary flight task, default task for the system.
 ******************************************************************************
*/
#include "FlightTask.hpp"
#include "GPIO.hpp"
#include "SystemDefines.hpp"

//void motorPWMTask()
//{
//  /* USER CODE BEGIN motorPWMTask */
//
//  HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_4);
//  htim8.Instance->CCR4 = 25;
//
//  for (;;)
//  {
//	  htim8.Instance->CCR4 = 70; //55
//	  osDelay(5000);
//	  htim8.Instance->CCR4 = 25;
//	  osDelay(5000);
//
//  }
//  /* USER CODE END motorPWMTask */
//}


/**
 * @brief Constructor for FlightTask
 */
FlightTask::FlightTask() : Task(FLIGHT_TASK_QUEUE_DEPTH_OBJS)
{
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
void FlightTask::Run(void * pvParams)
{
//    uint32_t tempSecondCounter = 0; // TODO: Temporary counter, would normally be in HeartBeat task or HID Task, unless FlightTask is the HeartBeat task
    GPIO::LED1::Off();
//    motorPWMTask();
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_4);
    htim8.Instance->CCR4 = 50;
    while (1) {
      htim8.Instance->CCR4 = 250; //55
	  osDelay(5000);
	  htim8.Instance->CCR4 = 50;
	  osDelay(5000);
    }
}
