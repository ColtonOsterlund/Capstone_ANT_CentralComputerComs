#include "ANTServer.h"

#include "BackendANTDriver.h"
#include "ant_messages.h"

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

void ANTServer::send_ANT_message(ANTMessage& msg)
{
	unsigned char* msg_arr = new unsigned char[msg.get_length() + ANT_MSG_HEADER_LENGTH];
	msg_arr[ANT_MSG_CONVEYOR_ID_INDEX] = msg.get_conveyor_id();
	msg_arr[ANT_MSG_ID_INDEX] = msg.get_id();

	for (int i = 0; i < msg.get_length(); i++) {
		msg_arr[ANT_MSG_HEADER_LENGTH + i] = msg.get_data()[i];
	}

	Send_message_to_ANT(msg_arr, msg.get_length() + ANT_MSG_HEADER_LENGTH);
	delete[] msg_arr;
}

void ANTServer::continue_flow_control()
{
	queue_handler->set_flow_continue();
}
