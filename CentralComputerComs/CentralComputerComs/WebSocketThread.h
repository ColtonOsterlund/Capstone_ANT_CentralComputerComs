#pragma once

#include <thread>

#include "CentralComputerThread.h"
#include "WebSocketServer.h"
#include "WebSocketQueueHandler.h"
#include "WebSocketMessage.h"
#include "SendReceiveQueue.h"

class WebSocketThread: public CentralComputerThread
{
public:
	WebSocketThread(SendReceiveQueue<WebSocketMessage>* queues);
	
	// Allow the class to be callable as a thread
	void operator()() override;

	void request_termination() override;

private:
	WebSocketServer socket_server;
	WebSocketQueueHandler queue_handler;
};

