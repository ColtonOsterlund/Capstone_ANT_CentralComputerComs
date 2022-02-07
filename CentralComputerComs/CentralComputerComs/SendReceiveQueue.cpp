#include <iostream>

#include "SendReceiveQueue.h"

SendReceiveQueue::SendReceiveQueue(MessageQueue* receive_queue, MessageQueue* send_queue)
{
    this->receive_queue = receive_queue;
    this->send_queue = send_queue;
}

void SendReceiveQueue::send_message(int id, unsigned char* data, int length)
{
    Message msg = Message(id, data, length);
    send_queue->push(msg);
}

Message SendReceiveQueue::receive_message()
{
    if (receive_queue_is_empty()) {
        std::cout << "Popping message from empty queue!" << std::endl;
    }
    return send_queue->pop();
}

bool SendReceiveQueue::receive_queue_is_empty()
{
    return receive_queue->is_empty();
}
