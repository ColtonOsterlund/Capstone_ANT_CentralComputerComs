#pragma once
#include <json.hpp>
using json = nlohmann::json;

class WebSocketMessage
{
public:
	// Length is the length of the data array
	WebSocketMessage(std::string message);
	~WebSocketMessage();
	WebSocketMessage(const WebSocketMessage& m1);
	unsigned char get_id();
	json get_data();
	std::string to_string();

private:
	unsigned char id;
	json data;
};

