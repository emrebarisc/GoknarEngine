#ifndef __DYNAMICOBJECTFACTORY_H__
#define __DYNAMICOBJECTFACTORY_H__

#include "Core.h"

#include <unordered_map>
#include <functional>
#include <string>
#include <vector>

class Component;
class ObjectBase;

enum class DynamicComponentOwnerRequirement : unsigned char
{
    ObjectBase,
    PhysicsObject,
    OverlappingPhysicsObject
};

class DynamicObjectFactory
{
public:
    using CreateFunction = std::function<ObjectBase*()>;
    using IsObjectFunction = std::function<bool(const ObjectBase*)>;
    using CreateComponentFunction = std::function<Component*(Component*)>;
    using IsComponentFunction = std::function<bool(const Component*)>;

    struct ComponentClassInfo
    {
        CreateComponentFunction createFunction;
        IsComponentFunction isInstanceFunction;
        DynamicComponentOwnerRequirement ownerRequirement{ DynamicComponentOwnerRequirement::ObjectBase };
        bool isEditorCreatable{ true };
    };

    static DynamicObjectFactory* GetInstance()
    {
        if (instance_ == nullptr)
        {
            instance_ = new DynamicObjectFactory();
            RegisterBuiltInClasses();
        }

        return instance_;
    }

    void RegisterClass(const std::string& className, CreateFunction createFunction)
    {
        RegisterClass(className, createFunction, nullptr);
    }

    void RegisterClass(const std::string& className, CreateFunction createFunction, IsObjectFunction isObjectFunction)
    {
        objectMap_[className] = createFunction;
        objectTypeChecks_.push_back({ className, isObjectFunction });
    }

    template <class T>
    void RegisterObjectClass(const std::string& className)
    {
        RegisterClass(
            className,
            []() -> ObjectBase*
            {
                return new T();
            },
            [](const ObjectBase* object) -> bool
            {
                return dynamic_cast<const T*>(object) != nullptr;
            });
    }

    void RegisterComponentClass(
        const std::string& className,
        CreateComponentFunction createComponentFunction,
        IsComponentFunction isComponentFunction,
        DynamicComponentOwnerRequirement ownerRequirement = DynamicComponentOwnerRequirement::ObjectBase,
        bool isEditorCreatable = true)
    {
        componentMap_[className] =
        {
            createComponentFunction,
            isComponentFunction,
            ownerRequirement,
            isEditorCreatable
        };
        componentTypeChecks_.push_back({ className, isComponentFunction });
    }

    template <class T>
    void RegisterComponentClass(
        const std::string& className,
        DynamicComponentOwnerRequirement ownerRequirement = DynamicComponentOwnerRequirement::ObjectBase,
        bool isEditorCreatable = true)
    {
        RegisterComponentClass(
            className,
            [](Component* parentComponent) -> Component*
            {
                return new T(parentComponent);
            },
            [](const Component* component) -> bool
            {
                return dynamic_cast<const T*>(component) != nullptr;
            },
            ownerRequirement,
            isEditorCreatable);
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

    Component* CreateComponent(const std::string& className, Component* parentComponent)
    {
        const auto& componentIterator = componentMap_.find(className);
        if (componentIterator != componentMap_.end())
        {
            return componentIterator->second.createFunction(parentComponent);
        }

        return nullptr;
    }

    const std::unordered_map<std::string, CreateFunction>& GetObjectMap() const
    {
        return objectMap_;
    }

    const std::unordered_map<std::string, ComponentClassInfo>& GetComponentMap() const
    {
        return componentMap_;
    }

    std::string GetRegisteredObjectClassName(const ObjectBase* object) const
    {
        for (auto typeCheckIterator = objectTypeChecks_.rbegin(); typeCheckIterator != objectTypeChecks_.rend(); ++typeCheckIterator)
        {
            if (typeCheckIterator->second && typeCheckIterator->second(object))
            {
                return typeCheckIterator->first;
            }
        }

        return "ObjectBase";
    }

    std::string GetRegisteredComponentClassName(const Component* component) const
    {
        for (auto typeCheckIterator = componentTypeChecks_.rbegin(); typeCheckIterator != componentTypeChecks_.rend(); ++typeCheckIterator)
        {
            if (typeCheckIterator->second && typeCheckIterator->second(component))
            {
                return typeCheckIterator->first;
            }
        }

        return "Component";
    }

private:
	DynamicObjectFactory() {}
    static void RegisterBuiltInClasses();
    static DynamicObjectFactory* instance_;

    std::unordered_map<std::string, CreateFunction> objectMap_;
    std::unordered_map<std::string, ComponentClassInfo> componentMap_;
    std::vector<std::pair<std::string, IsObjectFunction>> objectTypeChecks_;
    std::vector<std::pair<std::string, IsComponentFunction>> componentTypeChecks_;
};

#endif
