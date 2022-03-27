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

	void continue_flow_control();

	void receive_ANT_message(unsigned char* payload);

private:
	void received_input_conveyor_state(int conveyor_id, bool is_available);

	void send_conveyor_state_request(int conveyor_id);

	void send_message_to_driver(ANTMessage& msg);

	ANTQueueHandler* queue_handler;
	ANTMessage* pending_msg;
	bool checking_input_conveyor_ready;
};

