#include "ANTThreadMessageHandler.h"

#include <iostream>
#include <set>

#include "ant_messages.h"

ANTThreadMessageHandler::ANTThreadMessageHandler(SendReceiveQueue<ANTMessage>* queues): ProcessingThreadMessageHandler(queues) {}

bool ANTThreadMessageHandler::handle_message()
{
	bool message_handled = false;
	bool vars_set = true;

	if (queues == NULL) {
		std::cout << "SendReceiveQueue not set for ANTThreadMessageHandler" << std::endl;
		vars_set = false;
	}
	else if (conveyor_system == NULL) {
		std::cout << "ConveyorSystem not set for ANTThreadMessageHandler" << std::endl;
		vars_set = false;
	}


	if (vars_set && !queues->receive_queue_is_empty()) {
		ANTMessage msg = queues->receive_message();
		std::cout << "Message processed by ANTThreadMessageHandler" << std::endl;

		switch (msg.get_id()) {

			case CLEAR_BOX_RESPONSE_ID:
				handle_clear_box_response_msg(msg);
				break;

			case PACKAGE_ARRIVED_ID:
				handle_package_arrived_msg(msg);
				break;

			default:
				std::cout << "Processing Thread ANT Message Handler: Got message with unknown id: " << std::to_string(msg.get_id()) << std::endl;
				break;
		}

		message_handled = true;
	}

	return message_handled;
}

void ANTThreadMessageHandler::set_conveyor_system(ConveyorSystem* system)
{
	this->conveyor_system = system;
}

void ANTThreadMessageHandler::send_conveyor_disconnect_msg(int conveyor_id)
{
	ANTMessage msg = ANTMessage(
		DISCONNECT_ID,
		conveyor_id
	);

	enqueue_message(msg);
}

void ANTThreadMessageHandler::send_conveyor_connect_msg(int conveyor_id, int connection_id, ConveyorConnectionType conn_type, ConveyorLocationType location)
{
	unsigned char payload[CONVEYOR_CONNECT_LENGTH] = { 0 };

	payload[+ConveyorConnectPayloadIndex::TARGET_ID] = connection_id;
	payload[+ConveyorConnectPayloadIndex::CONNECTION_TYPE] = static_cast<int>(conn_type);
	payload[+ConveyorConnectPayloadIndex::LOCATION] = static_cast<int>(location);

	ANTMessage msg = ANTMessage(
		CONVEYOR_CONNECT_ID,
		conveyor_id,
		payload,
		CONVEYOR_CONNECT_LENGTH
	);

	enqueue_message(msg);
}

void ANTThreadMessageHandler::send_destination_box_connect_msg(int conveyor_id, int destination_box, int location)
{
	unsigned char payload[BOX_CONNECT_LENGTH] = { 0 };

	payload[+BoxConnectPayloadIndex::BOX_ID] = destination_box;
	payload[+BoxConnectPayloadIndex::LOCATION] = location;

	ANTMessage msg = ANTMessage(
		BOX_CONNECT_ID,
		conveyor_id,
		payload,
		BOX_CONNECT_LENGTH
	);

	enqueue_message(msg);

}

void ANTThreadMessageHandler::send_package_to_input(int box_id, int package_id, PackageType pkg)
{
	unsigned char payload[ADD_PACKAGE_LENGTH] = { 0 };

	payload[+AddPackagePayloadIndex::BOX_ID] = box_id;
	payload[+AddPackagePayloadIndex::PACKAGE_ID] = package_id;
	payload[+AddPackagePayloadIndex::PACKAGE_TYPE] = static_cast<int>(pkg);

	ANTMessage msg = ANTMessage(
		ADD_PACKAGE_ID,
		INPUT_CONVEYOR_ID,
		payload,
		ADD_PACKAGE_LENGTH
	);

	enqueue_message(msg);
}

void ANTThreadMessageHandler::send_remove_package_msg(int conveyor_id, int package_id, int box_id)
{
	unsigned char payload[REMOVE_PACKAGE_LENGTH] = { 0 };

	payload[+RemovePackagePayloadIndex::BOX_ID] = box_id;
	payload[+RemovePackagePayloadIndex::PACKAGE_ID] = package_id;

	ANTMessage msg = ANTMessage(
		REMOVE_PACKAGE_ID,
		conveyor_id,
		payload,
		REMOVE_PACKAGE_LENGTH
	);

	enqueue_message(msg);
}

void ANTThreadMessageHandler::send_clear_box_msg(int conveyor_id, int box_id)
{
	unsigned char payload[CLEAR_BOX_LENGTH] = { 0 };

	payload[+ClearBoxPayloadIndex::BOX_ID] = box_id;

	ANTMessage msg = ANTMessage(
		CLEAR_BOX_ID,
		conveyor_id,
		payload,
		CLEAR_BOX_LENGTH
	);

	enqueue_message(msg);
}

void ANTThreadMessageHandler::send_routing_plan_msg(int conveyor_id, int box_id, int target_conveyor)
{
	unsigned char payload[ROUTING_PLAN_LENGTH] = { 0 };

	payload[+RoutingPlanPayloadIndex::BOX_ID] = box_id;
	payload[+RoutingPlanPayloadIndex::TARGET_CONVEYOR_ID] = target_conveyor;


	ANTMessage msg = ANTMessage(
		ROUTING_PLAN_ID,
		conveyor_id,
		payload,
		ROUTING_PLAN_LENGTH
	);

	enqueue_message(msg);
}

void ANTThreadMessageHandler::handle_clear_box_response_msg(ANTMessage& msg)
{
	int box_id = msg.get_data()[+ClearBoxResponsePayloadIndex::BOX_ID];
	int num_packages = msg.get_data()[+ClearBoxResponsePayloadIndex::NUM_PACKAGES];

	std::set<int> packages;

	for (int i = CLEAR_BOX_RESPONSE_STATIC_LENGTH; i < CLEAR_BOX_RESPONSE_STATIC_LENGTH + num_packages; i++) {
		int package_id = static_cast<int>(msg.get_data()[i]);
		packages.insert(package_id);
	}

	conveyor_system->clear_box_completed(box_id, packages);
}

void ANTThreadMessageHandler::handle_package_arrived_msg(ANTMessage& msg)
{
	int box_id = msg.get_data()[+PackageArrivedIndex::BOX_ID];
	int package_id = msg.get_data()[+PackageArrivedIndex::PACKAGE_ID];

	conveyor_system->package_received(package_id, box_id);
}
