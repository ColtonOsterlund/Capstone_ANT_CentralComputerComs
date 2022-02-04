#pragma once

#include <queue>
#include <mutex>

#include "Message.h"

/*
* Thread-safe queue implementation for passing message between threads.
*/
class MessageQueue
{
public:
	MessageQueue();
	~MessageQueue();

	/*
	* Adds the passed message to the queue
	*/
	void push(Message message);

	/*
	* Pops the next message in the queue and returns it
	* 
	* The caller MUST check if the queue is empty before calling this function.
	*/
	Message pop();

	/*
	* Returns true if the queue is empty and false otherwise.
	*/
	bool is_empty();
	
private:
	std::queue<Message> queue;
	std::mutex queue_mutex;
};

