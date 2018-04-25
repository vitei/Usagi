/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2016
//	Description: Collection of functions that are useful in AI context.
*****************************************************************************/
#ifndef __USG_AI_COMMON__
#define __USG_AI_COMMON__
#include "Engine/Common/Common.h"
#include "Engine/Framework/Signal.h"
#include "Engine/AI/INavigationContext.h"

namespace usg
{

// The following bit of magic checks whether a system implements
// a particular Signal.  It is taken from the following SO post:
//   stackoverflow.com/questions/257288/it-it-possible-to-write-a-c-template-to-check-for-a-functions-existence
// We will make use of it to define our Signals.
#define HAS_MEM_FUNC(func, name)                                                     \
	template<typename T, typename Signature>                                         \
	struct name {                                                                    \
		typedef char Yes;                                                            \
		typedef long No;                                                             \
		template <typename U, U> struct TypeCheck;                                   \
		template <typename _1> static Yes &Check(TypeCheck<Signature, &_1::func> *); \
		template <typename   > static No  &Check(...);                               \
		static const bool value = (sizeof(Check<T>(0)) == sizeof(Yes));              \
	}

	template<bool C, typename T = void>
	struct enable_if
	{
		typedef T type;
	};
	template<typename T>
	struct enable_if<false, T> {};

	namespace ai
	{

		// Get pointer to NavigationWrapper from any kind of AI context. Returns a nullptr if the context does not provide NavigationWrapper.

		HAS_MEM_FUNC(Navigation, HasNavigation);

		template<typename ContextType>
		inline typename enable_if<!HasNavigation<ContextType, usg::ai::NavigationWrapper&(INavigationContext::*)()>::value, NavigationWrapper*>::type GetNavigation(ContextType& context)
		{
			return NULL;
		}

		template<typename ContextType>
		inline typename enable_if<HasNavigation<ContextType, usg::ai::NavigationWrapper&(INavigationContext::*)()>::value, NavigationWrapper*>::type GetNavigation(ContextType& context)
		{
			NavigationWrapper& navigation = context.Navigation();
			return &navigation;
		}

		// Conversion between 2D and 3D vectors are so common in the AI context, that we define the following convenience functions

		inline Vector2f ToVector2f(const Vector3f& v)
		{
			return Vector2f(v.x, v.z);
		}

		inline Vector3f ToVector3f(const Vector2f& v)
		{
			return Vector3f(v.x, 0, v.y);
		}

		inline uint16 FloatToUint16(float fFloat, float fMin, float fMax)
		{
			const float fUint16Max = (float)(0xffff);
			return (uint16)(fUint16Max*(fFloat - fMin) / (fMax - fMin));
		}

	}

}

#endif
