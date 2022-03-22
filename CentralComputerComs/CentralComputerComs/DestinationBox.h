#pragma once

#include <string>

#include "central_computer_types.h"

#define DESTINATION_BOX_SIZE 5

class DestinationBox
{
public:
	DestinationBox();

	PackageType get_package_type() { return package_type; };

	int get_id() { return id; }

	/* Initializes an empty box and readies it to accept packages. */
	void initialize_box(int box_id);

	/* Returns true if the box is initialized */
	bool is_initialized();

	/* Returns true if the destination box can accept packages of the passed type */
	bool can_accept_package(PackageType package);

	/* Returns true if the package was added and false if the package could not be sent to this box */
	bool add_package(PackageType package);

	/* Call this when the physical box sends a message that the package was received */
	void package_received();

	/* 
	* Returns true if the box was emptied successfully and false if the box could not be emptied.
	* The box may have a package in transit if false was returned and the system must wait for that package to arrive before emptying this box.
	*/
	bool empty_box();

	std::string to_string();

private:
	int id;
	PackageType package_type;
	int num_packages_stored;
	int num_packages_in_transit;

};

