#include "ANTThreadMessageHandler.h"

ANTThreadMessageHandler::ANTThreadMessageHandler(SendReceiveQueue<ANTMessage>* queues): ProcessingThreadMessageHandler(queues) {}

bool ANTThreadMessageHandler::handle_message()
{
	bool message_handled = false;
	bool vars_set = true;

	if (queues == NULL) {
		std::cout << "SendReceiveQueue not set for ANTThreadMessageHandler" << std::endl;
		vars_set = false;
	}
	else if (conveyor_system == NULL) {
		std::cout << "ConveyorSystem not set for ANTThreadMessageHandler" << std::endl;
		vars_set = false;
	}


	if (vars_set && !queues->receive_queue_is_empty()) {
		ANTMessage msg = queues->receive_message();
		// TODO: Handle message here
		std::cout << "Message processed by ANTThreadMessageHandler" << std::endl;
		message_handled = true;
	}

	return message_handled;
}

void ANTThreadMessageHandler::set_conveyor_system(ConveyorSystem* system)
{
	this->conveyor_system = system;
}