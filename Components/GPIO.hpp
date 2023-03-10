/**
 ******************************************************************************
 * File Name          : GPIO.hpp
 * Description        :
 *
 *	GPIO contains all GPIO pins wrapped in a namespace and corresponding functions
 *
 *	All GPIO pins should be controlled through this abstraction layer to ensure readable control.
 *
 ******************************************************************************
*/
#ifndef AVIONICS_INCLUDE_SOAR_CORE_GPIO_H
#define AVIONICS_INCLUDE_SOAR_CORE_GPIO_H
#include "SystemDefines.hpp"
#include "main.h"
#include "stm32f4xx_hal.h"

namespace GPIO
{
//TODO: Deal with these!
	namespace LED1
	{
		inline void On() { }
		inline void Off() { }
		inline void Toggle() { }

		inline bool IsOn() { return false; }
	}

	namespace LED2
	{
		inline void On() { }
		inline void Off() { }
		inline void Toggle() { }

		inline bool IsOn() { return false; }
	}
	
}

#endif /* AVIONICS_INCLUDE_SOAR_CORE_GPIO_H */
