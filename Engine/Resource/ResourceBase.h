/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef ResourceBase_h__
#define ResourceBase_h__

#include "Engine/Resource/ResourceDictionary.h"
#include "PakDecl.h"
#include "Engine/Core/stl/string.h"

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
		MAT_ANIM,
		PARTICLE_EFFECT,
		PARTICLE_EMITTER,
		PROTOCOL_BUFFER,
		PAK_FILE,
		HEIGHTFIELD
	};

	enum class ResourceState : uint8
	{
		REQUESTED = 0,
		CPU_LOADING,
		WAITING_DEPENDENCIES,
		CPU_READY,
		QUEUED_GPU_UPLOAD,
		GPU_UPLOADING,
		READY,
		FAILED,
		CANCELLED,
		UNLOADING
	};

	class ResourceBase
	{
	protected:
		ResourceBase(ResourceType eType)
		{
			m_nameHash = 0;
			//m_dataHash = 0;
			m_resourceType = eType;
			m_eState = ResourceState::REQUESTED;
		}
	public:
		virtual ~ResourceBase() {}
		NameHash GetNameHash() const { return m_nameHash; }
		//DataHash GetDataHash() const { return m_dataHash; }

		// Support for asynchronous loading, coded to match level editor for now
		virtual bool Init(GFXDevice* pDevice, const PakFileDecl::FileInfo* pFileHeader, const class FileDependencies* pDependencies, const void* pData) { ASSERT(false); return false; }
		virtual void Cleanup(GFXDevice* pDevice) {}
		bool IsReady() const { return m_eState == ResourceState::READY; }
		void SetReady(bool bReady) { m_eState = bReady ? ResourceState::READY : ResourceState::REQUESTED; }
		ResourceState GetState() const { return m_eState; }
		void SetState(ResourceState eState) { m_eState = eState; }
		ResourceType GetResourceType() const { return m_resourceType; }

#ifdef DEBUG_BUILD
		virtual uint32 GetSizeInMemory() const { return 0; }
		const usg::string& GetName() const { return m_name; }
#endif

	protected:
		void SetupHash( const char* name )
		{
			m_nameHash = ResourceDictionary::calcNameHash( name );
			//m_dataHash = ResourceDictionary::searchDataHashByName( m_nameHash ); // Possibly not found
#ifdef DEBUG_BUILD
			m_name = name;
#endif
		}

		ResourceState	m_eState;

	private:
		ResourceType	m_resourceType;
		NameHash		m_nameHash;
		//DataHash		m_dataHash;
#ifdef DEBUG_BUILD
		usg::string		m_name;
#endif

	};
	
}
#endif // ResourceBase_h__
