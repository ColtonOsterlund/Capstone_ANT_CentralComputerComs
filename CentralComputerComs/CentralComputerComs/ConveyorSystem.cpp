#include "ConveyorSystem.h"

ConveyorSystem::ConveyorSystem(): routing_plan()
{
}

void ConveyorSystem::set_websocket_message_handler(WebThreadMessageHandler* handler)
{
	websocket_handler = handler;
}

void ConveyorSystem::set_ant_message_handler(ANTThreadMessageHandler* handler)
{
	ant_handler = handler;
}


void ConveyorSystem::set_state(std::map<int, std::vector<int>> state)
{
}
