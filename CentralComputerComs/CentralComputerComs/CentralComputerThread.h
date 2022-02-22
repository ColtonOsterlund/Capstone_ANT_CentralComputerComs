#pragma once
#define DEFAULT_THREAD_SLEEP 1000 //1 second

/*
* Abstract thread class that includes a terminate_requested variable to track if the thread should clean up and close.
* 
*/
class CentralComputerThread
{
public:
	CentralComputerThread() : terminate_requested{ false } {}
	
	/*
	* Override this operator with the main body of the thread
	*/
	virtual void operator()() = 0;

	/*
	* Sleep this thread for the default amount
	*/
	void sleep();

	/*
	* Sleep this thread for the passed number of milliseconds
	*/
	void sleep(int milliseconds);

	/*
	* Request the thread to terminate
	*/
	virtual void request_termination();

protected:
	/*
	* Threads inheriting from this class must check for this variable in their main event loop to know when to exit
	*/
	bool terminate_requested;
};

