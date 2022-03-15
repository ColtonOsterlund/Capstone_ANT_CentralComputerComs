#pragma once

#include "ProcessingThreadMessageHandler.h"
#include "ANTMessage.h"
#include "SendReceiveQueue.h"
#include "ConveyorSystem.h"

#include "central_computer_types.h"


class ANTThreadMessageHandler: public ProcessingThreadMessageHandler<ANTMessage>
{
public:
	ANTThreadMessageHandler(SendReceiveQueue<ANTMessage>* queues);

	bool handle_message() override;

	void set_conveyor_system(ConveyorSystem* system);

	void send_conveyor_disconnect_msg(int conveyor_id);

	void send_conveyor_connect_msg(int conveyor_id, int connection_id, ConveyorConnectionType conn_type);

private:
	ConveyorSystem* conveyor_system = NULL;

};

