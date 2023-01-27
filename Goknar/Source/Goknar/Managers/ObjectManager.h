#ifndef __OBJECTMANAGER_H__
#define __OBJECTMANAGER_H__

#include <vector>

#include "Goknar/Core.h"

class ObjectBase;

class GOKNAR_API ObjectManager
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