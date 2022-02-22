#pragma once

#include <queue>
#include <mutex>

/*
* Thread-safe queue implementation for passing message between threads.
* 
* NOTE: implemented in the header file to remove linker errors due to templates
*/
template <class T>
class MessageQueue
{
public:
	MessageQueue() : queue(), queue_mutex() {}
	~MessageQueue() {}

	/*
	* Adds the passed message to the queue
	*/
	void push(T message)
	{
		std::lock_guard<std::mutex> guard(queue_mutex);
		queue.push(message);
	}

	/*
	* Pops the next message in the queue and returns it
	* 
	* The caller MUST check if the queue is empty before calling this function.
	*/
	T pop()
	{
		std::lock_guard<std::mutex> guard(queue_mutex);
		T m = queue.front();
		queue.pop();
		return m;
	}

	/*
	* Returns true if the queue is empty and false otherwise.
	*/
	bool is_empty()
	{
		std::lock_guard<std::mutex> guard(queue_mutex);
		return queue.empty();
	}

	
private:
	std::queue<T> queue;
	std::mutex queue_mutex;
};
