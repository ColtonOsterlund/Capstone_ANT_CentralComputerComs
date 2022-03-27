#pragma once

#include "CentralComputerThread.h"
#include "SendReceiveQueue.h"
#include "ANTServer.h"
#include "BackendANTMessage.h"

#define ANT_QUEUE_HANDLER_SLEEP_TIMER 100
#define ANT_QUEUE_HANDLER_FLOW_CONTROL_SLEEP_TIMER 25

class ANTServer;

enum class FlowControl {
	CONTINUE = 0,
	STOP = 1
};

class ANTQueueHandler: public CentralComputerThread
{
public:
	ANTQueueHandler(SendReceiveQueue<ANTMessage>* queues);

	void operator()() override;

	void push_message(int id, unsigned char* payload, int length);

	void set_ant_handler(ANTServer* server);

	void set_flow_stop();

	void set_flow_continue();

private:
	SendReceiveQueue<ANTMessage>* queues;
	ANTServer* ant_server;
	FlowControl flow_control;

};