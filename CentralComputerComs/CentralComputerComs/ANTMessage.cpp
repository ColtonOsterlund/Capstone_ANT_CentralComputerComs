#include "ANTMessage.h"


ANTMessage::ANTMessage(int id, unsigned char* message, int length) {
	this->id = id;
	this->length = length;
	this->data = new unsigned char[length];

	for (int i = 0; i < length; i++) {
		this->data[i] = message[i];
	}
}

ANTMessage::~ANTMessage() {
	delete this->data;
}
ANTMessage::ANTMessage(const ANTMessage& m1)
{
	this->id = m1.id;
	this->length = m1.length;
	this->data = new unsigned char[length];

	for (int i = 0; i < length; i++) {
		this->data[i] = m1.data[i];
	}
}

unsigned char ANTMessage::get_id() { return this->id; }
unsigned char* ANTMessage::get_data() { return this->data; }
