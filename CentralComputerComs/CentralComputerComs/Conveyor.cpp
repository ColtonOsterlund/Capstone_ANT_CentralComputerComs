#include "Conveyor.h"

Conveyor::Conveyor(int id): id(id), connections(CONVEYOR_MAX_CONNECTIONS), box() {
	for (int i = 0; i < CONVEYOR_MAX_CONNECTIONS; i++) {
		std::pair<int, ConveyorConnectionType> connection_pair(EMPTY_CONVEYOR_ID, ConveyorConnectionType::INVALID);

		connections.at(i) = connection_pair;
	}
}

Conveyor::Conveyor(const Conveyor& other): id(other.id), connections(other.connections), box(other.box) {}

void Conveyor::add_connection(int connection, ConveyorConnectionType connection_type, int location)
{
	std::pair<int, ConveyorConnectionType> connection_pair (connection, connection_type);
	connections.at(location) = connection_pair;
}

void Conveyor::add_destination_box(int box_id, int location) {
	box.initialize_box(box_id, location);
	std::pair<int, ConveyorConnectionType> box_connection_pair(BOX_CONVEYOR_ID, ConveyorConnectionType::SLAVE);
	connections.at(location) = box_connection_pair;
}

bool Conveyor::has_destination_box() {
	return box.is_initialized();
}

bool Conveyor::has_connection_at_location(int location) {
	if (connections.at(location).first != EMPTY_CONVEYOR_ID) {
		return true;
	}

	return false;
}

std::string Conveyor::to_string()
{
	std::string s;
	s.append(std::to_string(id));
	s.append(" : ");
	
	for (auto& conn : connections) {
		s.append(std::to_string(conn.first));
		s.append("-");
		if (conn.second == ConveyorConnectionType::MASTER) {
			s.append("MASTER");
		}
		else if (conn.second == ConveyorConnectionType::SLAVE) {
			s.append("SLAVE");
		}
		else {
			s.append("INVALID");
		}
		s.append(",");
	}

	if (box.is_initialized()) {
		s.append("Destination Box: ");
		s.append(box.to_string());
	}

	s.append("\n");

	return s;
}
