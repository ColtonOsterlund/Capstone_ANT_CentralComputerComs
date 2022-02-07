#pragma once

#include "MessageQueue.h"
#include "Message.h"

/*
* Wrapper class for a send queue and a receive queue for communication between threads. 
*/
class SendReceiveQueue
{
public:
	SendReceiveQueue(MessageQueue* receive_queue, MessageQueue* send_queue);

	/*
	* Add a message to the send queue
	*/
	void send_message(int id, unsigned char* data, int length);
	
	/*
	* Receive a message from the receive queue
	* 
	* NOTE: The caller MUST check if the receive queue is empty before calling this method
	*/
	Message receive_message();

	/*
	* Returns true if the receive queue is empty. False otherwise
	*/
	bool receive_queue_is_empty();

private:
	MessageQueue* receive_queue;
	MessageQueue* send_queue;
};

