#include <iostream>


#include "ANTThread.h"

ANTThread::ANTThread(MessageQueue* RxQueue, MessageQueue* TxQueue)
{

}

void ANTThread::operator()()
{
	std::cout << "hello from ANT Thread" << std::endl;
}
