#ifndef MATERIAL_H
#define MATERIAL_H

#include <limits.h>

#include "common.h"

#include "Engine/Graphics/Materials/Material.pb.h"
#include "MaterialDefinition/MaterialDefinitionExporter.h"

namespace exchange {

	class Material {
	public:

		Material() {
			usg::exchange::Material_init(&m_material);
			m_bStatic = false;
			m_bCustomFX = false;
		}
		virtual ~Material() {}


		void SetConstantIndex(uint32 uCombinerStage, uint32 uIndex)
		{
			m_material.constantIndexes[uCombinerStage] = uIndex;
		}

		usg::exchange::Material& pb(void) { return m_material; }
		const usg::exchange::Material& pb(void) const { return m_material; }

		bool IsStatic() const { return m_bStatic; }
		void SetStatic(bool bStatic) { m_bStatic = bStatic;  }

		// For the custom effects
		void InitCustomMaterial(const char* szName);
		
		template <class VariableType>
		void SetVariable(const char* szName, VariableType var, uint32 uIndex = 0)
		{
			for (uint32 i = 0; i < m_materialDef.GetConstantSetCount(); i++)
			{
				m_materialDef.OverrideTyped(i, szName, GetConstantSetData(i), &var, 1, uIndex);
			}
		}

		template <class VariableType>
		bool GetVariable(const char* szName, VariableType* pVar, uint32 uIndex = 0)
		{
			for (uint32 i = 0; i < m_materialDef.GetConstantSetCount(); i++)
			{
				if (m_materialDef.GetVariableTyped(i, GetConstantSetData(i), szName, pVar, uIndex))
					return true;
			}
			return false;
		}

		template <class VariableType>
		void SetVariableArray(const char* szName, VariableType* var, uint32 uCount=1, uint32 uIndex = 0)
		{
			for (uint32 i = 0; i < m_materialDef.GetConstantSetCount(); i++)
			{
				m_materialDef.OverrideTyped(i, szName, GetConstantSetData(i), var, uCount, uIndex);
			}
		}

		MaterialDefinitionExporter& GetCustomFX() { return m_materialDef;  }
		bool IsCustomFX() { return m_bCustomFX;  }
		void SetIsCustomFX(bool bFX) { m_bCustomFX = bFX; }
private:
	void AddConstantSet(const char* szName, uint32 uSize);
	void* GetConstantSetData(uint32 uSet);
	// For the information about the material
	
	MaterialDefinitionExporter	m_materialDef;
	usg::exchange::Material		m_material;

	bool	m_bStatic;
	bool	m_bCustomFX;
};

} // namespace exchange

#endif // MATERIAL_H
