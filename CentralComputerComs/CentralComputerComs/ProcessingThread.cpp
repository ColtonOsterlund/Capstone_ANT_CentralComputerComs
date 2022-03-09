#include "ProcessingThread.h"


ProcessingThread::ProcessingThread(SendReceiveQueue<WebSocketMessage>* web_queues, SendReceiveQueue<ANTMessage>* ant_queues): CentralComputerThread(), web_handler(web_queues), ant_handler(ant_queues), conveyor_system() {
	web_handler.set_conveyor_system(&conveyor_system);
}

void ProcessingThread::operator()()
{
	bool message_processed = false;

	while (!terminate_requested) {
		message_processed |= web_handler.handle_message();

		/* Sleep if no messages were processed */
		if (!message_processed) {
			this->sleep(PROCESSING_THREAD_SLEEP_MS);
		}
	}
}
