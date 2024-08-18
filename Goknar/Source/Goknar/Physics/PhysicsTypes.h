#ifndef __PHYSICSDATA_H__
#define __PHYSICSDATA_H__

#include "Core.h"

#include "BulletCollision/CollisionDispatch/btCollisionObject.h"

enum class GOKNAR_API CollisionGroup : unsigned int
{
    Default =                       0b1,
    
    WorldStaticBlock =              0b10,
    WorldDynamicBlock =             0b100,
	AllBlock =				        WorldDynamicBlock | WorldStaticBlock,

    WorldDynamicOverlap =           0b1000,
    WorldStaticOverlap =            0b10000,
    AllOverlap =                    WorldDynamicOverlap | WorldStaticOverlap,

    Character =                     0b100000,

    Custom0 =                       0b1000000,
    Custom1 =                       0b10000000,
    Custom2 =                       0b100000000,
    Custom3 =                       0b1000000000,
    Custom4 =                       0b10000000000,
    Custom5 =                       0b100000000000,
    Custom6 =                       0b1000000000000,
    Custom7 =                       0b10000000000000,
    Custom8 =                       0b100000000000000,
    Custom9 =                       0b1000000000000000,

    All = 0xFFFFFF
};  

enum class GOKNAR_API CollisionMask : unsigned int
{
    Default =              		    0b00000001,
    BlockWorldStatic = 			    (unsigned char)(CollisionGroup::WorldStaticBlock),
    BlockWorldDynamic = 		    (unsigned char)(CollisionGroup::WorldDynamicBlock),
    BlockCharacter = 			    (unsigned char)(CollisionGroup::Character),
    BlockAll = 					    BlockWorldDynamic | BlockWorldStatic | BlockCharacter,
    BlockAllExceptCharacter =       BlockWorldDynamic | BlockWorldStatic,

    OverlapWorldDynamic = 		    (unsigned char)(CollisionGroup::WorldDynamicOverlap),
    OverlapWorldStatic = 		    (unsigned char)(CollisionGroup::WorldStaticOverlap),
    OverlapCharacter = 		        (unsigned char)(CollisionGroup::Character),
    OverlapAll = 				    OverlapWorldDynamic | OverlapWorldStatic | OverlapCharacter,
    OverlapAllExceptCharacter =     OverlapWorldDynamic | OverlapWorldStatic,

    BlockAndOverlapAll = 		    Default | BlockAll | OverlapAll,

    Custom0 =                       (unsigned char)(CollisionGroup::Custom0),
    Custom1 =                       (unsigned char)(CollisionGroup::Custom1),
    Custom2 =                       (unsigned char)(CollisionGroup::Custom2),
    Custom3 =                       (unsigned char)(CollisionGroup::Custom3),
    Custom4 =                       (unsigned char)(CollisionGroup::Custom4),
    Custom5 =                       (unsigned char)(CollisionGroup::Custom5),
    Custom6 =                       (unsigned char)(CollisionGroup::Custom6),
    Custom7 =                       (unsigned char)(CollisionGroup::Custom7),
    Custom8 =                       (unsigned char)(CollisionGroup::Custom8),
    Custom9 =                       (unsigned char)(CollisionGroup::Custom9)
};

enum class GOKNAR_API CollisionFlag
{
    None = -1,
    DynamicObject = btCollisionObject::CF_DYNAMIC_OBJECT,
    StaticObject = btCollisionObject::CF_STATIC_OBJECT,
    KinematicObject = btCollisionObject::CF_KINEMATIC_OBJECT,
    NoContactResponse = btCollisionObject::CF_NO_CONTACT_RESPONSE,
    CustomMaterialCallback = btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK,
    CharacterObject = btCollisionObject::CF_CHARACTER_OBJECT,
    DisableVisualizeObject = btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT,
    DisableSpuCollisionProcessing = btCollisionObject::CF_DISABLE_SPU_COLLISION_PROCESSING,
    HasContactStiffnessDamping = btCollisionObject::CF_HAS_CONTACT_STIFFNESS_DAMPING,
    HasCustomDebugRenderingColor = btCollisionObject::CF_HAS_CUSTOM_DEBUG_RENDERING_COLOR,
    HasFrictionAnchor = btCollisionObject::CF_HAS_FRICTION_ANCHOR,
    HasCollisionSoundTrigger = btCollisionObject::CF_HAS_COLLISION_SOUND_TRIGGER,
};

#endif