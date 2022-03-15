#include "ConveyorSystem.h"

ConveyorSystem::ConveyorSystem(): routing_plan(), conveyors(), boxes() {}

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

		for (auto& conn_id : entry.value()) {
			ConveyorConnectionType conn_type = ConveyorConnectionType::MASTER;

			if (conveyors.find(conn_id) != conveyors.end()) {
				conn_type = ConveyorConnectionType::SLAVE;
			}

			conveyor.add_connection(conn_id, conn_type);
		}

		conveyors[conveyor_id] = conveyor;
	}

	std::cout << "Created new configuration: " << std::endl;
	for (auto& n : conveyors) {
		std::cout << n.second.to_string();
	}

}

void ConveyorSystem::clear_configuration() {
	// TODO clear boxes
	for (auto& n : conveyors) {
		ant_handler->send_conveyor_disconnect_msg(n.second.get_id());
	}

	conveyors.clear();
	boxes.clear();
}