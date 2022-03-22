#include "WebThreadMessageHandler.h"

#include <iostream>
#include <json.hpp>

#include "websocket_messages.h"

using json = nlohmann::json;


WebThreadMessageHandler::WebThreadMessageHandler(SendReceiveQueue<WebSocketMessage>* queues): ProcessingThreadMessageHandler<WebSocketMessage>(queues) {}

bool WebThreadMessageHandler::handle_message()
{
	bool message_handled = false;
	bool vars_set = true;

	if (queues == NULL) {
		std::cout << "SendReceiveQueue not set for WebThreadMessageHandler" << std::endl;
		vars_set = false;
	}
	else if (conveyor_system == NULL) {
		std::cout << "ConveyorSystem not set for WebThreadMessageHandler" << std::endl;
		vars_set = false;
	}


	if (vars_set && !queues->receive_queue_is_empty()) {
		WebSocketMessage msg = queues->receive_message();
		std::cout << "Message processed by WebThreadMessageHandler: " << msg.to_string() << std::endl;
		
		switch (msg.get_id()) {
			case CONVEYOR_STATE_MSG:
				conveyor_system->set_state(msg.get_data());
				break;
			
			case ADD_DESTINATION_BOX:
				conveyor_system->add_destination_box(msg.get_data());
				break;

			case ADD_PACKAGE:
				conveyor_system->send_package(msg.get_data());
				break;

			case DESTINATION_BOX_STATUS:
				conveyor_system->get_destination_box_state(msg.get_data()["box_id"]);
				break;

			default:
				std::cout << "ProcessingThread: Unknown Websocket message with id: " << std::to_string(msg.get_id()) << std::endl;
		}

		message_handled = true;
	}

	return message_handled;
}

void WebThreadMessageHandler::set_conveyor_system(ConveyorSystem* system)
{
	this->conveyor_system = system;
}

void WebThreadMessageHandler::send_package_add_success(int package_id, int box_id)
{
	send_package_add_response(package_id, true, box_id, "");
}

void WebThreadMessageHandler::send_package_add_failure(int package_id, std::string error)
{
	send_package_add_response(package_id, false, -1, error);
}

void WebThreadMessageHandler::send_package_received_confirmation(int package_id, int box_id)
{
	json msg_json;
	msg_json["id"] = PACKAGE_RECEIVED_ACK;
	msg_json["package_id"] = package_id;
	msg_json["box_id"] = box_id;

	WebSocketMessage msg = WebSocketMessage(msg_json);
	enqueue_message(msg);
}

void WebThreadMessageHandler::send_destination_box_state(int box_id, int package_type, std::set<int> packages_in_transit, std::set<int> packages_stored)
{
	json msg_json;
	json packages_in_transit_j = json(packages_in_transit);
	json packages_stored_j = json(packages_stored);

	msg_json["id"] = DESTINATION_BOX_STATUS_RESPONSE;
	msg_json["box_id"] = box_id;
	msg_json["package_type"] = package_type;
	msg_json["packages_in_transit"] = packages_in_transit_j;
	msg_json["packages_stored"] = packages_stored_j;

	WebSocketMessage msg = WebSocketMessage(msg_json);
	enqueue_message(msg);
}

void WebThreadMessageHandler::send_package_add_response(int package_id, bool success, int box_id, std::string details) {
	json msg_json;
	msg_json["id"] = ADD_PACKAGE_RESPONSE;
	msg_json["package_id"] = package_id;
	msg_json["success"] = success;

	if (box_id != -1) {
		msg_json["box_id"] = box_id;

	}

	if (!details.empty()) {
		msg_json["details"] = details;
	}

	WebSocketMessage msg = WebSocketMessage(msg_json);
	enqueue_message(msg);
}
