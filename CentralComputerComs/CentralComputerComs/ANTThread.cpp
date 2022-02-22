#include <iostream>


#include "ANTThread.h"

ANTThread::ANTThread(SendReceiveQueue<ANTMessage>* queues)
{

}

void ANTThread::operator()()
{
	std::cout << "hello from ANT Thread" << std::endl;
}
