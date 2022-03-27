#include "ANTServer.h"

#include "BackendANTDriver.h"

constexpr auto ANT_BACKEND_SERVER_DEVICE_NUMBER = 0;

ANTServer::ANTServer(): queue_handler(nullptr)
{
	Set_backend_ANT_server(this);
}

void ANTServer::operator()()
{
	Run_driver(ANT_BACKEND_SERVER_DEVICE_NUMBER);
}

void ANTServer::set_queue_handler(ANTQueueHandler* handler)
{
	this->queue_handler = handler;
}
