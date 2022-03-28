#pragma once

#define EMPTY_CONVEYOR_ID -1

// Used to define where boxes are located
#define BOX_CONVEYOR_ID -2


#define INPUT_CONVEYOR_ID 1


enum class ConveyorConnectionType {
	MASTER = 0,
	SLAVE = 1,
	INVALID
};

enum class ConveyorLocationType {
	NORTH = 0,
	EAST = 1,
	SOUTH = 2,
	WEST = 3,
	INVALID
};

enum class PackageType {
	SMALL = 0,
	MEDIUM = 1,
	LARGE = 2,
	NONE,
	INVALID
};

