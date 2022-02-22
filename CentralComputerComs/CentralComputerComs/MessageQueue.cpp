#include "MessageQueue.h"

template<class T>
MessageQueue<T>::MessageQueue() : queue(), queue_mutex() {}

template<class T>
MessageQueue<T>::~MessageQueue() {}

template<class T>
void MessageQueue<T>::push(T message)
{
	std::lock_guard<std::mutex> guard(queue_mutex);
	queue.push(message);
}

template<class T>
T MessageQueue<T>::pop()
{
	std::lock_guard<std::mutex> guard(queue_mutex);
	T m = queue.front();
	queue.pop();
	return m;
}

template<class T>
bool MessageQueue<T>::is_empty()
{
	std::lock_guard<std::mutex> guard(queue_mutex);
	return queue.empty();
}
