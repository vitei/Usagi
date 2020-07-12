/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef NET_PLATFORM_H
#define NET_PLATFORM_H



#include OS_HEADER(.,NetPlatform_ps.h)

namespace usg {

void net_startup();
void net_shutdown();
sint64 net_get_uid();
double net_get_time();
sint16 net_get_endian_value();
sint32 net_get_message_offset();

}

#endif
