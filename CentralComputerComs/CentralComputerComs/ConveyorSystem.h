#pragma once

// Forward declaration to deal with double association
class ConveyorSystem;

#include <map>
#include <vector>

#include "WebThreadMessageHandler.h"
#include "RoutingPlan.h"
#include "ANTThreadMessageHandler.h"

// See note above
class WebThreadMessageHandler;
class ANTThreadMessageHandler;

class ConveyorSystem
{
public:
	ConveyorSystem();

	void set_websocket_message_handler(WebThreadMessageHandler* handler);

	void set_ant_message_handler(ANTThreadMessageHandler* handler);

	void set_state(std::map<int, std::vector<int>> state);

private:
	ANTThreadMessageHandler* ant_handler = NULL;
	WebThreadMessageHandler* websocket_handler = NULL;
	RoutingPlan routing_plan;
};

