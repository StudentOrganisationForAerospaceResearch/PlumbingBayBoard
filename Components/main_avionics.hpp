/**
 ******************************************************************************
 * File Name          : Main.hpp
 * Description        : Header file for Main.cpp, acts as an interface between
 *  STM32CubeIDE and our application.
 ******************************************************************************
*/
#ifndef AVIONICS_INCLUDE_SOAR_MAIN_H
#define AVIONICS_INCLUDE_SOAR_MAIN_H
#include "Mutex.hpp"
#include "stm32f4xx_hal.h"

/* Interface Functions ------------------------------------------------------------------*/
/* These functions act as our program's 'main' and any functions inside CubeIDE's main --*/
void run_main();
void run_StartDefaultTask();

/* Global Functions ------------------------------------------------------------------*/
void print(const char* format, ...);
void soar_assert_debug(bool condition, const char* file, uint16_t line, const char* str = nullptr, ...);

/* Global Variable Interfaces ------------------------------------------------------------------*/
/* All must be extern from main_avionics.cpp -------------------------------------------------*/
namespace Global
{
    extern Mutex vaListMutex;
}


/* System Handles ------------------------------------------------------------------*/
/* This should be the only place externs are allowed -------------------------------*/
//UART Handles
extern UART_HandleTypeDef huart1;   // UART1 - Conduit/Protobuf
extern UART_HandleTypeDef huart5;   // UART5 - Debug

//ADC Handles
extern ADC_HandleTypeDef hadc1;      // ADC1 - Pressure Transducer ADC

//I2C Handles
//extern I2C_HandleTypeDef hi2c1;      // I2C1 -- EEPROM (? - Do we still have an I2C EEPROM)

//SPI Handles
extern SPI_HandleTypeDef hspi1;      // SPI1 - IMU
extern SPI_HandleTypeDef hspi3;      // SPI3 - Barometer MOSI/MISO/CLK

//CRC Handles
extern CRC_HandleTypeDef hcrc;       // CRC - Hardware CRC System Handle

//DMA Handles
extern DMA_HandleTypeDef hdma_uart4_rx; // DMA UART 4 RX -
extern DMA_HandleTypeDef hdma_uart5_rx; // DMA UART 5 RX -
extern DMA_HandleTypeDef hdma_uart5_tx; // DMA UART 5 TX -

//Timer Handles
extern TIM_HandleTypeDef htim8; // TIM 8


namespace SystemHandles {
    // Aliases
    constexpr UART_HandleTypeDef* UART_Protocol = &huart1;
    constexpr UART_HandleTypeDef* UART_Debug = &huart5;

	constexpr ADC_HandleTypeDef* ADC_Pressure_Transducer = &hadc1;

    constexpr SPI_HandleTypeDef* SPI_IMU = &hspi1;
    constexpr SPI_HandleTypeDef* SPI_Barometer = &hspi3;

    constexpr CRC_HandleTypeDef* CRC_Handle = &hcrc;

    // DMA Aliases

}

#endif /* AVIONICS_INCLUDE_SOAR_MAIN_H */
