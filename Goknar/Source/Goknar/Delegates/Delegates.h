#ifndef __DELEGATES_H__
#define __DELEGATES_H__

#include "Goknar/Core.h"

#include <functional>


template<typename ReturnType, typename... Arguments>
using Callback = std::function<ReturnType(Arguments...)>;

template<typename ReturnType, typename... Arguments>
class GOKNAR_API Delegate
{
public:
    Delegate() = default;
    ~Delegate() = default;

    virtual void Broadcast(Arguments... arguments) = 0;

    void operator()(Arguments... arguments)
    {
        Broadcast(arguments...);
    }

protected:

private:
};

template<typename ReturnType, typename... Arguments>
class GOKNAR_API SinglecastDelegate : public Delegate<ReturnType, Arguments...>
{
public:
    SinglecastDelegate() = default;
    ~SinglecastDelegate() = default;

    template<typename Class, typename... Binders>
	void SetCallback(Class* owner, ReturnType (Class::* callback)(Arguments...), Binders... binders)
    {
        callback_ = std::bind(callback, owner, binders...);
    }

    void RemoveCallback()
    {
        callback_ = nullptr;
    }

    virtual void Broadcast(Arguments... arguments) override
    {
        callback_(arguments...);
    }

protected:

private:
    std::function<ReturnType(Arguments...)>& callback_(Arguments...) { nullptr };
};

template<typename ReturnType, typename... Arguments>
class GOKNAR_API MulticastDelegate : public Delegate<ReturnType, Arguments...>
{
public:
    MulticastDelegate() = default;
    ~MulticastDelegate() = default;

    template<typename Class, typename... Binders>
    void AddCallback(Class* owner, ReturnType(Class::* callback)(Arguments...), Binders... binders)
    {
        callbacks_.push_back(std::move(std::bind(callback, owner, binders...)));
    }

    template<typename Class>
    void ClearCallbacksOf(const Class* owner)
    {
    }

    virtual void Broadcast(Arguments... arguments) override
    {
        for(const std::function<ReturnType(Arguments...)>& callback : callbacks_)
        {
            callback(arguments...);
        }
    }

protected:

private:
    std::vector< std::function<ReturnType(Arguments...)> > callbacks_;
};

#endif