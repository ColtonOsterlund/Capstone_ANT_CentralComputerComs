#include "Conveyor.h"

Conveyor::Conveyor(int id): id(id), connections() {}

Conveyor::Conveyor(const Conveyor& other): id(other.id), connections(other.connections) {}

void Conveyor::add_connection(int connection, ConveyorConnectionType connection_type)
{
	std::pair<int, ConveyorConnectionType> connection_pair (connection, connection_type);
	connections.push_back(connection_pair);
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
		else {
			s.append("SLAVE");
		}
		s.append(",");
	}
	s.append("\n");

	return s;
}
