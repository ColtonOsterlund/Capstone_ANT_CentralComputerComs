#include "MainThread.h"

#include <iostream>

#include "ANTThread.h"
#include "WebSocketThread.h"

MainThread::MainThread():
	CentralComputerThread(),
	ANTRxQueue(), ANTTxQueue(), 
	WebRxQueue(), WebTxQueue(), 
	ant_thread_queues(&ANTTxQueue, &ANTRxQueue),			// Will receive from the Tx Queue (to send out) and will send to the Rx Queue (towards the processing thread)
	ant_processing_queues(&ANTRxQueue, &ANTTxQueue),		// Will receive from the Rx Queue and send to the Tx Queue
	websocket_thread_queues(&WebTxQueue, &WebRxQueue),		// Will receive from the Tx Queue (to send out) and will send to the Rx Queue (towards the processing thread)
	websocket_processing_queues(&WebRxQueue, &WebTxQueue)	// Will receive from the Rx Queue and send to the Tx Queue
{

	class_threads[0] = new ANTThread(&ant_thread_queues);
	class_threads[1] = new WebSocketThread(&websocket_thread_queues);

}

MainThread::~MainThread() {
	for (int i = 0; i < NUM_THREADS; i++) {
		delete class_threads[i];
	}
}

void MainThread::operator()()
{
	for (int i = 0; i < NUM_THREADS; i++) {
		actual_threads[i] = std::thread(std::ref(*class_threads[i]));
	}

	get_user_command();

	for (int i = 0; i < NUM_THREADS; i++) {
		actual_threads[i].join();
	}
}

void MainThread::request_termination()
{
	CentralComputerThread::request_termination();
	for (int i = 0; i < NUM_THREADS; i++) {
		class_threads[i]->request_termination();
	}
}

void MainThread::get_user_command()
{
	int selection;

	std::cout << "Enter 0 to terminate the server..." << std::endl;

	while (!terminate_requested) {
		std::cin >> selection;
		switch (selection) {
			case TERMINATE:
				request_termination();
				break;
			default:
				std::cout << "Command not recognized" << std::endl;
		}
	}
}

