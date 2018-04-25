#pragma once

namespace usg
{
	template<typename T>
	inline bool CheckSingleExclusion(usg::ComponentGetter getter, usg::Exclusion<T>)
	{
		using ComponentType = typename T::first_type;
		using SearchMaskType = typename T::second_type;
		Optional<ComponentType, SearchMaskType> c;
		getter(c);
		return !c.Exists();
	}

	inline bool CheckExclusions(usg::ComponentGetter e, usg::Exclusion<>)
	{
		return true;
	}

	template<typename T, typename... Args>
	inline bool CheckExclusions(usg::ComponentGetter getter, usg::Exclusion<T, Args...>)
	{
		return CheckSingleExclusion(getter, Exclusion<T>()) && CheckExclusions(getter, Exclusion<Args...>());
	}

}
