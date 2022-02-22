#pragma once

#include <queue>
#include <mutex>

/*
* Thread-safe queue implementation for passing message between threads.
*/
template <class T>
class MessageQueue
{
public:
	MessageQueue();
	~MessageQueue();

	/*
	* Adds the passed message to the queue
	*/
	void push(T message);

	/*
	* Pops the next message in the queue and returns it
	* 
	* The caller MUST check if the queue is empty before calling this function.
	*/
	T pop();

	/*
	* Returns true if the queue is empty and false otherwise.
	*/
	bool is_empty();
	
private:
	std::queue<T> queue;
	std::mutex queue_mutex;
};

