#include <iostream>

#include "WebSocketThread.h"

WebSocketThread::WebSocketThread(MessageQueue* RxQueue, MessageQueue* TxQueue)
{
}

void WebSocketThread::operator()()
{
	std::cout << "hello from WebSocket Thread" << std::endl;
}
