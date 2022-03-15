#pragma once

#include <vector>
#include <string>

#include "central_computer_types.h"


class Conveyor
{
public:
	Conveyor(): id(-1), connections() {}

	Conveyor(int id);

	Conveyor(const Conveyor& other);

	int get_id() { return id; }

	void add_connection(int connection, ConveyorConnectionType connection_type);

	std::string to_string();

private:
	int id;
	std::vector<std::pair<int, ConveyorConnectionType>> connections;

};

