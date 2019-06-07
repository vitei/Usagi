/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef ResourceBase_h__
#define ResourceBase_h__

#include "Engine/Resource/ResourceDictionary.h"

namespace usg
{

	#define PRIVATIZE_RES_COPY(NameOfClass) 	NameOfClass(NameOfClass &rhs) : ResourceBase(ResourceType::UNDEFINED) { ASSERT(false); } \
												NameOfClass& operator=(NameOfClass &rhs) { ASSERT(false); return *this; }

	class GFXDevice;

	enum class ResourceType : uint32
	{
		UNDEFINED = 0,
		TEXTURE,
		SHADER,
		EFFECT,
		COLLISION,
		MODEL,
		FONT,
		CUSTOM_EFFECT,
		SKEL_ANIM,
		PARTICLE_EFFECT,
		PARTICLE_EMITTER,
		PROTOCOL_BUFFER,
		PAK_HEADER
	};

	class ResourceBase
	{
	protected:
		ResourceBase(ResourceType eType)
		{
			m_nameHash = 0;
			m_dataHash = 0;
			m_resourceType = eType;
			m_bReady = false;
		}
	public:
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

		bool		m_bReady;

	private:
		ResourceType	m_resourceType;
	};
	
}
#endif // ResourceBase_h__
