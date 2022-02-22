#include "CentralComputerThread.h"

#include <thread>
#include <chrono>

void CentralComputerThread::sleep()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(DEFAULT_THREAD_SLEEP));
}

void CentralComputerThread::sleep(int milliseconds)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

void CentralComputerThread::request_termination()
{
	terminate_requested = true;
}
