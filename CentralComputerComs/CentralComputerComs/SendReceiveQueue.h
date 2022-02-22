#pragma once

#include "MessageQueue.h"

/*
* Wrapper class for a send queue and a receive queue for communication between threads. 
*/
template<class T>
class SendReceiveQueue
{
public:
	SendReceiveQueue(MessageQueue<T>* receive_queue, MessageQueue<T>* send_queue);

	/*
	* Add a message to the send queue
	*/
	void send_message(T message);
	
	/*
	* Receive a message from the receive queue
	* 
	* NOTE: The caller MUST check if the receive queue is empty before calling this method
	*/
	T receive_message();

	/*
	* Returns true if the receive queue is empty. False otherwise
	*/
	bool receive_queue_is_empty();

private:
	MessageQueue<T>* receive_queue;
	MessageQueue<T>* send_queue;
};

