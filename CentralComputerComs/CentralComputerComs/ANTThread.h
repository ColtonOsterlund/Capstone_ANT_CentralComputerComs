#pragma once

#include "MessageQueue.h"

class ANTThread
{
public:
	ANTThread(MessageQueue* RxQueue, MessageQueue* TxQueue);

	// Allow the class to be callable as a thread
	void operator()();

private:
	 
};

