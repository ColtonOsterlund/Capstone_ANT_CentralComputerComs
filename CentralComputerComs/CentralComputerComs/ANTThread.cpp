#include <iostream>


#include "ANTThread.h"

ANTThread::ANTThread(SendReceiveQueue<ANTMessage>* queues): CentralComputerThread(), ant_server(), queue_handler(queues) {
	ant_server.set_queue_handler(&queue_handler);
	queue_handler.set_ant_handler(&ant_server);
}

void ANTThread::operator()()
{
	std::thread queue_processing_thread(std::ref(queue_handler));
	std::thread ant_server_thread(std::ref(ant_server));

	while (!terminate_requested) {
		sleep();
	}

	queue_handler.request_termination();
	ant_server.request_termination();

	queue_processing_thread.join();
	ant_server_thread.join();
}
