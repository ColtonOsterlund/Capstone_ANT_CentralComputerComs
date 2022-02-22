#pragma once

/*
* Message class to wrap byte array messages between threads
*/
class ANTMessage
{
public:
	// Length is the length of the data array
	ANTMessage(int id, unsigned char* data, int length);
	~ANTMessage();
	ANTMessage(const ANTMessage& m1);
	unsigned char get_id();
	unsigned char* get_data();

private:
	unsigned char id;
	int length;
	unsigned char* data;
};

