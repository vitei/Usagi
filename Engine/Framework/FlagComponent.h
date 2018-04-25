/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
//
//  FlagComponent.h
//  Usagi_xcode
//
//  Created by Giles on 3/12/14.
//  Copyright (c) 2014 Vitei. All rights reserved.
//

#ifndef Usagi_xcode_FlagComponent_h
#define Usagi_xcode_FlagComponent_h

#include "Engine/Common/Common.h"

struct FlagComponent
{
	void	SetFlags(uint32 f)		{ flags |= f; }
	void	ClearFlags(uint32 f)	{ flags &= ~f; }
	uint32	GetFlags() const		{ return flags; }
	
	
	
	uint32	flags;
};


#endif
