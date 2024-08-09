#ifndef __DYNAMICOBJECTFACTORY_H__
#define __DYNAMICOBJECTFACTORY_H__

#include "Core.h"

#include <unordered_map>
#include <functional>
#include <string>

class ObjectBase;

class DynamicObjectFactory
{
public:
    using CreateFunction = std::function<ObjectBase*()>;

    static DynamicObjectFactory* GetInstance()
    {
        if (instance_ == nullptr)
        {
            instance_ = new DynamicObjectFactory();
        }

        return instance_;
    }

    void RegisterClass(const std::string& className, CreateFunction createFunction)
    {
        objectMap_[className] = createFunction;
    }

    ObjectBase* Create(const std::string& className)
    {
        const auto& objectIterator = objectMap_.find(className);
        if (objectIterator != objectMap_.end())
        {
            return objectIterator->second();
        }

        return nullptr;
    }

    const std::unordered_map<std::string, CreateFunction>& GetObjectMap() const
    {
        return objectMap_;
    }

private:
	DynamicObjectFactory() {}
    static DynamicObjectFactory* instance_;

    std::unordered_map<std::string, CreateFunction> objectMap_;
};

#endif