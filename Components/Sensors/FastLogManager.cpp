/**
 ******************************************************************************
 * File Name          : FastLogManager.cpp
 * Description        : FastLogManager manages the fast log state machine
 ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "FastLogManager.hpp"
#include "PBBProtocolTask.hpp"
#include "main_avionics.hpp"
#include "Data.h"
#include "DebugTask.hpp"
#include "Task.hpp"
#include "main.h"

/* Constructor ------------------------------------------------------------------*/
FastLogManager::FastLogManager()
{
    state = IDLE;
    msTick = 0;
    msStartTick = 0;
    sendIdx = 0;
}

/* External Functions ------------------------------------------------------------------*/
void FastLogManager::TransitionPend()
{
    if(state == IDLE) {
        state = PEND;
    }
}

void FastLogManager::TransitionStart()
{
    if(state == PEND) {
        msStartTick = 0;
        state = START;
    }
}

void FastLogManager::TransitionReset()
{
    pressureLogBuffer.clear();
    state = IDLE;
}

/* Internal Functions ------------------------------------------------------------------*/  
void FastLogManager::TransitionSend()
{
    sendIdx = 0;
    state = SEND;
}

void FastLogManager::TransmitLogData(PressureLog& pl, uint32_t timestamp) {
    Proto::TelemetryMessage msg;
	msg.set_source(Proto::Node::NODE_PBB);
	msg.set_target(Proto::Node::NODE_DMB);
	Proto::PressureLog plg;
	plg.set_time(timestamp);
	plg.set_ib_pressure(pl.ibPressure);
    plg.set_pv_pressure(pl.pvPressure);
    msg.set_pressureLog(plg);
	EmbeddedProto::WriteBufferFixedSize<DEFAULT_PROTOCOL_WRITE_BUFFER_SIZE> writeBuffer;
	msg.serialize(writeBuffer);

    // Send the data
    PBBProtocolTask::SendProtobufMessage(writeBuffer, Proto::MessageID::MSG_TELEMETRY);
}

/* Functions ------------------------------------------------------------------*/

void FastLogManager::Run() {
    xLastWakeTime = xTaskGetTickCount();
    switch (state) {
        case IDLE:
            SamplePressureTransducer();
            TransmitProtocolPressureData();
            vTaskDelayUntil(&xLastWakeTime, AUTO_PT_SLOW_PERIOD);
            break;
        case PEND:
            SamplePressureTransducer();

            // If the tick is greater than the AUTO_PT_SLOW_PERIOD, transmit the data
            if (++msTick > AUTO_PT_SLOW_PERIOD) {
                TransmitProtocolPressureData();
                msTick = 0;
            }
            
            // Add to buffer
            pressureLogBuffer.push(data);

            // Delay
            vTaskDelayUntil(&xLastWakeTime, 1);
            break;
        case START:
            SamplePressureTransducer();

            // If the tick is greater than the AUTO_PT_SLOW_PERIOD, transmit the data
            if (++msTick > AUTO_PT_SLOW_PERIOD) {
                TransmitProtocolPressureData();
                msTick = 0;
            }

            // Add to buffer
            pressureLogBuffer.push(data);

            // If the start tick is greater than the log buffer size - 1000, stop filling the buffer and go to send
            if (++msStartTick > (LOG_BUFFER_SIZE - 1000)) {
                state = SEND;
            }

            // Delay
            vTaskDelayUntil(&xLastWakeTime, 1);
            break;
        case SEND:
            // If the tick is past the slow period
            if ((++msTick * AUTO_PT_SEND_PERIOD) > AUTO_PT_SLOW_PERIOD) {
                // Sample and send
                SamplePressureTransducer();
                TransmitProtocolPressureData();
                msTick = 0;
            }

            // Manage sending the log data
            if(sendIdx < LOG_BUFFER_SIZE) {
                PressureLog data = pressureLogBuffer[sendIdx];
                TransmitLogData(data, sendIdx);
                sendIdx += 1;
            } else {
                sendIdx = 0;
            }
            
            // Delay
            vTaskDelayUntil(&xLastWakeTime, AUTO_PT_SEND_PERIOD);
            break;
    }
}


/* Pressure Transducer Readers ------------------------------------------------------------------*/

void ADC_Select_CH3 (void)
{
	ADC_ChannelConfTypeDef sConfig = {0};
	  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	  */
	  sConfig.Channel = ADC_CHANNEL_3;
	  sConfig.Rank = 1;
	  sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES;
	  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	  {
	    Error_Handler();
	  }
}

void ADC_Select_CH15 (void)
{
	ADC_ChannelConfTypeDef sConfig = {0};
	  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	  */
	  sConfig.Channel = ADC_CHANNEL_15;
	  sConfig.Rank = 1;
	  sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES;
	  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	  {
	    Error_Handler();
	  }
}


/**
 * @brief This function reads and updates pressure readings
 *          from the pressure transducer.
 */
void FastLogManager::SamplePressureTransducer()
{
	static const int PT_VOLTAGE_ADC_POLL_TIMEOUT = 50;
	static const double PRESSURE_SCALE = 1.5220883534136546; // Value to scale to original voltage value
	double adcVal[2] = {};
	double pressureTransducerValue1 = 0;
	double pressureTransducerValue2 = 0;
	double vi = 0;

	/* Functions -----------------------------------------------------------------*/
	ADC_Select_CH3();
	HAL_ADC_Start(&hadc1);  // Enables ADC and starts conversion of regular channels
	if(HAL_ADC_PollForConversion(&hadc1, PT_VOLTAGE_ADC_POLL_TIMEOUT) == HAL_OK) { //Check if conversion is completed
		adcVal[0] = HAL_ADC_GetValue(&hadc1); // Get ADC Value
		HAL_ADC_Stop(&hadc1);
		}
	ADC_Select_CH15();
		HAL_ADC_Start(&hadc1);  // Enables ADC and starts conversion of regular channels
		if(HAL_ADC_PollForConversion(&hadc1, PT_VOLTAGE_ADC_POLL_TIMEOUT) == HAL_OK) { //Check if conversion is completed
			adcVal[1] = HAL_ADC_GetValue(&hadc1); // Get ADC Value
			HAL_ADC_Stop(&hadc1);
			}
	vi = ((3.3/4095) * (adcVal[0])); // Converts 12 bit ADC value into voltage
	pressureTransducerValue1 = (250 * (vi * PRESSURE_SCALE) - 125) * 1000; // Multiply by 1000 to keep decimal places
	data.ibPressure = (int32_t) pressureTransducerValue1; // Pressure in PSI
	vi = ((3.3/4095) * (adcVal[1])); // Converts 12 bit ADC value into voltage
		pressureTransducerValue2 = (250 * (vi * PRESSURE_SCALE) - 125) * 1000; // Multiply by 1000 to keep decimal places
		data.pvPressure = (int32_t) pressureTransducerValue2; // Pressure in PSI
}

/**
 * @brief Transmits a protocol barometer data sample
 */
void FastLogManager::TransmitProtocolPressureData()
{
    Proto::TelemetryMessage msg;
	msg.set_source(Proto::Node::NODE_PBB);
	msg.set_target(Proto::Node::NODE_DMB);
	Proto::PbbPressure pressData;
	pressData.set_ib_pressure(data.ibPressure);
	pressData.set_lower_pv_pressure(data.pvPressure);
	msg.set_pbbPressure(pressData);

	EmbeddedProto::WriteBufferFixedSize<DEFAULT_PROTOCOL_WRITE_BUFFER_SIZE> writeBuffer;
	msg.serialize(writeBuffer);

    // Send the barometer data
    PBBProtocolTask::SendProtobufMessage(writeBuffer, Proto::MessageID::MSG_TELEMETRY);
}
