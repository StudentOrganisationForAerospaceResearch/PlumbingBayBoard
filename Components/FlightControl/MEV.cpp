/**
 ******************************************************************************
 * File Name          : MEV.hpp
 * Description        : Servo State and Control Actions
 ******************************************************************************
*/

#include "main.h"
#include "MEV.hpp"
#include "FlightTask.hpp"
#include "PBBProtocolTask.hpp"

bool MEV::servoState = false;

void MEV::OpenMEV() {
    SOAR_PRINT("closed");
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_4);
    htim8.Instance->CCR4 = 109;
    servoState = true;
}

void MEV::CloseMEV() {
    SOAR_PRINT("opened");
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_4);
    htim8.Instance->CCR4 = 50;
    servoState = false;
}

bool MEV::GetServoState() {
    return servoState;
}

/**
 * @brief Transmits a protocol barometer data sample
 */
void MEV::TransmitProtocolServoState()
{
    SOAR_PRINT("Servo State Transmit...\n");

    Proto::TelemetryMessage msg;
    msg.set_source(Proto::Node::NODE_PBB);
    msg.set_target(Proto::Node::NODE_DMB);
    Proto::MEVState mevStateInfo;
    mevStateInfo.set_mev_open(GetServoState());
	msg.set_mevstate(mevStateInfo);

    EmbeddedProto::WriteBufferFixedSize<DEFAULT_PROTOCOL_WRITE_BUFFER_SIZE> writeBuffer;
    msg.serialize(writeBuffer);

    // Send the barometer data
    PBBProtocolTask::SendProtobufMessage(writeBuffer, Proto::MessageID::MSG_TELEMETRY);
}

