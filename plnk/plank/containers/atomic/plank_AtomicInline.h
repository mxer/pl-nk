/*
 -------------------------------------------------------------------------------
 This file is part of the Plink, Plonk, Plank libraries
  by Martin Robinson
 
 https://github.com/0x4d52/pl-nk/
 
 Copyright University of the West of England, Bristol 2011-16
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


#if PLANK_APPLE && PLANK_X86 && PLANK_32BIT
#include "arch/plank_AtomicInline_Mac_X86_32.h"
#elif PLANK_APPLE && PLANK_X86 && PLANK_64BIT
#include "arch/plank_AtomicInline_Mac_X86_64.h"
#elif PLANK_APPLE && PLANK_PPC && PLANK_32BIT
#include "arch/plank_AtomicInline_Mac_PPC_32.h"
#elif PLANK_APPLE && PLANK_ARM && PLANK_32BIT
#include "arch/plank_AtomicInline_iOS_ARM_32.h"
#elif PLANK_APPLE && PLANK_ARM && PLANK_64BIT
#include "arch/plank_AtomicInline_iOS_ARM_64.h"
#elif PLANK_WIN && PLANK_X86 && PLANK_32BIT
#include "arch/plank_AtomicInline_Win_X86_32.h"
#elif PLANK_WIN && PLANK_X86 && PLANK_64BIT
#include "arch/plank_AtomicInline_Win_X86_64.h"
#elif PLANK_LINUX && PLANK_32BIT
#include "arch/plank_AtomicInline_Linux_32.h"
#elif PLANK_LINUX && PLANK_64BIT
#include "arch/plank_AtomicInline_Linux_64.h"
#elif PLANK_ANDROID && PLANK_X86 && PLANK_32BIT
#include "arch/plank_AtomicInline_Android_X86_32.h"
#elif PLANK_ANDROID && PLANK_X86 && PLANK_64BIT
#include "arch/plank_AtomicInline_Android_X86_64.h"
#elif PLANK_ANDROID && PLANK_ARM && PLANK_32BIT
#include "arch/plank_AtomicInline_Android_ARM_32.h"
#elif PLANK_ANDROID && PLANK_ARM && PLANK_64BIT
#include "arch/plank_AtomicInline_Android_ARM_64.h"
#endif

#if !PLANK_ATOMICS_DEFINED
#include "arch/plank_AtomicInline_Lock.h"
#endif
