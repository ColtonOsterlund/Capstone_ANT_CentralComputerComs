#include "ConveyorSystem.h"

#include <set>
#include <queue>

ConveyorSystem::ConveyorSystem(): conveyors() {}

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

			if (conn_id != EMPTY_CONVEYOR_ID) {
				conveyor.add_connection(conn_id, conn_type, location);
				ant_handler->send_conveyor_connect_msg(conveyor_id, conn_id, conn_type, ConveyorLocationType(location));
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
	int conveyor_id = ids["conveyor_id"];
	int box_id = ids["box_id"];
	int location = ids["box_location"];
	std::string err;

	if (conveyors.find(conveyor_id) != conveyors.end()) {
		if (conveyors.find(conveyor_id)->second.has_destination_box()) {
			err.append("ConveyorSystem::add_destination_box: Trying to add a box to a conveyor that already has one");

		}
		else if (conveyors.find(conveyor_id)->second.has_connection_at_location(location)) {
			err.append("ConveyorSystem::add_destination_box: Conveyor selected already has a connection at that location");
		}
		else {
			conveyors.find(conveyor_id)->second.add_destination_box(box_id, location);
			ant_handler->send_destination_box_connect_msg(conveyor_id, box_id, location);

			std::cout << "\nAdding destination box..." << std::endl;
			std::cout << conveyors.find(conveyor_id)->second.to_string() << std::endl;

			create_routing_plan(box_id, conveyor_id);
		}

	}
	else {
		err.append("ConveyorSystem::add_destination_box: Conveyor does not exist");
	}

	if (!err.empty()) {
		websocket_handler->send_add_box_response(box_id, conveyor_id, false, err);
	}
	else {
		websocket_handler->send_add_box_response(box_id, conveyor_id, true, "");
	}

}

void ConveyorSystem::get_destination_box_state(int box_id)
{
	if (box_available(box_id)) {
		DestinationBox& box = find_box(box_id);
		websocket_handler->send_destination_box_state(box.get_id(), static_cast<int>(box.get_package_type()), box.get_packages_in_transit(), box.get_stored_packages());
	}
	else {
		std::cout << "ConveyorSystem: Trying to get state of box that does not exist" << std::endl;
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
				box_id = conveyor.get_box().get_id();

				ant_handler->send_package_to_input(box_id, pkg_id, pkg_type);
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
	if (box_available(box_id)) {
		find_box(box_id).package_received(package_id);
		websocket_handler->send_package_received_confirmation(package_id, box_id);
	}
	else {
		std::cout << "ConveyorSystem: Package received by box that does not exist in processing thread" << std::endl;
	}
}

void ConveyorSystem::remove_package(json ids)
{
	int box_id = ids["box_id"];
	int package_id = ids["package_id"];

	if (box_available(box_id)) {
		DestinationBox& box = find_box(box_id);

		if (box.has_package_stored(package_id)) {
			if (find_box(box_id).remove_package(package_id)) {
				int conveyor_id = get_conveyor_from_box(box_id).get_id();
				ant_handler->send_remove_package_msg(conveyor_id, package_id, box_id);
				websocket_handler->send_remove_package_success(package_id, box_id);
			}
			else {
				// idk how it would ever get here without getting to the other errors
				websocket_handler->send_remove_package_failure(package_id, box_id, "Could not remove package.");
			}

		} else if (box.has_package_in_transit(package_id)) {
			websocket_handler->send_remove_package_failure(package_id, box_id, "Could not remove package. Package is in transit towards box.");
		} else {
			websocket_handler->send_remove_package_failure(package_id, box_id, "Box does not have package in storage or in transit.");

		}
	}
	else {
		websocket_handler->send_remove_package_failure(package_id, box_id, "This box does not exist");
	}
}

void ConveyorSystem::clear_box(int box_id)
{
	if (box_available(box_id)) {
		int conveyor_id = get_conveyor_from_box(box_id).get_id();
		ant_handler->send_clear_box_msg(conveyor_id, box_id);
	}
	else {
		websocket_handler->send_clear_box_fail(box_id, "Box not found");
	}
}

void ConveyorSystem::clear_box_completed(int box_id, std::set<int>& packages_removed)
{
	websocket_handler->send_clear_box_success(box_id, packages_removed);
	find_box(box_id).clear_box(packages_removed);
}

void ConveyorSystem::clear_configuration() {
	for (auto& n : conveyors) {
		ant_handler->send_conveyor_disconnect_msg(n.second.get_id());
	}

	conveyors.clear();
}

bool ConveyorSystem::box_available(int box_id)
{
	// Invalid box number
	if (box_id == -1) {
		return false;
	}

	bool box_is_avail = false;
	for (auto& pair : conveyors) {
		Conveyor& conveyor = pair.second;
		if (conveyor.has_destination_box() && conveyor.get_box().get_id() == box_id) {
			box_is_avail = true;
			break;
		}
	}
	return box_is_avail;
}

DestinationBox& ConveyorSystem::find_box(int box_id)
{
	for (auto& pair : conveyors) {
		Conveyor& conveyor = pair.second;
		if (conveyor.has_destination_box() && conveyor.get_box().get_id() == box_id) {
			DestinationBox& box = conveyor.get_box();
			return box;
		}
	}

	std::cout << "ConveyorSystem: Could not find box with matching id... Returning invalid box" << std::endl;

	// Return something
	return conveyors.at(1).get_box();
}

Conveyor& ConveyorSystem::get_conveyor_from_box(int box_id) {

	for (auto& pair : conveyors) {
		Conveyor& conveyor = pair.second;
		if (conveyor.has_destination_box() && conveyor.get_box().get_id() == box_id) {
			return conveyor;
		}
	}

	std::cout << "ConveyorSystem: Could not find matching conveyor to passed box id" << std::endl;
	// Returning something
	return conveyors.at(INPUT_CONVEYOR_ID);
}

std::vector<int> ConveyorSystem::calculate_routing_plan(int target_conveyor)
{
	std::set<int> visited = std::set<int>();
	std::queue<std::pair<int, std::vector<int>>> node_queue;

	std::vector<int> starting_path;
	starting_path.push_back(INPUT_CONVEYOR_ID);
	std::pair<int, std::vector<int>> starting_pair(INPUT_CONVEYOR_ID, starting_path);
	node_queue.push(starting_pair);

	while (!node_queue.empty()) {
		std::pair<int, std::vector<int>> current_pair = node_queue.front();
		node_queue.pop();
		
		// Check if we have already visited the current node (If two nodes added this one we choose the first path that was already processed)
		if (visited.find(current_pair.first) != visited.end()) {
			continue;
		}

		visited.insert(current_pair.first);

		// Edge case where the input conveyor is also the target
		if (current_pair.first == target_conveyor) {
			return current_pair.second;
		}

		for (auto& conn_pair : conveyors.at(current_pair.first).get_connections()) {
			int conn_id = conn_pair.first;

			// Only process connections that are not invalid and have not been visited yet
			if (conn_id < VALID_CONVEYOR_ID_MIN || visited.find(conn_id) != visited.end()) {
				continue;
			}

			// Copy the current path
			std::vector<int> new_path(current_pair.second);
			new_path.push_back(conn_id);

			// Return if we have reached the target
			if (conn_id == target_conveyor) {
				return new_path;
			}

			std::pair<int, std::vector<int>> new_pair(conn_id, new_path);
			node_queue.push(new_pair);

		}

	}

	return std::vector<int>();
}

void ConveyorSystem::create_routing_plan(int box_id, int target_conveyor)
{
	std::vector<int> path = calculate_routing_plan(target_conveyor);
	if (path.empty()) {
		std::cout << "\nThe path calculated was empty" << std::endl;
	}

	int prev_conv_id = INPUT_CONVEYOR_ID;
	for (auto& current_id : path) {
		if (prev_conv_id == current_id) {
			continue;
		}
		ant_handler->send_routing_plan_msg(prev_conv_id, box_id, current_id);

		prev_conv_id = current_id;
	}
	ant_handler->send_routing_plan_msg(path.back(), box_id, path.back());
}
