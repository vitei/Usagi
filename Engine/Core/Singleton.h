/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Singleton baseclass
*****************************************************************************/
#pragma once

#ifndef USG_CORE_SINGLETON
#define	USG_CORE_SINGLETON


namespace usg {

template <class T>
class Singleton
{
public:
	static T*	Create();
	static T*	Inst();
	// TODO: Register in list for automatic deletion
	static void Cleanup() { if(m_pInstance) { delete m_pInstance; m_pInstance = nullptr; } }

protected:
	Singleton() {}
	~Singleton() { }
	const Singleton& operator=(const Singleton& src);
	static T*		m_pInstance;
};


template <class T>
T* Singleton<T>::m_pInstance = nullptr;

template <class T>
inline T* Singleton<T>::Create()
{
	//MutexLock obtain_lock(m_mutex);
	if ( m_pInstance == nullptr)
	{
		m_pInstance = vnew(ALLOC_OBJECT) T;
	}
	return m_pInstance;
}

template <class T>
inline T* Singleton<T>::Inst()
{
	return m_pInstance;
}

} // namespace usg

#endif // USG_CORE_SINGLETON
