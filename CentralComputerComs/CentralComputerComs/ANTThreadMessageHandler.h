#pragma once

#include "ProcessingThreadMessageHandler.h"
#include "ANTMessage.h"
#include "SendReceiveQueue.h"
#include "ConveyorSystem.h"


class ANTThreadMessageHandler: public ProcessingThreadMessageHandler<ANTMessage>
{
public:
	ANTThreadMessageHandler(SendReceiveQueue<ANTMessage>* queues);

	bool handle_message() override;

	void set_conveyor_system(ConveyorSystem* system);

private:
	ConveyorSystem* conveyor_system = NULL;

};

