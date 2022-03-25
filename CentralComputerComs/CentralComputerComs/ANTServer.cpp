#include "ANTServer.h"

ANTServer::ANTServer(): queue_handler(nullptr)
{
}

void ANTServer::operator()()
{
	//TODO create actual ant server
	while (!terminate_requested) {
		sleep();
	}
}

void ANTServer::set_queue_handler(ANTQueueHandler* handler)
{
	this->queue_handler = handler;
}
