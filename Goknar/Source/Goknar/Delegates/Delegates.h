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

protected:

private:
};

template<typename ReturnType, typename... Arguments>
class GOKNAR_API SinglecastDelegate : public Delegate<ReturnType, Arguments...>
{
public:
    SinglecastDelegate() = default;
    ~SinglecastDelegate() = default;

	template<typename Class>
	void SetCallback(Class* owner, ReturnType (Class::* callback)(Arguments...))
    {
        callback_ = std::bind(callback, owner);
    }

    void RemoveCallback()
    {
    }

    virtual void Broadcast(Arguments... arguments) override
    {
        callback_(arguments...);
    }

protected:

private:
    std::function<ReturnType(Arguments...)>& callback_(Arguments...);
};

template<typename ReturnType, typename... Arguments>
class GOKNAR_API MulticastDelegate : public Delegate<ReturnType, Arguments...>
{
public:
    MulticastDelegate() = default;
    ~MulticastDelegate() = default;

	void AddCallback(const std::function<ReturnType(Arguments...)>& callback)
    {
        callbacks_.push_back(callback);
    }

    void RemoveCallback(const std::function<ReturnType(Arguments...)>& callback)
    {
    }

    virtual void Broadcast(Arguments... arguments) override
    {
        for(const auto& callback : callbacks_)
        {
            callback(arguments...);
        }
    }

protected:

private:
    std::vector< std::function<ReturnType(Arguments...)> > callbacks_;
};

#endif