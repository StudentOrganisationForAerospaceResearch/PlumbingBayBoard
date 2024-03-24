/**
 ******************************************************************************
 * File Name          : MEV.hpp
 * Description        : Servo State and Control Actions
 ******************************************************************************
*/

#ifndef SOAR_MEV_HPP_
#define SOAR_MEV_HPP_

#include "SystemDefines.hpp"
#include "PBBProtocolTask.hpp"

class MEV {
    public:
        static void OpenMEV();
        static void CloseMEV();

        static bool GetServoState();
        inline static void TransmitProtocolServoState();

    private:
        static bool servoState;
};

void MEV::TransmitProtocolServoState()
{
    SOAR_PRINT("Servo State Transmit...\n");

    Proto::TelemetryMessage msg;
    msg.set_source(Proto::Node::NODE_PBB);
    msg.set_target(Proto::Node::NODE_DMB);
    Proto::CombustionControlStatus mevStateInfo;
    mevStateInfo.set_mev_open(MEV::GetServoState());
	msg.set_combustionControlStatus(mevStateInfo);

    EmbeddedProto::WriteBufferFixedSize<DEFAULT_PROTOCOL_WRITE_BUFFER_SIZE> writeBuffer;
    msg.serialize(writeBuffer);

    // Send the barometer data
    PBBProtocolTask::SendProtobufMessage(writeBuffer, Proto::MessageID::MSG_TELEMETRY);
}

#endif
