#include "ANTMessage.h"


ANTMessage::ANTMessage(int id, int conveyor_id, unsigned char* message, int length): id(id), conveyor_id(conveyor_id), length(length),  {
	this->data = new unsigned char[length];

	for (int i = 0; i < length; i++) {
		this->data[i] = message[i];
	}
}

ANTMessage::ANTMessage(int id, int conveyor_id): id(id), conveyor_id(conveyor_id), data(nullptr), length(0) {}

ANTMessage::~ANTMessage() {
	if (data != nullptr) {
		delete this->data;
	}
}
ANTMessage::ANTMessage(const ANTMessage& m1)
{
	this->id = m1.id;
	this->length = m1.length;

	if (m1.length == 0) {
		this->data = nullptr;
	}
	else {
		this->data = new unsigned char[length];

		for (int i = 0; i < length; i++) {
			this->data[i] = m1.data[i];
		}
	}
}
