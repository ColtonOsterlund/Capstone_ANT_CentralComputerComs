#pragma once

#include "MessageQueue.h"

class WebSocketThread
{
public:
	WebSocketThread(MessageQueue* RxQueue, MessageQueue* TxQueue);
	
	// Allow the class to be callable as a thread
	void operator()();

private:
};

