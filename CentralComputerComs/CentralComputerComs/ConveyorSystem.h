#pragma once

// Forward declaration to deal with double association
class ConveyorSystem;

#include <map>
#include <vector>
#include <json.hpp>

#include "WebThreadMessageHandler.h"
#include "ANTThreadMessageHandler.h"
#include "Conveyor.h"
#include "DestinationBox.h"

// See note above
class WebThreadMessageHandler;
class ANTThreadMessageHandler;

using json = nlohmann::json;


class ConveyorSystem
{
public:
	ConveyorSystem();

	void set_websocket_message_handler(WebThreadMessageHandler* handler);

	void set_ant_message_handler(ANTThreadMessageHandler* handler);

	void set_state(json configuration);

	void add_destination_box(json ids);

	void get_destination_box_state(int box_id);

	void send_package(json pkg);

	void package_received(int package_id, int box_id);

	void remove_package(json ids);

	void clear_box(int box_id);

	void clear_box_completed(int box_id, std::set<int>& packages_removed);

private:
	void clear_configuration();

	bool box_available(int box_id);

	DestinationBox& find_box(int box_id);

	Conveyor& get_conveyor_from_box(int box_id);

	void create_routing_plan(int box_id, int target_conveyor);

	std::vector<int> calculate_routing_plan(int target_conveyor);

	ANTThreadMessageHandler* ant_handler = NULL;
	WebThreadMessageHandler* websocket_handler = NULL;

	std::map<int, Conveyor> conveyors;

};

