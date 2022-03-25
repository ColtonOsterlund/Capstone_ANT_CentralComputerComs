#pragma once

#include "ANTQueueHandler.h"

class ANTQueueHandler;

class ANTServer: public CentralComputerThread
{
public:
	ANTServer();

	void operator()() override;

	void set_queue_handler(ANTQueueHandler* handler);

private:
	ANTQueueHandler* queue_handler;
};

