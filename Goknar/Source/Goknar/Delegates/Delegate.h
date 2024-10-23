/*

	Copyright (C) 2017 by Sergey A Kryukov: derived work
	http://www.SAKryukov.org
	http://www.codeproject.com/Members/SAKryukov

	Based on original work by Sergey Ryazanov:
	"The Impossibly Fast C++ Delegates", 18 Jul 2005
	https://www.codeproject.com/articles/11015/the-impossibly-fast-c-delegates

	MIT license:
	http://en.wikipedia.org/wiki/MIT_License

	Original publication: https://www.codeproject.com/Articles/1170503/The-Impossibly-Fast-Cplusplus-Delegates-Fixed

*/

#pragma once
#include "DelegateBase.h"

template <typename T> class Delegate;
template <typename T> class MulticastDelegate;

template<typename RET, typename ...PARAMS>
class Delegate<RET(PARAMS...)> final : private DelegateBase<RET(PARAMS...)> {
public:

	Delegate() = default;

	bool isNull() const { return invocation.stub == nullptr; }
	bool operator ==(void* ptr) const {
		return (ptr == nullptr) && this->isNull();
	} //operator ==
	bool operator !=(void* ptr) const {
		return (ptr != nullptr) || (!this->isNull());
	} //operator !=

	Delegate(const Delegate& another) { another.invocation.Clone(invocation); }

	template <typename LAMBDA>
	Delegate(const LAMBDA& lambda) {
		assign((void*)(&lambda), lambda_stub<LAMBDA>);
	} //Delegate

	Delegate& operator =(const Delegate& another) {
		another.invocation.Clone(invocation);
		return *this;
	} //operator =

	template <typename LAMBDA> // template instantiation is not needed, will be deduced (inferred):
	Delegate& operator =(const LAMBDA& instance) {
		assign((void*)(&instance), lambda_stub<LAMBDA>);
		return *this;
	} //operator =

	bool operator == (const Delegate& another) const { return invocation == another.invocation; }
	bool operator != (const Delegate& another) const { return invocation != another.invocation; }

	bool operator ==(const MulticastDelegate<RET(PARAMS...)>& another) const { return another == (*this); }
	bool operator !=(const MulticastDelegate<RET(PARAMS...)>& another) const { return another != (*this); }

	template <class T, RET(T::*TMethod)(PARAMS...)>
	static Delegate Create(T* instance) {
		return Delegate(instance, method_stub<T, TMethod>);
	} //create

	template <class T, RET(T::*TMethod)(PARAMS...) const>
	static Delegate Create(T const* instance) {
		return Delegate(const_cast<T*>(instance), const_method_stub<T, TMethod>);
	} //create

	template <RET(*TMethod)(PARAMS...)>
	static Delegate Create() {
		return Delegate(nullptr, function_stub<TMethod>);
	} //create

	template <typename LAMBDA>
	static Delegate Create(const LAMBDA & instance) {
		return Delegate((void*)(&instance), lambda_stub<LAMBDA>);
	} //create

	RET operator()(PARAMS... arg) const {
		return (*invocation.stub)(invocation.object, arg...);
	} //operator()

private:

	Delegate(void* anObject, typename DelegateBase<RET(PARAMS...)>::stub_type aStub) {
		invocation.object = anObject;
		invocation.stub = aStub;
	} //Delegate

	void assign(void* anObject, typename DelegateBase<RET(PARAMS...)>::stub_type aStub) {
		this->invocation.object = anObject;
		this->invocation.stub = aStub;
	} //assign

	template <class T, RET(T::*TMethod)(PARAMS...)>
	static RET method_stub(void* this_ptr, PARAMS... params) {
		T* p = static_cast<T*>(this_ptr);
		return (p->*TMethod)(params...);
	} //method_stub

	template <class T, RET(T::*TMethod)(PARAMS...) const>
	static RET const_method_stub(void* this_ptr, PARAMS... params) {
		T* const p = static_cast<T*>(this_ptr);
		return (p->*TMethod)(params...);
	} //const_method_stub

	template <RET(*TMethod)(PARAMS...)>
	static RET function_stub(void* this_ptr, PARAMS... params) {
		return (TMethod)(params...);
	} //function_stub

	template <typename LAMBDA>
	static RET lambda_stub(void* this_ptr, PARAMS... arg) {
		LAMBDA* p = static_cast<LAMBDA*>(this_ptr);
		return (p->operator())(arg...);
	} //lambda_stub

	friend class MulticastDelegate<RET(PARAMS...)>;
	typename DelegateBase<RET(PARAMS...)>::InvocationElement invocation;

}; //class Delegate
