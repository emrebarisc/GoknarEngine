#ifndef __CONTACTGENERATOR_H__
#define __CONTACTGENERATOR_H__

#include "Core.h"

class PhysicsContact;

class GOKNAR_API ContactGenerator
{
public:
    /**
     * Fills the given contact structure with the generated
     * contact. The contact pointer should point to the first
     * available contact in a contact array, where limit is the
     * maximum number of contacts in the array that can be written
     * to. The method returns the number of contacts that have
     * been written.
     */
    virtual unsigned int AddContact(PhysicsContact* contact, unsigned int limit) const = 0;
};

#endif