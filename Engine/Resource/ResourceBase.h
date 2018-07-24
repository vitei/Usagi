/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef ResourceBase_h__
#define ResourceBase_h__

#include "Engine/Resource/ResourceDictionary.h"

namespace usg
{

	class GFXDevice;

	enum class ResourceType : uint32
	{
		UNDEFINED = 0,
		TEXTURE,
		SHADER,
		EFFECT
	};

	class ResourceBase
	{
	public:
		ResourceBase()
		{
			m_nameHash = 0;
			m_dataHash = 0;
			m_resourceType = ResourceType::UNDEFINED;
			m_bReady = false;
		}
		virtual ~ResourceBase() {}
		NameHash GetNameHash() const { return m_nameHash; }
		DataHash GetDataHash() const { return m_dataHash; }

		// Support for asynchronous loading, coded to match level editor for now
		virtual void CleanUp(GFXDevice* pDevice) {}
		bool IsReady() const { return m_bReady; }
		void SetReady(bool bReady) { m_bReady = bReady; }
		ResourceType GetResourceType() const { return m_resourceType; }

	protected:
		void SetupHash( const char* name )
		{
			m_nameHash = ResourceDictionary::calcNameHash( name );
			m_dataHash = ResourceDictionary::searchDataHashByName( m_nameHash ); // Possibly not found
		}
		NameHash		m_nameHash;
		DataHash		m_dataHash;
		ResourceType	m_resourceType;

		bool		m_bReady;
	};
	
}
#endif // ResourceBase_h__
