#ifndef __ObjectBase_H__
#define __ObjectBase_H__

#include "Core.h"

#include "Math.h"

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

    void SetWorldPosition(const Vector3& position);
    void SetWorldRotation(const Vector3& rotation);
    void SetWorldScaling(const Vector3& scaling);

protected:

private:
    Vector3 worldPosition_;
    Vector3 worldRotation_;
    Vector3 worldScaling_;

    bool tickable_;
};

#endif