/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2017
****************************************************************************/
#pragma once

#ifndef _CMPR_H
#define _CMPR_H

namespace usg
{
	template <typename T>
	struct ComponentProperties {
		static constexpr uint32 FastPoolChunkSize = 0; // Zero means we use the default value, defined ComponentEntity.cpp
		static constexpr bool HasOnLoaded = false;
		static constexpr bool HasOnActivate = false;
		static constexpr bool HasOnDeactivate = false;
		static constexpr bool IsPlainOldData = false;
	};
}

#endif //_COMPONENT_H
