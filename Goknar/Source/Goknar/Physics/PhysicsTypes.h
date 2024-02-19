#ifndef __PHYSICSDATA_H__
#define __PHYSICSDATA_H__

#include "Core.h"

#include "BulletCollision/CollisionDispatch/btCollisionObject.h"

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

enum class GOKNAR_API CollisionFlag
{
    NONE = -1,
    DYNAMIC_OBJECT = btCollisionObject::CF_DYNAMIC_OBJECT,
    STATIC_OBJECT = btCollisionObject::CF_STATIC_OBJECT,
    KINEMATIC_OBJECT = btCollisionObject::CF_KINEMATIC_OBJECT,
    NO_CONTACT_RESPONSE = btCollisionObject::CF_NO_CONTACT_RESPONSE,
    CUSTOM_MATERIAL_CALLBACK = btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK,
    CHARACTER_OBJECT = btCollisionObject::CF_CHARACTER_OBJECT,
    DISABLE_VISUALIZE_OBJECT = btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT,
    DISABLE_SPU_COLLISION_PROCESSING = btCollisionObject::CF_DISABLE_SPU_COLLISION_PROCESSING,
    HAS_CONTACT_STIFFNESS_DAMPING = btCollisionObject::CF_HAS_CONTACT_STIFFNESS_DAMPING,
    HAS_CUSTOM_DEBUG_RENDERING_COLOR = btCollisionObject::CF_HAS_CUSTOM_DEBUG_RENDERING_COLOR,
    HAS_FRICTION_ANCHOR = btCollisionObject::CF_HAS_FRICTION_ANCHOR,
    HAS_COLLISION_SOUND_TRIGGER = btCollisionObject::CF_HAS_COLLISION_SOUND_TRIGGER,
};

#endif