#pragma once

#include "ANTMessage.h"
#include "SendReceiveQueue.h"
#include "CentralComputerThread.h"
#include "ANTServer.h"
#include "ANTQueueHandler.h"

class ANTThread: public CentralComputerThread
{
public:
	ANTThread(SendReceiveQueue<ANTMessage>* queues);

	// Allow the class to be callable as a thread
	void operator()() override;

private:
	ANTQueueHandler queue_handler;
	ANTServer ant_server;
};

