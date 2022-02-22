#include <thread>

#include "WebSocketThread.h"

WebSocketThread::WebSocketThread(SendReceiveQueue<WebSocketMessage>* queues) : CentralComputerThread(), socket_server(), queue_handler(queues) {
	socket_server.set_queue_handler(&queue_handler);
	queue_handler.set_websocket_server(&socket_server);
}

void WebSocketThread::operator()()
{

	std::thread queue_processing_thread(std::ref(queue_handler));
	std::thread websocket_server_thread(std::ref(socket_server));

	while (!terminate_requested) {
		sleep();
	}

	queue_handler.request_termination();
	socket_server.request_termination();

	queue_processing_thread.join();
	websocket_server_thread.join();
}

