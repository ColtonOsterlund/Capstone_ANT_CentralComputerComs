#pragma once

#include "ProcessingThreadMessageHandler.h"
#include "ConveyorSystem.h"
#include "SendReceiveQueue.h"
#include "WebSocketMessage.h"


class WebThreadMessageHandler: public ProcessingThreadMessageHandler<WebSocketMessage>
{
public:
	WebThreadMessageHandler(SendReceiveQueue<WebSocketMessage>* queues);

	/* Check if there are messages in the queue and handle them. Return true if a message was handled and false otherwise.*/
	bool handle_message() override;

	void set_conveyor_system(ConveyorSystem* system);

private:
	ConveyorSystem* conveyor_system = NULL;

};

