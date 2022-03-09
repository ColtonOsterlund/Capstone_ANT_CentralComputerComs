#include "WebThreadMessageHandler.h"

#include <iostream>

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
		// TODO: Handle message here
		std::cout << "Message processed by WebThreadMessageHandler: " << msg.to_string() << std::endl;
		message_handled = true;
	}

	return message_handled;
}

void WebThreadMessageHandler::set_conveyor_system(ConveyorSystem* system)
{
	this->conveyor_system = system;
}
