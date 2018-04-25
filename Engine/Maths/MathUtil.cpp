/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include OS_HEADER(Engine/Core/Timer, TimeTracker.h)
#include <time.h>
#include "MathUtil.h"

namespace usg
{
namespace Math
{

void SeedRand()
{
	srand( (uint32)TimeTracker::GetSystemTime() );
}


sint32 RangedRandomSInt(const sint32 low, const sint32 high)
{
	sint32 sRange = (high-low);
	return sRange == 0? low : (rand()%sRange)+low;
}

uint32 RangedRandomUInt(const uint32 low, const uint32 high)
{
	return (uint32)RangedRandomSInt(sint32(low), sint32(high));
}

uint32 Rand()
{
	return (uint32)rand();
}

float RandSign()
{
	if (Rand() % 2 == 0)
	{
		return 1.0f;
	}

	return -1.0f;
}

uint32 RandMax()
{
	return RAND_MAX;
}

//Random numbers
float RangedRandom(const float low, const float high)
{	
	// If low is bigger a negative number will be returned
	// Basically adds a random percentage of the difference between the high and low values
	// to the low value, resulting in a number lying somewhere inbetween
	return Math::Lerp<float>(low, high, ((float)rand())/((float)RAND_MAX) );	
}

}
}

