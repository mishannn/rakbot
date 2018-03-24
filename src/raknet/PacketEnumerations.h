/// \file
/// \brief All the packet identifiers used by RakNet.  Packet identifiers comprise the first byte of any message.
///
/// This file is part of RakNet Copyright 2003 Kevin Jenkins.
///
/// Usage of RakNet is subject to the appropriate license agreement.
/// Creative Commons Licensees are subject to the
/// license found at
/// http://creativecommons.org/licenses/by-nc/2.5/
/// Single application licensees are subject to the license found at
/// http://www.rakkarsoft.com/SingleApplicationLicense.html
/// Custom license users are subject to the terms therein.
/// GPL license users are subject to the GNU General Public
/// License as published by the Free
/// Software Foundation; either version 2 of the License, or (at your
/// option) any later version.

#ifndef __PACKET_ENUMERATIONS_H
#define __PACKET_ENUMERATIONS_H

/// You should not edit the file PacketEnumerations.h as it is a part of RakNet static library
/// To define your own message id, define an enum following the code example that follows. 
///
/// \code
/// enum {
///   ID_MYPROJECT_MSG_1 = ID_USER_PACKET_ENUM
///   ID_MYPROJECT_MSG_2, 
///    ... 
/// };
/// \endcode 
///
/// \note All these enumerations should be casted to (unsigned char) before writing them to RakNet::BitStream

enum PacketEnumeration {
	ID_INTERNAL_PING = 6,
	ID_PING,
	ID_PING_OPEN_CONNECTIONS,
	ID_CONNECTED_PONG,
	ID_REQUEST_STATIC_DATA,
	ID_CONNECTION_REQUEST,
	ID_AUTH_KEY,
	ID_SECURED_CONNECTION_RESPONSE,
	ID_SECURED_CONNECTION_CONFIRMATION,
	ID_RPC_MAPPING,
	ID_BROADCAST_PINGS = 18,
	ID_SET_RANDOM_NUMBER_SEED,
	ID_RPC,
	ID_RPC_REPLY = 22,
	ID_DETECT_LOST_CONNECTIONS,
	ID_OPEN_CONNECTION_REQUEST,
	ID_OPEN_CONNECTION_REPLY,
	ID_CONNECTION_COOKIE,
	ID_RSA_PUBLIC_KEY_MISMATCH = 28,
	ID_CONNECTION_ATTEMPT_FAILED,
	ID_NEW_INCOMING_CONNECTION,
	ID_NO_FREE_INCOMING_CONNECTIONS,
	ID_DISCONNECTION_NOTIFICATION,
	ID_CONNECTION_LOST,
	ID_CONNECTION_REQUEST_ACCEPTED,
	ID_CONNECTION_BANNED = 36,
	ID_INVALID_PASSWORD,
	ID_MODIFIED_PACKET,
	ID_PONG,
	ID_TIMESTAMP,
	ID_RECEIVED_STATIC_DATA,
	ID_REMOTE_DISCONNECTION_NOTIFICATION,
	ID_REMOTE_CONNECTION_LOST,
	ID_REMOTE_NEW_INCOMING_CONNECTION,
	ID_REMOTE_EXISTING_CONNECTION,
	ID_REMOTE_STATIC_DATA,
	ID_ADVERTISE_SYSTEM = 55,

	ID_VEHICLE_SYNC = 200,
	ID_RCON_COMMAND,
	ID_RCON_RESPONCE,
	ID_AIM_SYNC,
	ID_WEAPONS_UPDATE,
	ID_STATS_UPDATE,
	ID_BULLET_SYNC,
	ID_PLAYER_SYNC,
	ID_MARKERS_SYNC,
	ID_UNOCCUPIED_SYNC,
	ID_TRAILER_SYNC,
	ID_PASSENGER_SYNC,
	ID_SPECTATOR_SYNC
};

#endif

