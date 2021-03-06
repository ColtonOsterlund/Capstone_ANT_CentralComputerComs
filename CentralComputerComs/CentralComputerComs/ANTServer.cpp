#include "ANTServer.h"

#include <iostream>

#include "BackendANTDriver.h"
#include "ant_messages.h"
#include "central_computer_types.h"

constexpr auto ANT_BACKEND_SERVER_DEVICE_NUMBER = 0;

ANTServer::ANTServer(): queue_handler(nullptr), pending_input_conveyor_msg(nullptr), checking_input_conveyor_ready(false), waiting_for_receive(false), waiting_conveyor_id(EMPTY_CONVEYOR_ID), waiting_msg_id(-1), pending_msg(nullptr), timer(nullptr), timestamp(0)
{
	timer = new ANTServerTimer(this);
	Set_backend_ANT_server(this);
}

void ANTServer::operator()()
{
	Run_driver(ANT_BACKEND_SERVER_DEVICE_NUMBER);
	timer->stop_timer();
	timer->request_termination();
	timer->join();
	delete timer;
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
	else if (msg.get_id() == CLEAR_BOX_ID) {
		wait_for_message(CLEAR_BOX_RESPONSE_ID, msg.get_conveyor_id());
		send_message_to_driver(msg, ++timestamp);
	}
	else {
		wait_for_message(msg.get_id(), msg.get_conveyor_id());
		send_message_to_driver(msg, ++timestamp);
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
			timer->stop_timer();
			std::cout << "ANTServer: Received message that we were waiting for" << std::endl;
			waiting_for_receive = false;
			waiting_msg_id = -1;
			waiting_conveyor_id = EMPTY_CONVEYOR_ID;
			
			if (pending_msg != nullptr) {
				delete pending_msg;
				pending_msg = nullptr;
			}

			continue_flow_control();
		}
		break;
	}

	case CONVEYOR_STATE_RESPONSE_ID:
	{
		int conveyor_id = payload[(+ConveyorStateResponseIndex::CONVEYOR_ID) + ANT_RESPONSE_HEADER_LENGTH];
		bool is_available = payload[(+ConveyorStateResponseIndex::AVAILABLE) + ANT_RESPONSE_HEADER_LENGTH];
		if (waiting_for_receive && waiting_msg_id == msg_id && waiting_conveyor_id == conveyor_id) {
			timer->stop_timer();
			received_input_conveyor_state(conveyor_id, is_available);
		}
		break;
	}

	case CLEAR_BOX_RESPONSE_ID:
	{
		if (waiting_for_receive && waiting_msg_id == msg_id) {
			timer->stop_timer();
			waiting_for_receive = false;
			waiting_msg_id = -1;
			waiting_conveyor_id = EMPTY_CONVEYOR_ID;

			if (pending_msg != nullptr) {
				delete pending_msg;
				pending_msg = nullptr;
			}

			int num_packages = payload[+ClearBoxResponsePayloadIndex::NUM_PACKAGES];
			queue_handler->push_message(CLEAR_BOX_RESPONSE_ID, &payload[ANT_RESPONSE_HEADER_LENGTH], CLEAR_BOX_RESPONSE_STATIC_LENGTH + num_packages);

			continue_flow_control();
		}

		break;
	}

	case PACKAGE_ARRIVED_ID:
	{
		queue_handler->push_message(PACKAGE_ARRIVED_ID, &payload[ANT_RESPONSE_HEADER_LENGTH], PACKAGE_ARRIVED_LENGTH);
		break;
	}


	default:
	{
		std::cout << "ANTServer: Received unknown ANT ACK message with id: " << std::to_string(msg_id) << std::endl;
		break;
	}

	} // END SWITCH
}

void ANTServer::pending_message_timed_out()
{
	if (pending_msg != nullptr) {
		std::cout << "\nMessage timed out\n" << std::endl;
		ANTMessage msg(pending_msg->get_id(), pending_msg->get_conveyor_id(), pending_msg->get_data(), pending_msg->get_length());
		send_message_to_driver(msg, timestamp);
	}
}

void ANTServer::received_input_conveyor_state(int conveyor_id, bool is_available)
{
	if (pending_input_conveyor_msg != nullptr) {
		if (is_available) {
			checking_input_conveyor_ready = false;
			wait_for_message(ADD_PACKAGE_ID, conveyor_id);
			send_message_to_driver(*pending_input_conveyor_msg, ++timestamp);
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

	wait_for_message(CONVEYOR_STATE_RESPONSE_ID, conveyor_id);
	send_message_to_driver(msg, ++timestamp);
}

void ANTServer::send_message_to_driver(ANTMessage& msg, unsigned char time) {
	if (pending_msg != nullptr) {
		delete pending_msg;
		pending_msg = nullptr;
	}

	pending_msg = new ANTMessage(msg);

	unsigned char* msg_arr = new unsigned char[8]; //msg.get_length() + ANT_MSG_HEADER_LENGTH];
	msg_arr[ANT_MSG_CONVEYOR_ID_INDEX] = msg.get_conveyor_id();
	msg_arr[ANT_MSG_ID_INDEX] = msg.get_id();
	msg_arr[ANT_MSG_TIMESTAMP_INDEX] = time;

	for (int i = 0; i < msg.get_length(); i++) {
		msg_arr[ANT_MSG_HEADER_LENGTH + i] = msg.get_data()[i];
	}

	timer->start_timer();

	// Add time as well
	Send_message_to_ANT(msg_arr, ANT_BACKEND_MSG_MAX_SIZE);
	delete[] msg_arr;
}

void ANTServer::wait_for_message(int msg_id, int conveyor_id)
{
	waiting_for_receive = true;
	waiting_conveyor_id = conveyor_id;
	waiting_msg_id = msg_id;
}
