/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A simple class for callbacks
*****************************************************************************/
#ifndef _USG_CALLBACK_H_
#define _USG_CALLBACK_H_

namespace usg {

template <class Class, typename ReturnType, typename Parameter = void>
class Callback
{
public:
	typedef ReturnType (Class::*Method)(Parameter);

	Callback(Class* instance, Method method) : m_instance(instance), m_method(method)
	{}

	inline ReturnType const call(Parameter parameter)
	{
		return (m_instance->*m_method)(parameter);
	}

private:
	Class* m_instance;
	Method m_method;
};

template <class Class, typename ReturnType>
class Callback<Class, ReturnType>
{
public:
	typedef ReturnType (Class::*Method)(void);

	Callback(Class* instance, Method method) : m_instance(instance), m_method(method)
	{}

	inline ReturnType const call()
	{
		return (m_instance->*m_method)();
	}

private:
	Class* m_instance;
	Method m_method;
};

}

#endif
