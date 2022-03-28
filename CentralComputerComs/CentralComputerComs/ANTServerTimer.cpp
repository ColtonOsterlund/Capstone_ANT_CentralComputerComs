#include "ANTServerTimer.h"

#include <chrono>

ANTServerTimer::ANTServerTimer(ANTServer* server) : CentralComputerThread(), server(server), timer_is_running(false), timer_thread() {}

void ANTServerTimer::start_timer()
{
	if (timer_is_running) {
		std::cout << "WARNING: ANTServerTimer was started while already running" << std::endl;
		return;
	}

	timer_is_running = true;
	timer_thread = std::thread(std::ref(*this));
}

void ANTServerTimer::stop_timer()
{
	request_termination();
	if (timer_thread.joinable()) {
		timer_thread.join();
	}
}

void ANTServerTimer::reset_timer() {
	terminate_requested = false;
	timer_is_running = false;
}

void ANTServerTimer::operator()()
{
	auto start_time = std::chrono::steady_clock::now();

	bool timeout_occured = false;

	while (!terminate_requested) {
		auto time_now = std::chrono::steady_clock::now();
		std::chrono::duration<double> diff = time_now - start_time;
		if (diff.count() >= ANT_SERVER_TIMER_TIMEOUT_S) {
			timeout_occured = true;
		}
		else {
			sleep(ANT_SERVER_TIMER_SLEEP_MS);
		}
	}

	reset_timer();

	if (timeout_occured) {
		server->pending_message_timed_out();
	}
}
