/**
 ******************************************************************************
 * File Name          : MEV.hpp
 * Description        : Servo State and Control Actions
 ******************************************************************************
*/

#include "main.h"
#include "MEV.hpp"
#include "FlightTask.hpp"

bool MEV::servoState = false;

void MEV::OpenMEV() {
//    SOAR_PRINT("closed");
//    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_4);
//    htim8.Instance->CCR4 = 109;
//    servoState = true;
}

void MEV::CloseMEV() {
//    SOAR_PRINT("opened");
//    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_4);
//    htim8.Instance->CCR4 = 50;
//    servoState = false;
}

bool MEV::GetServoState() {
    return servoState;
}
