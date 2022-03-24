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

	void send_destination_box_connect_msg(int conveyor_id, int destination_box);

	void send_package_to_input(int package_id, PackageType pkg);

	void send_remove_package_msg(int package_id, int box_id);

	void send_clear_box_msg(int box_id);

private:
	ConveyorSystem* conveyor_system = NULL;

};

