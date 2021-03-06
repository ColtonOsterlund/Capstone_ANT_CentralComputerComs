#pragma once

#include <string>
#include <set>

#include "central_computer_types.h"

#define DESTINATION_BOX_SIZE 4

class DestinationBox
{
public:
	DestinationBox();

	DestinationBox(const DestinationBox& other);

	PackageType get_package_type() { return package_type; };

	int get_id() { return id; }

	std::set<int>& get_stored_packages() { return packages_stored; }

	std::set<int>& get_packages_in_transit() { return packages_in_transit; }

	ConveyorLocationType get_location() { return location; }

	/* Initializes an empty box and readies it to accept packages. */
	void initialize_box(int box_id, int location);

	/* Returns true if the box is initialized */
	bool is_initialized();

	/* Returns true if the destination box can accept packages of the passed type */
	bool can_accept_package(PackageType package);

	/* Returns true if the package was added and false if the package could not be sent to this box */
	bool add_package(int package_id, PackageType package);

	/* Call this when the physical box sends a message that the package was received */
	void package_received(int package_id);

	/*
	* Clears the passed packages from the stored and in transit packages
	*/
	void clear_box(std::set<int>& removed_packages);

	/* 
	* Empties a box completely
	* Returns true if the box was emptied successfully and false if the box could not be emptied.
	*/
	bool empty_box();

	/* Returns true if the package is stored at the destination box else false */
	bool has_package_stored(int package_id);

	/* Returns true if the package is in transit towards the box */
	bool has_package_in_transit(int package_id);

	/* Returns true if the package was successfully removed */
	bool remove_package(int package_id);

	std::string to_string();

private:
	int id;
	ConveyorLocationType location;
	PackageType package_type;
	std::set<int> packages_stored;
	std::set<int> packages_in_transit;

};

