#include "ConveyorSystem.h"

ConveyorSystem::ConveyorSystem(): routing_plan(), conveyors() {}

void ConveyorSystem::set_websocket_message_handler(WebThreadMessageHandler* handler)
{
	websocket_handler = handler;
}

void ConveyorSystem::set_ant_message_handler(ANTThreadMessageHandler* handler)
{
	ant_handler = handler;
}

void ConveyorSystem::set_state(json configuration)
{
	clear_configuration();

	// Key is id of conveyor and items are connections
	for (auto& entry : configuration.items()) {
		int conveyor_id = std::stoi(entry.key());

		Conveyor conveyor = Conveyor(conveyor_id);

		int location = 0;
		for (auto& conn_id : entry.value()) {
			ConveyorConnectionType conn_type = ConveyorConnectionType::MASTER;

			if (conveyors.find(conn_id) != conveyors.end()) {
				conn_type = ConveyorConnectionType::SLAVE;
			}

			conveyor.add_connection(conn_id, conn_type, location);
			location++;
		}

		conveyors[conveyor_id] = conveyor;
	}

	std::cout << "Created new configuration: " << std::endl;
	for (auto& n : conveyors) {
		std::cout << n.second.to_string();
	}

}

void ConveyorSystem::add_destination_box(json ids)
{
	int conveyor_id = ids["conveyor_id"];
	int box_id = ids["box_id"];
	if (conveyors.find(conveyor_id) != conveyors.end()) {
		conveyors.find(conveyor_id)->second.add_destination_box(box_id);
		ant_handler->send_destination_box_connect_msg(conveyor_id, box_id);

		std::cout << "\nAdding destination box..." << std::endl;
		std::cout << conveyors.find(conveyor_id)->second.to_string() << std::endl;
	}
	else {
		std::cout << "ConveyorSystem::add_destination_box: Conveyor does not exist" << std::endl;
	}

}

void ConveyorSystem::clear_configuration() {
	for (auto& n : conveyors) {
		ant_handler->send_conveyor_disconnect_msg(n.second.get_id());
	}

	conveyors.clear();
}