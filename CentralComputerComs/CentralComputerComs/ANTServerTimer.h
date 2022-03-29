#pragma once

#include "ANTServer.h"
#include "CentralComputerThread.h"

// What a mess. i dont want to spend a lot of time doing function pointers to member functions so we do shortcut
class ANTServer;

#define ANT_SERVER_TIMER_TIMEOUT_S 0.5
#define ANT_SERVER_TIMER_SLEEP_MS 5

class ANTServerTimer : public CentralComputerThread
{
public:
	ANTServerTimer(ANTServer* server);

	void start_timer();

	void stop_timer();

	void reset_timer();

	void operator()() override;

private:
	ANTServer* server;
	bool timer_is_running;
	std::thread timer_thread;
};

