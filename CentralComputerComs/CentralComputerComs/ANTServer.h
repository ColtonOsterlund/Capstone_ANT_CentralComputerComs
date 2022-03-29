#pragma once

#include "ANTQueueHandler.h"
#include "BackendANTMessage.h"
#include "ANTServerTimer.h"

class ANTQueueHandler;
class ANTServerTimer;

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

	void pending_message_timed_out();

private:
	void received_input_conveyor_state(int conveyor_id, bool is_available);

	void send_conveyor_state_request(int conveyor_id);

	void send_message_to_driver(ANTMessage& msg, unsigned char time);

	void wait_for_message(int msg_id, int conveyor_id);


	ANTQueueHandler* queue_handler;
	ANTMessage* pending_input_conveyor_msg;
	bool checking_input_conveyor_ready;

	ANTMessage* pending_msg;
	bool waiting_for_receive;
	int waiting_conveyor_id;
	int waiting_msg_id;

	ANTServerTimer* timer;
	unsigned char timestamp;
};

