#pragma once

// Forward declaration to deal with double association
class ConveyorSystem;

#include <map>
#include <vector>
#include <json.hpp>

#include "WebThreadMessageHandler.h"
#include "RoutingPlan.h"
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


private:
	void clear_configuration();

	ANTThreadMessageHandler* ant_handler = NULL;
	WebThreadMessageHandler* websocket_handler = NULL;
	RoutingPlan routing_plan;

	std::map<int, Conveyor> conveyors;

};

