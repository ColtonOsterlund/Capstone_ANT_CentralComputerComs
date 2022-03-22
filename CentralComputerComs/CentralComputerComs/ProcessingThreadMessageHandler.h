#pragma once

#include <iostream>

#include "SendReceiveQueue.h"


template <class T>
class ProcessingThreadMessageHandler
{
public:
	ProcessingThreadMessageHandler(SendReceiveQueue<T>* queues) {
		this->queues = queues;
	}

	/* Check if there are messages in the queue and handle them. Return true if a message was handled and false otherwise.*/
	virtual bool handle_message() = 0;

	void enqueue_message(T& msg) {
		if (queues != NULL) {
			queues->send_message(msg);
		}
		else {
			std::cout << "ProcessingThreadMessageHandler child class: Trying to send a message to queue that has not been set" << std::endl;
		}
	}

protected:
	SendReceiveQueue<T>* queues = NULL;
};

