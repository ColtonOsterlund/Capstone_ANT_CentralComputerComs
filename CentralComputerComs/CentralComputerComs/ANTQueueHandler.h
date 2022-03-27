#pragma once

#include "CentralComputerThread.h"
#include "SendReceiveQueue.h"
#include "ANTServer.h"
#include "BackendANTMessage.h"

#define ANT_QUEUE_HANDLER_SLEEP_TIMER 100

class ANTServer;

class ANTQueueHandler: public CentralComputerThread
{
public:
	ANTQueueHandler(SendReceiveQueue<ANTMessage>* queues);

	void operator()() override;

	void push_message(int id, unsigned char* payload, int length);

	void set_ant_handler(ANTServer* server);

private:
	SendReceiveQueue<ANTMessage>* queues;
	ANTServer* ant_server;

};