#pragma once

#include "ANTMessage.h"
#include "SendReceiveQueue.h"
#include "CentralComputerThread.h"

class ANTThread: public CentralComputerThread
{
public:
	ANTThread(SendReceiveQueue<ANTMessage>* queues);

	// Allow the class to be callable as a thread
	void operator()();

private:
	 
};

