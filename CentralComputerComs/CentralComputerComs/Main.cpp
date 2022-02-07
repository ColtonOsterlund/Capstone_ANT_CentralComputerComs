
#include <thread>
#include <iostream>

#include "MessageQueue.h"
#include "ANTThread.h"
#include "WebSocketThread.h"

void mock_function(MessageQueue& RxQueue, MessageQueue& TxQueue) {
	std::cout << "mocking threads" << std::endl;
}

int main() {
	MessageQueue ANTRxQueue;
	MessageQueue ANTTxQueue;

	MessageQueue WebRxQueue;
	MessageQueue WebTxQueue;

	ANTThread ant_thread = ANTThread(&ANTRxQueue, &ANTTxQueue);
	WebSocketThread web_thread = WebSocketThread(&WebRxQueue, &WebTxQueue);

	std::thread main_ant_thread(ant_thread);
	std::thread main_web_thread(web_thread);
	
	main_ant_thread.join();
	main_web_thread.join();
}