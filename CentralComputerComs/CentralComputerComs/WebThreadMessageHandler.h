#pragma once

#include <string>

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

	void send_package_add_success();

	void send_package_add_failure(std::string error);


private:
	void send_package_add_response(bool success, std::string details);

	ConveyorSystem* conveyor_system = NULL;

};

