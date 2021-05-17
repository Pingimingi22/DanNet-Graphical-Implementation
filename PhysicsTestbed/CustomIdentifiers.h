#pragma once
#include "MessageIdentifiers.h"


// ========================== Custom user MessageIdentifiers ========================== //
// Inspired by RakNet, a big part of DanNet is the way in which users make their own packet
// identifiers. User's have to create an enum class with the first enum being initialized to
// MessageIdnetifier::CUSTOM_USER_ENUM. Every subsequent custom identifier will be + 1 as an
// int.
// ==================================================================================== //
enum CustomIdentifier
{
	PLAYER_CREATE = MessageIdentifier::CUSTOM_USER_ENUM,
	PLAYER_MOVE,
};