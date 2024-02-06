#ifndef __PHYSICSDATA_H__
#define __PHYSICSDATA_H__

#include "Core.h"

enum class GOKNAR_API CollisionGroup : unsigned char
{
    None =              0b00000000,
    WorldDynamic =      0b00000010,
    WorldStatic =       0b00000100,
	All =				WorldDynamic | WorldStatic
};  

enum class GOKNAR_API CollisionMask : unsigned char
{
    None =              		0b00000000,
    BlockWorldDynamic = 		(unsigned char)(CollisionGroup::WorldDynamic),
    BlockWorldStatic = 			(unsigned char)(CollisionGroup::WorldStatic),
    BlockAll = 					BlockWorldDynamic | BlockWorldStatic
};

#endif