/*
This software is subject to the license described in the License.txt file
included with this software distribution. You may not use this file except in compliance
with this license.

Copyright (c) Dynastream Innovations Inc. 2014
All rights reserved.
*/

#ifndef __APP_EVENT__
#define __APP_EVENT__

#include "nrf_soc.h"
#include "app_util_platform.h"


// Potentially replace the method below with
// do { g_event_flags != event ) while (!(g_event_flags & event))

#define EVENT_SET(event)                    do  \
{                                               \
    CRITICAL_REGION_ENTER();                    \
    g_event_flags|= event;                      \
    CRITICAL_REGION_EXIT();                     \
} while(0)

#define EVENT_CLEAR(event)                  do  \
{                                               \
    CRITICAL_REGION_ENTER();                    \
    g_event_flags&= ~event;                     \
    CRITICAL_REGION_EXIT();                     \
} while(0)


// Application Events
#define EVENT_ANT_STACK                     ((uint32_t)0x00000001)
#define EVENT_TIMEOUT                       ((uint32_t)0x00000002)


#endif

