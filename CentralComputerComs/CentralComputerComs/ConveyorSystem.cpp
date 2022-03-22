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

			if (conn_id != -1) {
				conveyor.add_connection(conn_id, conn_type, location);
			}

			location++;
			if (location > 3) {
				location = 0;
			}
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
	//TODO create routing plan to box and send to ant side

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

void ConveyorSystem::get_destination_box_state(int box_id)
{
	for (auto& pair : conveyors) {
		Conveyor& conveyor = pair.second;
		if (conveyor.has_destination_box() && conveyor.get_box().get_id() == box_id) {
			DestinationBox box = conveyor.get_box();
			websocket_handler->send_destination_box_state(box.get_id(), static_cast<int>(box.get_package_type()), box.get_packages_in_transit(), box.get_stored_packages());
			break;
		}
	}
}

void ConveyorSystem::send_package(json pkg)
{
	int raw_type = pkg["type"];
	PackageType pkg_type = static_cast<PackageType>(raw_type);
	
	int pkg_id = pkg["package_id"];

	bool package_sent = false;
	int box_id = -1;


	for (auto& pair : conveyors) {
		Conveyor& conveyor = pair.second;
		if (conveyor.has_destination_box() && conveyor.get_box().can_accept_package(pkg_type)) {
			if (conveyor.get_box().add_package(pkg_id, pkg_type)) {
				ant_handler->send_package_to_input(pkg_id, pkg_type);

				box_id = conveyor.get_box().get_id();
				package_sent = true;

				std::cout << "Added package:" << std::to_string(pkg_id) << " to destination box : " << conveyor.to_string() << std::endl;

				break;
			}
		}
	}

	if (package_sent) {
		websocket_handler->send_package_add_success(pkg_id, box_id);
	}
	else {
		websocket_handler->send_package_add_failure(pkg_id, "No destination boxes available.");
	}
}

void ConveyorSystem::package_received(int package_id, int box_id)
{
	for (auto& pair : conveyors) {
		Conveyor& conveyor = pair.second;
		if (conveyor.has_destination_box() && conveyor.get_box().get_id() == box_id) {
			conveyor.get_box().package_received(package_id);
			websocket_handler->send_package_received_confirmation(package_id, box_id);
			break;
		}
	}
}

void ConveyorSystem::clear_configuration() {
	for (auto& n : conveyors) {
		ant_handler->send_conveyor_disconnect_msg(n.second.get_id());
	}

	conveyors.clear();
}