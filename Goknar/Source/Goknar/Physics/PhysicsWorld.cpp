#include "pch.h"

#include "Goknar/Physics/PhysicsWorld.h"

#include "Goknar/Log.h"
#include "Goknar/Math/Matrix.h"
#include "Goknar/Physics/Contacts/ContactGenerator.h"
#include "Goknar/Physics/ForceGenerators/ForceGenerator.h"
#include "Goknar/Physics/RigidBody.h"

PhysicsWorld::PhysicsWorld(unsigned int maxContacts, unsigned int iterations) :
    contactResolver_(iterations),
    firstBody_(nullptr),
    firstContactGen_(nullptr),
    maxContacts_(maxContacts)
{
    forceRegistry_ = new ForceRegistry();

    contacts_ = new PhysicsContact[maxContacts];

    calculateIterations_ = (iterations == 0);

    collisionData_.contactArray = contacts_;
    collisionData_.friction = 0.9f;
    collisionData_.restitution = 0.1f;
    collisionData_.tolerance = 0.1f;
}

PhysicsWorld::~PhysicsWorld()
{
    delete[] contacts_;
    delete forceRegistry_;

    std::vector<CollisionPrimitive*>::iterator collisionPrimitiveIterator = collisions_.begin();
    while (collisionPrimitiveIterator != collisions_.end())
    {
        delete (*collisionPrimitiveIterator);

        ++collisionPrimitiveIterator;
    }
}

CollisionPlane plane;
void PhysicsWorld::PhysicsTick(float deltaTime)
{
    remainingPhysicsTickDeltaTime_ += deltaTime;
    while (PHYSICS_TICK_DELTA_TIME <= remainingPhysicsTickDeltaTime_)
    {
        forceRegistry_->UpdateForces(PHYSICS_TICK_DELTA_TIME);

        //for (auto collision : collisions_)
        //{
        //    collision->CalculateInternals();
        //}

        RigidBodies::iterator rigidBodyIterator = rigidBodies_.begin();
        while (rigidBodyIterator != rigidBodies_.end())
        {
            (*rigidBodyIterator)->PhysicsTick(PHYSICS_TICK_DELTA_TIME);

            ++rigidBodyIterator;
        }

        {
            // Set up the collision data structure
            collisionData_.Reset(maxContacts_);

            int collisionCount = collisions_.size();
            for (int i = 0; i < collisionCount; ++i)
            {
                plane.direction = Vector3(0, 0, 1);
                plane.offset = 0;
                //if (CollisionDetector::BoxAndHalfSpace(*static_cast<CollisionBox*>(collisions_[i]), plane, &collisionData_))
                //{
                //    GOKNAR_INFO("COLLISION!");
                //}

                CollisionPrimitive* primitiveOne = collisions_[i];
                CollisionBox* collisionBoxOne = dynamic_cast<CollisionBox*>(primitiveOne);
                CollisionSphere* collisionSphereOne = dynamic_cast<CollisionSphere*>(primitiveOne);

                for (int j = i + 1; j < collisionCount; ++j)
                {
                    if (!collisionData_.HasMoreContacts())
                    {
                        break;
                    }

                    CollisionPrimitive* primitiveTwo = collisions_[j];
                    CollisionBox* collisionBoxTwo = dynamic_cast<CollisionBox*>(primitiveTwo);
                    CollisionSphere* collisionSphereTwo = dynamic_cast<CollisionSphere*>(primitiveTwo);

                    if (collisionBoxOne && collisionBoxTwo)
                    {
                        CollisionDetector::BoxAndBox(*collisionBoxOne, *collisionBoxTwo, &collisionData_);
                    }
                    else if (collisionBoxOne && collisionSphereTwo)
                    {
                        CollisionDetector::BoxAndSphere(*collisionBoxOne, *collisionSphereTwo, &collisionData_);
                    }
                    else if (collisionBoxTwo && collisionSphereOne)
                    {
                        CollisionDetector::BoxAndSphere(*collisionBoxTwo, *collisionSphereOne, &collisionData_);
                    }
                    else if (collisionSphereOne && collisionSphereTwo)
                    {
                        CollisionDetector::SphereAndSphere(*collisionSphereOne, *collisionSphereTwo, &collisionData_);
                    }
                }
                if (!collisionData_.HasMoreContacts()) break;
            }
        }

        //if (0 < collisionData_.contactCount)
        //{
        //    GOKNAR_WARN("count: {}", collisionData_.contactCount);
        //    for (int collisionIndex = 0; collisionIndex < collisionData_.contactCount; ++collisionIndex)
        //    {
        //        if (0.f < collisionData_.contactArray[collisionIndex].contactVelocity.SquareLength())
        //        {
        //            GOKNAR_INFO("\velocity: {}", collisionData_.contactArray[collisionIndex].contactVelocity.ToString());
        //            GOKNAR_INFO("\tposition: {} normal: {}", collisionData_.contactArray[collisionIndex].contactPoint.ToString(), collisionData_.contactArray[collisionIndex].contactNormal.ToString());
        //        }
        //    }
        //}

        static unsigned int collisionCount = collisionData_.contactCount;

        //if (collisionCount != collisionData_.contactCount)
        //{
        //    GOKNAR_INFO("Collision count is changed!");
        //}

        // Resolve detected contacts
        if (calculateIterations_)
        {
            contactResolver_.SetIterations(collisionData_.contactCount * 4);
        }
        contactResolver_.ResolveContacts(collisionData_.contactArray, collisionData_.contactCount, PHYSICS_TICK_DELTA_TIME);

        collisionCount = collisionData_.contactCount;

        //// Generate contacts
        //unsigned int usedContacts = GenerateContacts();

        //// And process them
        //if (calculateIterations_)
        //{
        //    contactResolver_.SetIterations(usedContacts * 4);
        //}
        //contactResolver_.ResolveContacts(contacts_, usedContacts, deltaTime);

        remainingPhysicsTickDeltaTime_ -= PHYSICS_TICK_DELTA_TIME;
    }
}

void PhysicsWorld::AddRigidBody(RigidBody* rigidBody)
{
    rigidBodies_.push_back(rigidBody);
}

void PhysicsWorld::RemoveRigidBody(RigidBody* rigidBody)
{
    RigidBodies::iterator rigidBodyIterator = rigidBodies_.begin();
    while (rigidBodyIterator != rigidBodies_.end())
    {
        if (*rigidBodyIterator == rigidBody)
        {
            rigidBodies_.erase(rigidBodyIterator);
            break;
        }

        ++rigidBodyIterator;
    }
}

void PhysicsWorld::AddCollision(CollisionPrimitive* collision)
{
    collisions_.push_back(collision);
}

unsigned int PhysicsWorld::GenerateContacts()
{
    unsigned int limit = maxContacts_;
    PhysicsContact* nextContact = contacts_;

    ContactGenRegistration* reg = firstContactGen_;
    while (reg)
    {
        unsigned used = reg->gen->AddContact(nextContact, limit);
        limit -= used;
        nextContact += used;

        // We've run out of contacts to fill. This means we're missing
        // contacts.
        if (limit <= 0) break;

        reg = reg->next;
    }

    // Return the number of contacts used.
    return maxContacts_ - limit;
}