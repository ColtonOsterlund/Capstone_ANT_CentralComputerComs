#pragma once

#include <string>
#include <set>


#include "ProcessingThreadMessageHandler.h"
#include "ConveyorSystem.h"
#include "SendReceiveQueue.h"
#include "WebSocketMessage.h"


class WebThreadMessageHandler: public ProcessingThreadMessageHandler<WebSocketMessage>
{
public:
	WebThreadMessageHandler(SendReceiveQueue<WebSocketMessage>* queues);

	/* Check if there are messages in the queue and handle them. Return true if a message was handled and false otherwise.*/
	bool handle_message() override;

	void set_conveyor_system(ConveyorSystem* system);

	void send_package_add_success(int package_id, int box_id);

	void send_package_add_failure(int package_id, std::string error);

	void send_package_received_confirmation(int package_id, int box_id);

	void send_destination_box_state(int box_id, int package_type, std::set<int> packages_in_transit, std::set<int> packages_stored);

	void send_remove_package_success(int package_id, int box_id);

	void send_remove_package_failure(int package_id, int box_id, std::string error);

	void send_clear_box_fail(int box_id, std::string error);

	void send_clear_box_success(int box_id, std::set<int> packages_stored);


private:
	void send_package_add_response(int package_id, bool success, int box_id, std::string details);

	void send_remove_package_response(int package_id, bool success, int box_id, std::string details);

	void send_clear_box_response(int box_id, bool success, std::set<int> packages_stored, std::string details);

	ConveyorSystem* conveyor_system = NULL;

};

