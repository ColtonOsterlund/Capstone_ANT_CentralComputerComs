#include <thread>
#include <iostream>
#include "MessageQueue.h"

void mock_function(MessageQueue& RxQueue, MessageQueue& TxQueue) {
	std::cout << "mocking threads" << std::endl;
}

int main() {
	MessageQueue ANTRxQueue;
	MessageQueue ANTTxQueue;

	MessageQueue WebRxQueue;
	MessageQueue WebTxQueue;

	std::thread ant_thread(mock_function, std::ref(ANTRxQueue), std::ref(ANTTxQueue));
	std::thread web_thread(mock_function, std::ref(WebRxQueue), std::ref(WebTxQueue));
	
	ant_thread.join();
	web_thread.join();
}