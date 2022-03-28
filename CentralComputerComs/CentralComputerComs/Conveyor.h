#pragma once

#include <vector>
#include <string>

#include "DestinationBox.h"
#include "central_computer_types.h"


#define CONVEYOR_MAX_CONNECTIONS 4

class Conveyor
{
public:
	Conveyor(): id(-1), connections() {}

	Conveyor(int id);

	Conveyor(const Conveyor& other);

	int get_id() { return id; }

	DestinationBox& get_box() { return box; }

	std::vector<std::pair<int, ConveyorConnectionType>>& get_connections() { return connections; }

	void add_connection(int connection, ConveyorConnectionType connection_type, int location);

	void add_destination_box(int box_id, int location);

	bool has_destination_box();

	bool has_connection_at_location(int location);

	std::string to_string();

private:
	int id;
	std::vector<std::pair<int, ConveyorConnectionType>> connections;
	DestinationBox box;
};

