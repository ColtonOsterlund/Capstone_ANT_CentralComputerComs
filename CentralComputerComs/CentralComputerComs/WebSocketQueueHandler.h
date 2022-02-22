#pragma once

#include "CentralComputerThread.h"
#include "SendReceiveQueue.h"
#include "WebSocketMessage.h"
#include "WebSocketServer.h"

#define WEBSOCKET_QUEUE_THREAD_SLEEP_MS 100

class WebSocketServer; /* Forward declare to avoid association issue */

class WebSocketQueueHandler : public CentralComputerThread
{
public:

	WebSocketQueueHandler(SendReceiveQueue<WebSocketMessage>* queues);

	void operator()() override;

	void push_message(std::string message);

	void set_websocket_server(WebSocketServer* server);

private:
	SendReceiveQueue<WebSocketMessage>* message_queues;
	WebSocketServer* ws_server;
};

