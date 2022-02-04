#include "MessageQueue.h"

MessageQueue::MessageQueue() : queue(), queue_mutex() {}

MessageQueue::~MessageQueue() {}

void MessageQueue::push(Message message)
{
	std::lock_guard<std::mutex> guard(queue_mutex);
	queue.push(message);
}

Message MessageQueue::pop()
{
	std::lock_guard<std::mutex> guard(queue_mutex);
	Message m = queue.front();
	queue.pop();
	return m;
}

bool MessageQueue::is_empty()
{
	std::lock_guard<std::mutex> guard(queue_mutex);
	return queue.empty();
}
