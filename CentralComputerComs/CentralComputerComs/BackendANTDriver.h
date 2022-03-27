#pragma once

#include "ANTServer.h"

// Forward declaration for dependency
class ANTServer;

void Set_backend_ANT_server(ANTServer* server);

void Run_driver(unsigned char device_number);