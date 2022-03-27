#pragma once

#include <string>

/*
* Message class to wrap byte array messages between threads
*/
class ANTMessage
{
public:
	// Length is the length of the data array
	ANTMessage(int id, int conveyor_id, unsigned char* data, int length);
	ANTMessage(int id, unsigned char* data, int length);
	ANTMessage(int id, int conveyor_id);
	~ANTMessage();
	ANTMessage(const ANTMessage& m1);
	int get_id() { return id; }
	unsigned char* get_data() { return data; }
	int get_length() { return length; }
	int get_conveyor_id() { return conveyor_id; };

	std::string to_string();

private:
	int id;
	int length;
	int conveyor_id;
	unsigned char* data;
};

