/*
 -------------------------------------------------------------------------------
 This file is part of the Plink, Plonk, Plank libraries
  by Martin Robinson
 
 http://code.google.com/p/pl-nk/
 
 Copyright University of the West of England, Bristol 2011-12
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
 * Neither the name of University of the West of England, Bristol nor 
   the names of its contributors may be used to endorse or promote products
   derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
 DISCLAIMED. IN NO EVENT SHALL UNIVERSITY OF THE WEST OF ENGLAND, BRISTOL BE 
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE 
 GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
 OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 
 This software makes use of third party libraries. For more information see:
 doc/license.txt included in the distribution.
 -------------------------------------------------------------------------------
 */

#ifndef PLANK_SIMPLESTACK_H
#define PLANK_SIMPLESTACK_H

#include "plank_SimpleLinkedListElement.h"

PLANK_BEGIN_C_LINKAGE

/** A simple stack.
 
 @defgroup PlankSimpleStackClass Plank SimpleStack class
 @ingroup PlankClasses
 @{
 */

typedef struct PlankSimpleStack* PlankSimpleStackRef; 

typedef PlankResult (*PlankSimpleStackFreeElementDataFunction)(PlankP);

PlankSimpleStackRef pl_SimpleStack_CreateAndInit();
PlankSimpleStackRef pl_SimpleStack_Create();
PlankResult pl_SimpleStack_Init (PlankSimpleStackRef p);
PlankResult pl_SimpleStack_DeInit (PlankSimpleStackRef p);
PlankResult pl_SimpleStack_Destroy (PlankSimpleStackRef p);
PlankResult pl_SimpleStack_Clear (PlankSimpleStackRef p);
PlankResult pl_SimpleStack_SetFreeElementDataFunction (PlankSimpleStackRef p, PlankSimpleStackFreeElementDataFunction freeFunction);
PlankResult pl_SimpleStack_Push (PlankSimpleStackRef p, const PlankSimpleStackElementRef element);
PlankResult pl_SimpleStack_Pop (PlankSimpleStackRef p, PlankSimpleStackElementRef* element);
PlankLL pl_SimpleStack_GetSize (PlankSimpleStackRef p);

/** @} */

PLANK_END_C_LINKAGE

#if !DOXYGEN
typedef struct PlankSimpleStack
{
	PlankSimpleStackElementRef	head;
    PlankLL count;
    PlankSimpleStackFreeElementDataFunction freeFunction;
} PlankSimpleStack;
#endif

#endif // PLANK_SIMPLESTACK_H