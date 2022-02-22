#include "WebSocketMessage.h"

WebSocketMessage::WebSocketMessage(std::string message): data(json::parse(message))
{
	/* Set id and remove it from the json */
	id = data["id"];
	data.erase("id");
}

WebSocketMessage::~WebSocketMessage() {}

WebSocketMessage::WebSocketMessage(const WebSocketMessage& m1): id(m1.id), data(m1.data) {}

unsigned char WebSocketMessage::get_id()
{
	return id;
}

json WebSocketMessage::get_data()
{
	return data;
}

std::string WebSocketMessage::to_string()
{
	data["id"] = id;
	std::string msg_string = data.dump();
	data.erase("id");
	return msg_string;
}
