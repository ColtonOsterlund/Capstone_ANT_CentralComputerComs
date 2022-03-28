#include "ANTServer.h"

#include <iostream>

#include "BackendANTDriver.h"
#include "ant_messages.h"
#include "central_computer_types.h"

constexpr auto ANT_BACKEND_SERVER_DEVICE_NUMBER = 0;

ANTServer::ANTServer(): queue_handler(nullptr), pending_input_conveyor_msg(nullptr), checking_input_conveyor_ready(false), waiting_for_receive(false), waiting_conveyor_id(EMPTY_CONVEYOR_ID), waiting_msg_id(-1)
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
	if (msg.get_id() == ADD_PACKAGE_ID) {
		pending_input_conveyor_msg = new ANTMessage(msg);
		checking_input_conveyor_ready = true;
		send_conveyor_state_request(msg.get_conveyor_id());
	}
	else {
		send_message_to_driver(msg);
	}
}

void ANTServer::continue_flow_control()
{
	if (!checking_input_conveyor_ready) {
		queue_handler->set_flow_continue();
	}
}

void ANTServer::receive_ANT_message(unsigned char* payload)
{
	int msg_id = payload[ANT_RESPONSE_ID_INDEX];
	switch (msg_id) {
	
	case MESSAGE_RECEIVED_ID:
	{
		int msg_id = payload[(+MessageReceivedIndex::MSG_ID) + ANT_RESPONSE_HEADER_LENGTH];
		int conveyor_id = payload[(+MessageReceivedIndex::CONVEYOR_ID) + ANT_RESPONSE_HEADER_LENGTH];
		if (waiting_for_receive && waiting_msg_id == msg_id && waiting_conveyor_id == conveyor_id) {
			std::cout << "ANTServer: Received message that we were waiting for" << std::endl;
			waiting_for_receive = false;
			waiting_msg_id = -1;
			waiting_conveyor_id = EMPTY_CONVEYOR_ID;

			// Continue flow control if we are not doing an internal wait for the conveyor to be ready
			if (msg_id != CONVEYOR_STATE_RESPONSE_ID) {
				continue_flow_control();
			}
		}
		break;
	}

	case CONVEYOR_STATE_RESPONSE_ID:
	{
		int conveyor_id = payload[(+ConveyorStateResponseIndex::CONVEYOR_ID) + ANT_RESPONSE_HEADER_LENGTH];
		bool is_available = payload[(+ConveyorStateResponseIndex::AVAILABLE) + ANT_RESPONSE_HEADER_LENGTH];
		received_input_conveyor_state(conveyor_id, is_available);
		break;
	}

	case CLEAR_BOX_RESPONSE_ID:
	{
		int num_packages = payload[+ClearBoxResponsePayloadIndex::NUM_PACKAGES];
		queue_handler->push_message(CLEAR_BOX_RESPONSE_ID, &payload[ANT_RESPONSE_HEADER_LENGTH], CLEAR_BOX_RESPONSE_STATIC_LENGTH + num_packages);
		break;
	}

	default:
	{
		std::cout << "ANTServer: Received unknown ANT ACK message with id: " << std::to_string(msg_id) << std::endl;
		break;
	}
	}
}

void ANTServer::received_input_conveyor_state(int conveyor_id, bool is_available)
{
	if (pending_input_conveyor_msg != nullptr) {
		if (is_available) {
			checking_input_conveyor_ready = false;
			send_message_to_driver(*pending_input_conveyor_msg);
			delete pending_input_conveyor_msg;
			pending_input_conveyor_msg = nullptr;
		}
		else {
			// Keep sending until the conveyor is ready
			send_conveyor_state_request(conveyor_id);
		}

	} else {
		std::cout << "ANTServer: Received conveyor status command with no pending message. Most likely a duplicate msg" << std::endl;
	}

}

void ANTServer::send_conveyor_state_request(int conveyor_id)
{
	ANTMessage msg = ANTMessage(
		CONVEYOR_STATE_ID,
		conveyor_id
	);

	send_message_to_driver(msg);
}

void ANTServer::send_message_to_driver(ANTMessage& msg) {
	waiting_for_receive = true;
	waiting_conveyor_id = msg.get_conveyor_id();
	waiting_msg_id = msg.get_id();

	unsigned char* msg_arr = new unsigned char[msg.get_length() + ANT_MSG_HEADER_LENGTH];
	msg_arr[ANT_MSG_CONVEYOR_ID_INDEX] = msg.get_conveyor_id();
	msg_arr[ANT_MSG_ID_INDEX] = msg.get_id();

	for (int i = 0; i < msg.get_length(); i++) {
		msg_arr[ANT_MSG_HEADER_LENGTH + i] = msg.get_data()[i];
	}

	Send_message_to_ANT(msg_arr, msg.get_length() + ANT_MSG_HEADER_LENGTH);
	delete[] msg_arr;
}
