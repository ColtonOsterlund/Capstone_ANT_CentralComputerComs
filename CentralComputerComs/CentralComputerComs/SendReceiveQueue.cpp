#include <iostream>

#include "SendReceiveQueue.h"

template<class T>
SendReceiveQueue<T>::SendReceiveQueue(MessageQueue<T>* receive_queue, MessageQueue<T>* send_queue)
{
    this->receive_queue = receive_queue;
    this->send_queue = send_queue;
}

template<class T>
void SendReceiveQueue<T>::send_message(T message)
{
    send_queue->push(message);
}

template<class T>
T SendReceiveQueue<T>::receive_message()
{
    if (receive_queue_is_empty()) {
        std::cout << "Popping message from empty queue!" << std::endl;
    }
    return send_queue->pop();
}

template<class T>
bool SendReceiveQueue<T>::receive_queue_is_empty()
{
    return receive_queue->is_empty();
}
