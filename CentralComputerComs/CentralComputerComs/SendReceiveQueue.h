#pragma once

#include <iostream>

#include "MessageQueue.h"

/*
* Wrapper class for a send queue and a receive queue for communication between threads. 
* 
* NOTE: implemented in the header file to remove linker errors due to templates
*/
template<class T>
class SendReceiveQueue
{
public:
	SendReceiveQueue(MessageQueue<T>* receive_queue, MessageQueue<T>* send_queue)
	{
		this->receive_queue = receive_queue;
		this->send_queue = send_queue;
	}

	/*
	* Add a message to the send queue
	*/
	void send_message(T& message)
	{
		send_queue->push(message);
	}
	
	/*
	* Receive a message from the receive queue
	* 
	* NOTE: The caller MUST check if the receive queue is empty before calling this method
	*/
	T receive_message()
	{
		if (receive_queue_is_empty()) {
			std::cout << "Popping message from empty queue!" << std::endl;
		}
		return receive_queue->pop();
	}

	/*
	* Returns true if the receive queue is empty. False otherwise
	*/
	bool receive_queue_is_empty()
	{
		return receive_queue->is_empty();
	}


private:
	MessageQueue<T>* receive_queue;
	MessageQueue<T>* send_queue;
};