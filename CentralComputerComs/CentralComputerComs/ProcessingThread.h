#pragma once

#include "CentralComputerThread.h"
#include "ConveyorSystem.h"
#include "SendReceiveQueue.h"
#include "WebThreadMessageHandler.h"

#include "WebSocketMessage.h"
#include "BackendANTMessage.h"


#define PROCESSING_THREAD_SLEEP_MS 500

class ProcessingThread :
    public CentralComputerThread
{
public:
    ProcessingThread(SendReceiveQueue<WebSocketMessage>* web_queues, SendReceiveQueue<ANTMessage>* ant_queues);

    void operator()() override;

private:
    WebThreadMessageHandler web_handler;
    ANTThreadMessageHandler ant_handler;
    ConveyorSystem conveyor_system;
};

