#pragma once

#include <thread>
#include <vector>

#include "CentralComputerThread.h"
#include "MessageQueue.h"

#include "ANTMessage.h"
#include "WebSocketMessage.h"
#include "SendReceiveQueue.h"

#define NUM_THREADS 2

enum {
	TERMINATE
};

class MainThread: public CentralComputerThread
{
public:
	MainThread();
	~MainThread();

	void operator()() override;

	void request_termination() override;

	void get_user_command();


private:
	MessageQueue<ANTMessage> ANTRxQueue;
	MessageQueue<ANTMessage> ANTTxQueue;

	MessageQueue<WebSocketMessage> WebRxQueue;
	MessageQueue<WebSocketMessage> WebTxQueue;

	SendReceiveQueue<ANTMessage> ant_thread_queues;
	SendReceiveQueue<ANTMessage> ant_processing_queues;
	SendReceiveQueue<WebSocketMessage> websocket_thread_queues;
	SendReceiveQueue<WebSocketMessage> websocket_processing_queues;

	CentralComputerThread* class_threads[NUM_THREADS];
	std::thread actual_threads[NUM_THREADS];
};

