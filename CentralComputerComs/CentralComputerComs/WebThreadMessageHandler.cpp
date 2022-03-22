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

void WebThreadMessageHandler::send_package_add_success()
{
	send_package_add_response(true, "");
}

void WebThreadMessageHandler::send_package_add_failure(std::string error)
{
	send_package_add_response(false, error);
}

void WebThreadMessageHandler::send_package_add_response(bool success, std::string details) {
	json msg_json;
	msg_json["id"] = ADD_PACKAGE_RESPONSE;
	msg_json["success"] = success;

	if (!details.empty()) {
		msg_json["details"] = details;
	}

	WebSocketMessage msg = WebSocketMessage(msg_json);
	enqueue_message(msg);
}
