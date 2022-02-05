#pragma once

/*
* Message class to wrap byte array messages between threads
*/
class Message
{
public:
	// Length is the length of the data array
	Message(int id, unsigned char* data, int length);
	~Message();
	Message(const Message& m1);
	unsigned char get_id();
	unsigned char* get_data();

private:
	unsigned char id;
	int length;
	unsigned char* data;
};

