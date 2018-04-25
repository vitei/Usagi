/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_PARTICLES_SCRIPT_SPARK_EMITTER_H_
#define _USG_PARTICLES_SCRIPT_SPARK_EMITTER_H_
#include "Engine/Common/Common.h"
#include "Engine/Particles/Scripted/ScriptEmitter.h"

namespace usg{

	class ParticleMgr;

	class ScriptSparkEmitter : public usg::ScriptEmitter
	{
		typedef usg::ScriptEmitter Inherited;
	public:
		ScriptSparkEmitter();
		virtual ~ScriptSparkEmitter();

		virtual bool RequiresCPUUpdate() const { return true; }
		void FillOutConstantBuffer();

	protected:
		virtual float InitParticleData(void* pData, void* pMetaData, float fLerp);
		virtual void UpdateParticleCPUDataInt(void* pGPUData, void* pMetaData, float fElapsed);
	private:
		PRIVATIZE_COPY(ScriptSparkEmitter)

	
	};


}

#endif // _USG_PARTICLES_SCRIPT_SPARK_EMITTER_H_
