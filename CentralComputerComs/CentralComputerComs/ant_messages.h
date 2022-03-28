#pragma once

#define ANT_MSG_HEADER_LENGTH 2
#define ANT_MSG_CONVEYOR_ID_INDEX 0
#define ANT_MSG_ID_INDEX 1

#define ANT_RESPONSE_HEADER_LENGTH 1
#define ANT_RESPONSE_ID_INDEX 0



enum {
	CONVEYOR_STATE_ID = 0,
	CONVEYOR_STATE_RESPONSE_ID = 1,
	PACKAGE_ARRIVED_ID = 2,
	BOX_STATE_ID = 3,
	CONVEYOR_CONNECT_ID = 4,
	BOX_CONNECT_ID = 5,
	DISCONNECT_ID = 6,
	CLEAR_BOX_ID = 7,
	CLEAR_BOX_RESPONSE_ID = 8,
	REMOVE_PACKAGE_ID = 9,
	ADD_PACKAGE_ID = 10,
	ROUTING_PLAN_ID = 11,
	MESSAGE_RECEIVED_ID = 12,
};


#define CONVEYOR_STATE_LENGTH 0 // No payload

#define CONVEYOR_STATE_RESPONSE_LENGTH 2
enum class ConveyorStateResponseIndex {
	CONVEYOR_ID = 0,
	AVAILABLE = 1,
};

#define PACKAGE_ARRIVED_LENGTH 2
#define BOX_STATE_LENGTH 0 // msg not used

#define CONVEYOR_CONNECT_LENGTH 3
enum class ConveyorConnectPayloadIndex {
	TARGET_ID = 0,
	CONNECTION_TYPE = 1,
	LOCATION = 2,
};

#define BOX_CONNECT_LENGTH 2
enum class BoxConnectPayloadIndex {
	BOX_ID = 0,
	LOCATION = 1,
};

#define DISCONNECT_LENGTH 0 // No payload

#define CLEAR_BOX_LENGTH 1
enum class ClearBoxPayloadIndex {
	BOX_ID = 0,
};

#define CLEAR_BOX_RESPONSE_STATIC_LENGTH 2
enum class ClearBoxResponsePayloadIndex {
	BOX_ID = 0,
	NUM_PACKAGES,
};

#define REMOVE_PACKAGE_LENGTH 2
enum class RemovePackagePayloadIndex {
	BOX_ID = 0,
	PACKAGE_ID = 1,
};

#define ADD_PACKAGE_LENGTH 3
enum class AddPackagePayloadIndex {
	BOX_ID = 0,
	PACKAGE_ID = 1,
	PACKAGE_TYPE = 2,
};

#define ROUTING_PLAN_LENGTH 2
enum class RoutingPlanPayloadIndex {
	BOX_ID = 0,
	TARGET_CONVEYOR_ID = 1,
};

#define MESSAGE_RECEIVED_LENGTH 2
enum class MessageReceivedIndex {
	MSG_ID = 0,
	CONVEYOR_ID = 1,
};

/* Used to convert from enum class types into their underlying int type */
template <typename T>
constexpr auto operator+(T e) noexcept
-> std::enable_if_t<std::is_enum<T>::value, std::underlying_type_t<T>>
{
	return static_cast<std::underlying_type_t<T>>(e);
}
