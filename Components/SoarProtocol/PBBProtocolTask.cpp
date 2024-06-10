/**
 ******************************************************************************
 * File Name          : PBBProtocolTask.hpp
 * Description        : Protocol task, specific to PBB
 ******************************************************************************
*/
#include "PBBProtocolTask.hpp"

#include "FlightTask.hpp"
#include "ReadBufferFixedSize.h"
#include "MEV.hpp"
#include "GPIO.hpp"
#include "FastLogManager.hpp"

/**
 * @brief Initialize the PBBProtocolTask
 */
void PBBProtocolTask::InitTask()
{
    // Make sure the task is not already initialized
    SOAR_ASSERT(rtTaskHandle == nullptr, "Cannot initialize Protocol task twice");

    // Start the task
    BaseType_t rtValue =
        xTaskCreate((TaskFunction_t)PBBProtocolTask::RunTask,
            (const char*)"ProtocolTask",
            (uint16_t)TASK_PROTOCOL_STACK_DEPTH_WORDS,
            (void*)this,
            (UBaseType_t)TASK_PROTOCOL_PRIORITY,
            (TaskHandle_t*)&rtTaskHandle);

    //Ensure creation succeded
    SOAR_ASSERT(rtValue == pdPASS, "ProtocolTask::InitTask - xTaskCreate() failed");
}

/**
 * @brief Default constructor
 */
PBBProtocolTask::PBBProtocolTask() : ProtocolTask(Proto::Node::NODE_PBB, UART::Conduit_DMB, UART_TASK_COMMAND_SEND_RADIO)
{
}

/**
 * @brief Handle a command message
 */
void PBBProtocolTask::HandleProtobufCommandMessage(EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES>& readBuffer)
{
    Proto::CommandMessage msg;
    msg.deserialize(readBuffer);

    // Verify the source and target nodes, if they aren't as expected, do nothing
    if (msg.get_source() != Proto::Node::NODE_DMB || msg.get_target() != Proto::Node::NODE_PBB)
        return;

    // If the message does not have a DMB command, do nothing
    if (!msg.has_pbb_command())
        return;

    SOAR_PRINT("PROTO-INFO: Received PBB Command Message");

    // Process the db command
    switch (msg.get_pbb_command().get_command_enum())
    {
    case Proto::PbbCommand::Command::PBB_OPEN_MEV:
        //MEV::OpenMEV();
        break;
    case Proto::PbbCommand::Command::PBB_CLOSE_MEV:
        //MEV::CloseMEV();
        break;
    default:
        break;
    }
}

/**
 * @brief Handle a control message
 */
void PBBProtocolTask::HandleProtobufControlMesssage(EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES>& readBuffer)
{
    Proto::ControlMessage msg;
    msg.deserialize(readBuffer);

    // Verify the target node
    if (msg.get_target() != Proto::Node::NODE_PBB)
        return;

    // If the message does not have a fast log command, do nothing
    if (!msg.has_fast_log())
        return;

    SOAR_PRINT("PROTO-INFO: Received PBB Fast Log Command");

    // Process the db command
    switch (msg.get_fast_log().get_cmd())
    {
    case Proto::FastLog::FastLogCommand::FL_PEND:
        FastLogManager::Inst().TransitionPend();
        break;
    case Proto::FastLog::FastLogCommand::FL_START:
        FastLogManager::Inst().TransitionStart();
        break;
    case Proto::FastLog::FastLogCommand::FL_RESET:
        FastLogManager::Inst().TransitionReset();
        break;
    case Proto::FastLog::FastLogCommand::FL_SEND:
        // Do nothing
        break;
    }
}

/**
 * @brief Handle a telemetry message
 */
void PBBProtocolTask::HandleProtobufTelemetryMessage(EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES>& readBuffer)
{

}
