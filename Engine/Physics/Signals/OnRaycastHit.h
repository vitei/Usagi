/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2016
****************************************************************************/
#pragma once

#include "Engine/Framework/Component.h"
#include "Engine/Framework/Signal.h"
#include <algorithm>

namespace usg
{

	struct OnRaycastHitSignal : public Signal
	{
		static constexpr uint32 ID = 0x8912abcd;

		uint32 uSystemId, uRaycastId;
		RaycastHitBase* hits;
		Entity* entities;
		uint32 uNumHits;
		uint8* pWorkBuffer;
		const size_t uWorkBufferSize;

		OnRaycastHitSignal(uint32 uSystemId, uint32 uRaycastId, RaycastHitBase* hits, Entity* entities, uint32 uNumHits, uint8* pWorkBuffer, size_t uWorkBufferSize) :
			Signal(ID),
			uSystemId(uSystemId),
			uRaycastId(uRaycastId),
			hits(hits),
			entities(entities),
			uNumHits(uNumHits),
			pWorkBuffer(pWorkBuffer),
			uWorkBufferSize(uWorkBufferSize)
		{

		}

		SIGNAL_RESPONDER(OnRaycastHit)

		template<typename System>
		struct OnRaycastHitClosure : public SignalClosure
		{
		private:
			using HitType = typename System::RaycastHit;
			using HitResultType = typename System::RaycastResult;
			using RaycastHitInputsType = typename System::RaycastHitInputs;

			static constexpr size_t uBytesPerHit = sizeof(HitType) + sizeof(RaycastHitInputsType);
		public:

			OnRaycastHitSignal* signal;
			OnRaycastHitClosure(OnRaycastHitSignal* sig) : signal(sig) {}

			virtual void operator()(const Entity e, const void* in, void* out)
			{
				if (usg::GetSystemId<System>() != signal->uSystemId)
				{
					return;
				}

				uint32 uFinalHits = 0;
				const size_t uRaycastHitInputsBufferOffset = signal->uWorkBufferSize * sizeof(HitType) / (uBytesPerHit);

				HitType* pHitsBuffer = (HitType*)signal->pWorkBuffer;
				RaycastHitInputsType* pRaycastHitInputsBuffer = (RaycastHitInputsType*)(signal->pWorkBuffer + uRaycastHitInputsBufferOffset);
				const uint32 uMaxNumHits = std::min<uint32>(signal->uNumHits, std::min<uint32>((uint32)(uRaycastHitInputsBufferOffset / sizeof(HitType)), (uint32)(((signal->uWorkBufferSize - uRaycastHitInputsBufferOffset) / sizeof(HitType)))));
				for (uint32 j = 0; j < signal->uNumHits; j++)
				{
					ComponentGetter componentGetter(signal->entities[j]);
					if (System::GetRaycastHitInputs(componentGetter, pRaycastHitInputsBuffer[uFinalHits]))
					{
						auto pNewHit = new(&pHitsBuffer[uFinalHits])HitType(pRaycastHitInputsBuffer[uFinalHits]);
						pNewHit->fDistance = signal->hits[j].fDistance;
						pNewHit->vNormal = signal->hits[j].vNormal;
						pNewHit->vPosition = signal->hits[j].vPosition;
						pNewHit->uMaterialFlags = signal->hits[j].uMaterialFlags;
						pNewHit->uGroup = signal->hits[j].uGroup;
						uFinalHits++;
						if (uFinalHits == uMaxNumHits)
						{
							ASSERT(uMaxNumHits == signal->uNumHits && "Had to drop raycast results because work buffer is too small. Consider increasing the size.");
							break;
						}
					}
				}

				HitResultType result;
				result.uNumHits = uFinalHits;
				result.hits = uFinalHits > 0 ? pHitsBuffer : nullptr;
				result.uRaycastId = signal->uRaycastId;
				System::OnRaycastHit(*(const typename System::Inputs*)in, *(typename System::Outputs*)out, result);
			}
		};
	};

}