#include "Message.h"


Message::Message(unsigned char* message, int length) {
	this->id = message[0];
	this->length = length;
	this->data = new unsigned char[length];

	// Copy everything except the first byte
	for (int i = 1; i < length; i++) {
		this->data[i] = message[i];
	}
}

Message::~Message() {
	delete this->data;
}
Message::Message(const Message& m1)
{
	this->id = m1.id;
	this->length = m1.length;
	this->data = new unsigned char[length];

	for (int i = 0; i < length; i++) {
		this->data[i] = m1.data[i];
	}
}

unsigned char Message::get_id() { return this->id; }
unsigned char* Message::get_data() { return this->data; }
