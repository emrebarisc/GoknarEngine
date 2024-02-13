#ifndef __PHYSICSDATA_H__
#define __PHYSICSDATA_H__

#include "Core.h"

enum class GOKNAR_API CollisionGroup : unsigned char
{
    None =                          0b00000000,

    WorldDynamicBlock =             0b00000010,
    WorldStaticBlock =              0b00000100,
	AllBlock =				        WorldDynamicBlock | WorldStaticBlock,

    WorldDynamicOverlap =           0b00001000,
    WorldStaticOverlap =            0b00010000,
    AllOverlap =                    WorldDynamicOverlap | WorldStaticOverlap,

    Pawn =                          0b00100000,

	All =				            AllBlock | AllOverlap | Pawn,
};  

enum class GOKNAR_API CollisionMask : unsigned char
{
    None =              		0b00000000,
    BlockWorldDynamic = 		(unsigned char)(CollisionGroup::WorldDynamicBlock),
    BlockWorldStatic = 			(unsigned char)(CollisionGroup::WorldStaticBlock),
    BlockPawn = 				(unsigned char)(CollisionGroup::Pawn),
    BlockAll = 					BlockWorldDynamic | BlockWorldStatic | BlockPawn,

    OverlapWorldDynamic = 		(unsigned char)(CollisionGroup::WorldDynamicOverlap),
    OverlapWorldStatic = 		(unsigned char)(CollisionGroup::WorldStaticOverlap),
    OverlapPawn = 		        (unsigned char)(CollisionGroup::Pawn),
    OverlapAll = 				OverlapWorldDynamic | OverlapWorldStatic | OverlapPawn,
    OverlapAllExceptPawn = 		OverlapWorldDynamic | OverlapWorldStatic,

    BlockAndOverlapAll = 		BlockAll | OverlapAll,
};

#endif