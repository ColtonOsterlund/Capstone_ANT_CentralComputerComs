#include "ANTQueueHandler.h"

#include <iostream>

ANTQueueHandler::ANTQueueHandler(SendReceiveQueue<ANTMessage>* queues): CentralComputerThread(), flow_control(FlowControl::CONTINUE), queues(queues), ant_server(nullptr) {}

void ANTQueueHandler::operator()()
{
	while (!terminate_requested) {
		while (!queues->receive_queue_is_empty()) {
			if (flow_control == FlowControl::CONTINUE && ant_server != nullptr) {

				ANTMessage msg = queues->receive_message();
				std::cout << msg.to_string() << std::endl;
				set_flow_stop();
				ant_server->send_ANT_message(msg);
			}
			else if (flow_control == FlowControl::STOP) {
				sleep(ANT_QUEUE_HANDLER_FLOW_CONTROL_SLEEP_TIMER);
			}
			else {
				std::cout << "ANT Queue Handler: Attempting to send message to client when server is not set." << std::endl;
			}
		}
		sleep(ANT_QUEUE_HANDLER_SLEEP_TIMER);
	}
}

void ANTQueueHandler::push_message(int id, unsigned char* payload, int length)
{
	ANTMessage msg = ANTMessage(id, payload, length);

	queues->send_message(msg);
}

void ANTQueueHandler::set_ant_handler(ANTServer* server)
{
	ant_server = server;
}

void ANTQueueHandler::set_flow_stop() {
	flow_control = FlowControl::STOP;
}

void ANTQueueHandler::set_flow_continue() {
	flow_control = FlowControl::CONTINUE;
}

