/*
 *	Game Engine Project
 *	Emre Baris Coskun
 *	2018
 */

#ifndef __ObjectBase_H__
#define __ObjectBase_H__

#include "Core.h"

/*
    Base object class
*/
class GOKNAR_API ObjectBase
{
public:
    ObjectBase();

    virtual void BeginGame()
    {
        
    }

    virtual void Tick(float deltaTime)
    {

    }

    void SetTickable(bool tickable);

protected:

private:

    bool tickable_;
};

#endif