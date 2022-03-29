#pragma once

#include "ANTServer.h"

// Forward declaration for dependency
class ANTServer;

void Set_backend_ANT_server(ANTServer* server);

void Run_driver(unsigned char device_number);

void Send_message_to_ANT(unsigned char* msg, int length);

void Set_timeout();