/**
 ******************************************************************************
 * File Name          : PBBProtocolTask.hpp
 * Description        : Protocol task, specific to PBB
 ******************************************************************************
*/
#ifndef SOAR_PBBPROTOCOL_HPP_
#define SOAR_PBBPROTOCOL_HPP_
#include "ProtocolTask.hpp"
#include "Task.hpp"
#include "SystemDefines.hpp"
#include "UARTTask.hpp"

/* Enums ------------------------------------------------------------------*/

/* Class ------------------------------------------------------------------*/
class PBBProtocolTask : public ProtocolTask
{
public:
    static PBBProtocolTask& Inst() {
        static PBBProtocolTask inst;
        return inst;
    }

    static void SendProtobufMessage(EmbeddedProto::WriteBufferFixedSize<DEFAULT_PROTOCOL_WRITE_BUFFER_SIZE>& writeBuffer, Proto::MessageID msgId)
    {
		Inst().ProtocolTask::SendProtobufMessage(writeBuffer, msgId);
    }

    void InitTask();

protected:
    static void RunTask(void* pvParams) { PBBProtocolTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();

    // These handlers will receive a buffer and size corresponding to a decoded message
    void HandleProtobufCommandMessage(EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES>& readBuffer);
    void HandleProtobufControlMesssage(EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES>& readBuffer);
    void HandleProtobufTelemetryMessage(EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES>& readBuffer);

    // Member variables

private:
    PBBProtocolTask();        // Private constructor
    PBBProtocolTask(const PBBProtocolTask&);                        // Prevent copy-construction
    PBBProtocolTask& operator=(const PBBProtocolTask&);            // Prevent assignment
};

#endif    // SOAR_PBBPROTOCOL_HPP_
