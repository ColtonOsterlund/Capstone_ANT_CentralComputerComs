#pragma once

#include "ANTQueueHandler.h"
#include "BackendANTMessage.h"

class ANTQueueHandler;

class ANTServer: public CentralComputerThread
{
public:
	ANTServer();

	void operator()() override;

	void set_queue_handler(ANTQueueHandler* handler);

	bool is_terminated() { return terminate_requested; }

	void send_ANT_message(ANTMessage& msg);

private:
	ANTQueueHandler* queue_handler;
};

