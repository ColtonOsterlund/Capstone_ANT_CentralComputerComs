#pragma once

enum {
	REQUEST_MSG_ID = 0,
	CONVEYOR_STATE_ID = 1,
	PACKAGE_ARRIVED_ID = 2,
	BOX_STATE_ID = 3,
	CONVEYOR_CONNECT_ID = 4,
	BOX_CONNECT_ID = 5,
	DISCONNECT_ID = 6,
	CLEAR_BOX_ID = 7,
	CLEAR_BOX_RESPONSE_ID = 8,
	REMOVE_PACKAGE_ID = 9,
	ADD_PACKAGE_ID = 10,
};


#define CONVEYOR_STATE_LENGTH 1
#define PACKAGE_ARRIVED_LENGTH 2
#define BOX_STATE_LENGTH 0 // msg not used

#define CONVEYOR_CONNECT_LENGTH 3
enum ConveyorConnectPayloadIndex {
	TARGET_ID = 0,
	CONNECTION_TYPE = 1,
	LOCATION = 2,
};

#define BOX_CONNECT_LENGTH 1
enum BoxConnectPayloadIndex {
	BOX_ID = 0,
};

#define DISCONNECT_LENGTH 0 // No payload

#define CLEAR_BOX_LENGTH 1
enum ClearBoxPayloadIndex {
	BOX_ID = 0,
};

#define REMOVE_PACKAGE_LENGTH 2
enum RemovePackagePayloadIndex {
	BOX_ID = 0,
	PACKAGE_ID = 1,
};

#define ADD_PACKAGE_LENGTH 3
enum AddPackagePayloadIndex {
	BOX_ID = 0,
	PACKAGE_ID = 1,
	PACKAGE_TYPE = 2,
};

