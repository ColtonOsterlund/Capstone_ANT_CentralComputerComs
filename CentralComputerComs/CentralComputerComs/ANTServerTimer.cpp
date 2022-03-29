#include "ANTServerTimer.h"

#include <chrono>
#include <string>

ANTServerTimer::ANTServerTimer(ANTServer* server) : CentralComputerThread(), server(server), timer_is_running(false), timer_thread(nullptr) {}

void ANTServerTimer::start_timer()
{
	if (timer_is_running) {
		std::cout << "WARNING: ANTServerTimer was started while already running" << std::endl;
		return;
	}

	std::cout << "Starting timer" << std::endl;
	timer_is_running = true;
	timer_thread = new std::thread(std::ref(*this));
}

void ANTServerTimer::stop_timer()
{
	std::cout << "Stopping timer" << std::endl;
	request_termination();
	if (timer_thread != nullptr && timer_thread->joinable()) {
		timer_thread->join();
		delete timer_thread;
	}
}

void ANTServerTimer::reset_timer() {
	std::cout << "Resetting timer" << std::endl;
	terminate_requested = false;
	timer_is_running = false;
}

void ANTServerTimer::operator()()
{
	std::cout << "Timer running in thread" << std::endl;
	auto start_time = std::chrono::steady_clock::now();

	bool timeout_occured = false;

	while (!terminate_requested || timeout_occured) {
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
