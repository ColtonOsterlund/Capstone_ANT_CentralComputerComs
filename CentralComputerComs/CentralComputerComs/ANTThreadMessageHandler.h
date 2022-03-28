#pragma once

#include "ProcessingThreadMessageHandler.h"
#include "BackendANTMessage.h"
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

	void send_conveyor_connect_msg(int conveyor_id, int connection_id, ConveyorConnectionType conn_type, ConveyorLocationType location);

	void send_destination_box_connect_msg(int conveyor_id, int destination_box, int location);

	void send_package_to_input(int box_id, int package_id, PackageType pkg);

	void send_remove_package_msg(int conveyor_id, int package_id, int box_id);

	void send_clear_box_msg(int conveyor_id, int box_id);
	
	void send_routing_plan_msg(int conveyor_id, int box_id, int target_conveyor);

private:
	void handle_clear_box_response_msg(ANTMessage& msg);

	void handle_package_arrived_msg(ANTMessage& msg);

	ConveyorSystem* conveyor_system = NULL;

};

