#pragma once

/*
* Message class to wrap byte array messages between threads
*/
class Message
{
public:
	// Expects messages to be have the first byte be the message id and the following bytes be the message data.
	Message(unsigned char* message, int length);
	~Message();
	Message(const Message& m1);
	unsigned char get_id();
	unsigned char* get_data();

private:
	unsigned char id;
	int length;
	unsigned char* data;
};

