/*
*	Game Engine Project
*	Emre Baris Coskun
*	2018
*/

#ifndef __OBJECTMANAGER_H__
#define __OBJECTMANAGER_H__

#include <vector>

#include "Core.h"

class ObjectBase;

class GAMEENGINE_API ObjectManager
{
    public:
	ObjectManager();
	~ObjectManager();

	void Init();

    void AddObject(ObjectBase *object)
    {
        objects_.push_back(object);
    }

    const std::vector<ObjectBase *>* GetObjects() const
    {
        return &objects_;
    }

    private:

    std::vector<ObjectBase *> objects_;
};

#endif