#include "WebSocketQueueHandler.h"

#include <iostream>

WebSocketQueueHandler::WebSocketQueueHandler(SendReceiveQueue<WebSocketMessage>* queues) : CentralComputerThread(), ws_server(NULL)
{
	this->message_queues = queues;
}

void WebSocketQueueHandler::operator()()
{
	while (!terminate_requested) {
		while (!message_queues->receive_queue_is_empty()) {
			if (ws_server != NULL) {
				std::string msg = message_queues->receive_message().to_string();
				ws_server->send(msg);
			}
			else {
				std::cout << "Web Queue Handler: Attempting to send message to client when server is not set." << std::endl;
			}

		}
		sleep(WEBSOCKET_QUEUE_THREAD_SLEEP_MS);
	}
}

void WebSocketQueueHandler::push_message(std::string message)
{
	WebSocketMessage msg(message);
	message_queues->send_message(msg);
}

void WebSocketQueueHandler::set_websocket_server(WebSocketServer* server) {
	ws_server = server;
}