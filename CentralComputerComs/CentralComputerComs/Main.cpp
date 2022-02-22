
#include <thread>
#include <iostream>

#include "MessageQueue.h"
#include "ANTThread.h"
#include "WebSocketThread.h"

#include "ANTMessage.h"
#include "WebSocketMessage.h"
#include "SendReceiveQueue.h"


int main() {
	MessageQueue<ANTMessage> ANTRxQueue;
	MessageQueue<ANTMessage> ANTTxQueue;

	MessageQueue<WebSocketMessage> WebRxQueue;
	MessageQueue<WebSocketMessage> WebTxQueue;

	/* The web socket will send to the rx queue and receive from the tx queue */
	SendReceiveQueue<ANTMessage> ant_thread_queues = SendReceiveQueue<ANTMessage>(&ANTTxQueue, &ANTRxQueue);
	SendReceiveQueue<WebSocketMessage> websocket_thread_queues = SendReceiveQueue<WebSocketMessage>(&WebTxQueue, &WebRxQueue);


	ANTThread ant_thread(&ant_thread_queues);
	WebSocketThread web_thread(&websocket_thread_queues);

	std::thread main_ant_thread(std::ref(ant_thread));
	std::thread main_web_thread(std::ref(web_thread));
	
	main_ant_thread.join();
	main_web_thread.join();
}