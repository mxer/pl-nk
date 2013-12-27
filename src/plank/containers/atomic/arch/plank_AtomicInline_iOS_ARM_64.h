/*
 -------------------------------------------------------------------------------
 This file is part of the Plink, Plonk, Plank libraries
  by Martin Robinson
 
 http://code.google.com/p/pl-nk/
 
 Copyright University of the West of England, Bristol 2011-13
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

// help prevent accidental inclusion other than via the intended header
#if PLANK_INLINING_FUNCTIONS

#if PLANK_NOATOMIC64BIT
    #include "../core/plank_ThreadSpinLock.h"
#endif

//------------------------------------------------------------------------------


#if !DOXYGEN
typedef struct PlankAtomicI
{
    volatile PlankI value;
} PlankAtomicI PLANK_ALIGN(4);

typedef struct PlankAtomicL
{
    volatile PlankL value;
} PlankAtomicL PLANK_ALIGN(8);

typedef struct PlankAtomicLL
{
    volatile PlankLL value;
#if PLANK_NOATOMIC64BIT
    PlankSpinLock lock;
#endif
} PlankAtomicLL PLANK_ALIGN(8);

typedef struct PlankAtomicF
{
    volatile PlankF value;
} PlankAtomicF PLANK_ALIGN(4);

typedef struct PlankAtomicD
{
    volatile PlankD value;
#if PLANK_NOATOMIC64BIT
    PlankThreadSpinLock lock;
#endif
} PlankAtomicD PLANK_ALIGN(8);

typedef struct PlankAtomicP
{
    volatile PlankP ptr;
} PlankAtomicP PLANK_ALIGN(8);

typedef struct PlankAtomicPX
{
    volatile PlankP ptr;
    volatile PlankL extra;
#if PLANK_NOATOMIC64BIT
    PlankThreadSpinLock lock;
#endif
} PlankAtomicPX PLANK_ALIGN(16);

typedef struct PlankAtomicLX
{
    volatile PlankL value;
    volatile PlankL extra;
#if PLANK_NOATOMIC64BIT
    PlankThreadSpinLock lock;
#endif
} PlankAtomicLX PLANK_ALIGN(16);
#endif

static inline void pl_AtomicMemoryBarrier()
{
#if PLANK_APPLE
    OSMemoryBarrier();
#elif PLANK_WIN
	_ReadWriteBarrier();
#elif PLANK_ANDROID
    __sync_synchronize();
#else
    #warning pl_AtomicMemoryBarrier() not fully implemented for this platform
#endif
}

#if PLANK_WIN

#if PLANK_32BIT
#pragma warning(disable:4035)
static inline PlankULL pl_InterlockedCompareExchange64 (volatile PlankULL *value, 
                                                        PlankULL newValue, 
                                                        PlankULL oldValue) 
{
    //value returned in eax::edx
    __asm {
        lea esi,oldValue;
        lea edi,newValue;
        
        mov eax,[esi];
        mov edx,4[esi];
        mov ebx,[edi];
        mov ecx,4[edi];
        mov esi,value;
        lock CMPXCHG8B [esi];			
    }
}
#pragma warning(default:4035)
#endif

#if PLANK_64BIT
static inline PlankULL pl_InterlockedCompareExchange64 (volatile PlankULL *value, 
                                                        PlankULL newValue, 
                                                        PlankULL oldValue) 
{
	return (PlankULL)_InterlockedCompareExchange64 ((volatile __int64*)value, 
												    *(__int64*)&newValue, 
												    *(__int64*)&oldValue);
}
#endif

#endif

//------------------------------------------------------------------------------

static inline PlankI pl_AtomicI_Get (PlankAtomicIRef p)
{
    return p->value; // should be aligned anyway and volatile so OK // pl_AtomicI_Add (p, 0);
}

static inline PlankI pl_AtomicI_GetUnchecked (PlankAtomicIRef p)
{
    return p->value;
}

static inline PlankI pl_AtomicI_Swap (PlankAtomicIRef p, PlankI newValue)
{
    PlankI oldValue;
    PlankB success;
    
    do {
        oldValue = *(PlankI*)p;
        success = pl_AtomicI_CompareAndSwap (p, oldValue, newValue);
    } while (!success);
    
    return oldValue;
}

static inline void pl_AtomicI_SwapOther (PlankAtomicIRef p1, PlankAtomicIRef p2)
{
    PlankI value1, value2;
    PlankB success;
    
    do {
        value1 = *(PlankI*)p1;
        value2 = *(PlankI*)p2;
        success = pl_AtomicI_CompareAndSwap (p1, value1, value2);
    } while (!success);
    
    *(PlankI*)p2 = value1;
}

static inline void pl_AtomicI_Set (PlankAtomicIRef p, PlankI newValue)
{
    pl_AtomicI_Swap (p, newValue);
}

#if PLANK_APPLE
static inline PlankI pl_AtomicI_Add (PlankAtomicIRef p, PlankI operand)
{
    return OSAtomicAdd32Barrier (*(int32_t*)&operand, 
                                 (volatile int32_t*)p);
}

static inline PlankB  pl_AtomicI_CompareAndSwap (PlankAtomicIRef p, PlankI oldValue, PlankI newValue)
{    
    return OSAtomicCompareAndSwap32Barrier (*(int32_t*)&oldValue, 
                                            *(int32_t*)&newValue, 
                                            (volatile int32_t*)p);
}
#endif

#if PLANK_WIN
static inline PlankI pl_AtomicI_Add (PlankAtomicIRef p, PlankI operand)
{
    return operand + _InterlockedExchangeAdd ((volatile long*)p, operand);
}

static inline PlankB  pl_AtomicI_CompareAndSwap (PlankAtomicIRef p, PlankI oldValue, PlankI newValue)
{    
    return oldValue == _InterlockedCompareExchange ((volatile long*)p, 
                                                    *(long*)&newValue, 
                                                    *(long*)&oldValue);
}
#endif

#if PLANK_ANDROID
static inline PlankI pl_AtomicI_Add (PlankAtomicIRef p, PlankI operand)
{
    return __sync_add_and_fetch ((volatile PlankI*)p, operand);
}

static inline PlankB  pl_AtomicI_CompareAndSwap (PlankAtomicIRef p, PlankI oldValue, PlankI newValue)
{
    return __sync_bool_compare_and_swap ((volatile PlankI*)p,
                                         oldValue,
                                         newValue);
}
#endif

static inline PlankI pl_AtomicI_Subtract (PlankAtomicIRef p, PlankI operand)
{
    return pl_AtomicI_Add (p, -operand);
}

static inline PlankI pl_AtomicI_Increment (PlankAtomicIRef p)
{
    return pl_AtomicI_Add (p, 1);
}

static inline PlankI pl_AtomicI_Decrement (PlankAtomicIRef p)
{
    return pl_AtomicI_Add (p, -1);
}

//------------------------------------------------------------------------------

static inline PlankL pl_AtomicL_Get (PlankAtomicLRef p)
{
    return p->value; // should be aligned anyway and volatile so OK // pl_AtomicL_Add (p, (PlankL)0);
}

static inline PlankL pl_AtomicL_GetUnchecked (PlankAtomicLRef p)
{
    return p->value;
}

static inline PlankL pl_AtomicL_Swap (PlankAtomicLRef p, PlankL newValue)
{
    PlankL oldValue;
    PlankB success;
    
    do {
        oldValue = *(PlankL*)p;
        success = pl_AtomicL_CompareAndSwap (p, oldValue, newValue);
    } while (!success);
    
    return oldValue;
}

static inline void pl_AtomicL_SwapOther (PlankAtomicLRef p1, PlankAtomicLRef p2)
{
    PlankL value1, value2;
    PlankB success;
    
    do {
        value1 = *(PlankL*)p1;
        value2 = *(PlankL*)p2;
        success = pl_AtomicL_CompareAndSwap (p1, value1, value2);
    } while (!success);
    
    *(PlankL*)p2 = value1;
}

static inline void pl_AtomicL_Set (PlankAtomicLRef p, PlankL newValue)
{
    pl_AtomicL_Swap (p, newValue);
}

#if PLANK_APPLE
#if PLANK_32BIT
static inline PlankL pl_AtomicL_Add (PlankAtomicLRef p, PlankL operand)
{
    return OSAtomicAdd32Barrier (*(int32_t*)&operand, 
                                 (volatile int32_t*)p);
}

static inline PlankB pl_AtomicL_CompareAndSwap (PlankAtomicLRef p, PlankL oldValue, PlankL newValue)
{    
    return OSAtomicCompareAndSwap32Barrier (*(int32_t*)&oldValue, 
                                            *(int32_t*)&newValue, 
                                            (volatile int32_t*)p);
}
#endif //PLANK_32BIT

#if PLANK_64BIT
static inline PlankL pl_AtomicL_Add (PlankAtomicLRef p, PlankL operand)
{
    return OSAtomicAdd64Barrier (*(int64_t*)&operand, 
                                 (volatile int64_t*)p);
}

static inline PlankB  pl_AtomicL_CompareAndSwap (PlankAtomicLRef p, PlankL oldValue, PlankL newValue)
{    
    return OSAtomicCompareAndSwap64Barrier (*(int64_t*)&oldValue, 
                                            *(int64_t*)&newValue, 
                                            (volatile int64_t*)p);
}
#endif //PLANK_64BIT
#endif //PLANK_APPLE

#if PLANK_WIN
#if PLANK_32BIT
static inline PlankL pl_AtomicL_Add (PlankAtomicLRef p, PlankL operand)
{
    return operand + _InterlockedExchangeAdd ((volatile long*)p, operand);
}

static inline PlankB  pl_AtomicL_CompareAndSwap (PlankAtomicLRef p, PlankL oldValue, PlankL newValue)
{    
    return oldValue == _InterlockedCompareExchange ((volatile long*)p, newValue, oldValue);
}
#endif //PLANK_32BIT

#if PLANK_64BIT
static inline PlankL pl_AtomicL_Add (PlankAtomicLRef p, PlankL operand)
{
    PlankL oldValue, newValue;
    PlankB success;
    
    do {
        oldValue = *(PlankL*)p;
        newValue = oldValue + operand;
        success = pl_AtomicL_CompareAndSwap (p, oldValue, newValue);
    } while (!success);
    
    return newValue;    
}

static inline PlankB  pl_AtomicL_CompareAndSwap (PlankAtomicLRef p, PlankL oldValue, PlankL newValue)
{    
    return (*(PlankULL*)&oldValue) == pl_InterlockedCompareExchange64 ((volatile PlankULL*)p,
									  					               *(PlankULL*)&newValue, 
                                                                       *(PlankULL*)&oldValue);
}
#endif //PLANK_64BIT
#endif //PLANK_WIN

#if PLANK_ANDROID
static inline PlankL pl_AtomicL_Add (PlankAtomicLRef p, PlankL operand)
{
    return __sync_add_and_fetch ((volatile PlankL*)p, operand);
}

static inline PlankB pl_AtomicL_CompareAndSwap (PlankAtomicLRef p, PlankL oldValue, PlankL newValue)
{
    return __sync_bool_compare_and_swap ((volatile PlankL*)p,
                                         oldValue,
                                         newValue);
}
#endif

static inline PlankL pl_AtomicL_Subtract (PlankAtomicLRef p, PlankL operand)
{
    return pl_AtomicL_Add (p, -operand);
}

static inline PlankL pl_AtomicL_Increment (PlankAtomicLRef p)
{
    return pl_AtomicL_Add (p, (PlankL)1);
}

static inline PlankL pl_AtomicL_Decrement (PlankAtomicLRef p)
{
    return pl_AtomicL_Add (p, (PlankL)(-1));
}

//------------------------------------------------------------------------------

static inline PlankResult pl_AtomicLL_Init (PlankAtomicLLRef p)
{
    PlankResult result = PlankResult_OK;
    
    if (p == PLANK_NULL)
    {
        result = PlankResult_MemoryError;
        goto exit;
    }
    
#if PLANK_NOATOMIC64BIT
    pl_ThreadSpinLock_Init (&p->lock);
#endif
    
    //    pl_AtomicLL_Set (p, (PlankLL)0);
    p->value = 0;
    
exit:
    return result;
}

static inline PlankResult pl_AtomicLL_DeInit (PlankAtomicLLRef p)
{
    PlankResult result = PlankResult_OK;
    
    if (p == PLANK_NULL)
    {
        result = PlankResult_MemoryError;
        goto exit;
    }
    
#if PLANK_NOATOMIC64BIT
    pl_ThreadSpinLock_DeInit (&p->lock);
#endif
    
exit:
    return result;
}

static inline PlankLL pl_AtomicLL_Get (PlankAtomicLLRef p)
{
#if PLANK_32BIT
    return pl_AtomicLL_Add (p, (PlankLL)0);
#endif
    
#if PLANK_64BIT
    return p->value; // should be aligned anyway and volatile so OK
#endif
}

static inline PlankLL pl_AtomicLL_GetUnchecked (PlankAtomicLLRef p)
{
    return p->value;
}

static inline PlankLL pl_AtomicLL_Swap (PlankAtomicLLRef p, PlankLL newValue)
{
    PlankLL oldValue;
    PlankB success;
    
    do {
        oldValue = *(PlankLL*)p;
        success = pl_AtomicLL_CompareAndSwap (p, oldValue, newValue);
    } while (!success);
    
    return oldValue;
}

static inline void pl_AtomicLL_SwapOther (PlankAtomicLLRef p1, PlankAtomicLLRef p2)
{
    PlankLL value1, value2;
    PlankB success;
    
    do {
        value1 = *(PlankLL*)p1;
        value2 = *(PlankLL*)p2;
        success = pl_AtomicLL_CompareAndSwap (p1, value1, value2);
    } while (!success);
    
    *(PlankLL*)p2 = value1;
}

static inline void pl_AtomicLL_Set (PlankAtomicLLRef p, PlankLL newValue)
{
    pl_AtomicLL_Swap (p, newValue);
}

#if PLANK_APPLE
#if PLANK_X86 || PLANK_ARM // and PLANK_APPLE
static inline PlankLL pl_AtomicLL_Add (PlankAtomicLLRef p, PlankLL operand)
{
    return OSAtomicAdd64Barrier (*(int64_t*)&operand, 
                                 (volatile int64_t*)p);
}

static inline PlankB pl_AtomicLL_CompareAndSwap (PlankAtomicLLRef p, PlankLL oldValue, PlankLL newValue)
{    
    return OSAtomicCompareAndSwap64Barrier (*(int64_t*)&oldValue, 
                                            *(int64_t*)&newValue, 
                                            (volatile int64_t*)p);
}
#endif // PLANK_X86 and PLANK_APPLE

#if PLANK_PPC // and PLANK_APPLE
static inline PlankLL pl_AtomicLL_Add (PlankAtomicLLRef p, PlankLL operand)
{
    PlankLL oldValue, newValue;
    PlankB success;
    
    do {
        oldValue = *(PlankLL*)p;
        newValue = oldValue + operand;
        success = pl_AtomicLL_CompareAndSwap (p, oldValue, newValue);
    } while (!success);
    
    return newValue;    
}
#endif // PLANK_PPC and PLANK_APPLE
#endif

#if PLANK_WIN
static inline PlankLL pl_AtomicLL_Add (PlankAtomicLLRef p, PlankLL operand)
{
    PlankLL oldValue, newValue;
    PlankB success;
    
    do {
        oldValue = *(PlankLL*)p;
        newValue = oldValue + operand;
        success = pl_AtomicLL_CompareAndSwap (p, oldValue, newValue);
    } while (!success);
    
    return newValue;    
}

static inline PlankB pl_AtomicLL_CompareAndSwap (PlankAtomicLLRef p, PlankLL oldValue, PlankLL newValue)
{    
    return oldValue == (PlankLL)pl_InterlockedCompareExchange64 ((volatile PlankULL*)p,
							   						             *(PlankULL*)&newValue, 
																 *(PlankULL*)&oldValue);
}
#endif

#if PLANK_ANDROID
static inline PlankLL pl_AtomicLL_Add (PlankAtomicLLRef p, PlankLL operand)
{
    return __sync_add_and_fetch ((volatile PlankLL*)p, operand);
}

static inline PlankB pl_AtomicLL_CompareAndSwap (PlankAtomicLLRef p, PlankLL oldValue, PlankLL newValue)
{
    return __sync_bool_compare_and_swap ((volatile PlankLL*)p,
                                         oldValue,
                                         newValue);
}
#endif

static inline PlankLL pl_AtomicLL_Subtract (PlankAtomicLLRef p, PlankLL operand)
{
    return pl_AtomicLL_Add (p, -operand);
}

static inline PlankLL pl_AtomicLL_Increment (PlankAtomicLLRef p)
{
    return pl_AtomicLL_Add (p, (PlankLL)1);
}

static inline PlankLL pl_AtomicLL_Decrement (PlankAtomicLLRef p)
{
    return pl_AtomicLL_Add (p, (PlankLL)(-1));
}

//------------------------------------------------------------------------------

static inline PlankF pl_AtomicF_Get (PlankAtomicFRef p)
{
//    PlankI bits = pl_AtomicI_Add ((PlankAtomicIRef)p, 0); // use the I version
//    return *(PlankF*)&bits;
    
    return p->value; // should be aligned anyway and volatile so OK
}

static inline PlankF pl_AtomicF_GetUnchecked (PlankAtomicFRef p)
{
    return  p->value;
}

static inline PlankF pl_AtomicF_Swap (PlankAtomicFRef p, PlankF newValue)
{
    PlankF oldValue;
    PlankB success;
    
    do
    {
        oldValue = *(PlankF*)p;
        success = pl_AtomicF_CompareAndSwap (p, oldValue, newValue);
    } while (!success);
    
    return oldValue;
}

static inline void pl_AtomicF_SwapOther (PlankAtomicFRef p1, PlankAtomicFRef p2)
{
    PlankF value1, value2;
    PlankB success;
    
    do {
        value1 = *(PlankF*)p1;
        value2 = *(PlankF*)p2;
        success = pl_AtomicF_CompareAndSwap (p1, value1, value2);
    } while (!success);
    
    *(PlankF*)p2 = value1;
}

static inline void pl_AtomicF_Set (PlankAtomicFRef p, PlankF newValue)
{
    pl_AtomicF_Swap (p, newValue);
}

static inline PlankF pl_AtomicF_Add (PlankAtomicFRef p, PlankF operand)
{
    PlankF newValue, oldValue;
    PlankB success;
    
    do {
        oldValue = *(PlankF*)p;
        newValue = oldValue + operand;
        success = pl_AtomicF_CompareAndSwap (p, oldValue, newValue);
    } while (!success);
    
    return newValue;
}

#if PLANK_APPLE
static inline PlankB pl_AtomicF_CompareAndSwap (PlankAtomicFRef p, PlankF oldValue, PlankF newValue)
{    
    return OSAtomicCompareAndSwap32Barrier (*(int32_t*)&oldValue, 
                                            *(int32_t*)&newValue, 
                                            (volatile int32_t*)p);
}
#endif

#if PLANK_WIN
static inline PlankB pl_AtomicF_CompareAndSwap (PlankAtomicFRef p, PlankF oldValue, PlankF newValue)
{   
	long oldLong = *(long*)&oldValue;
    return oldLong == _InterlockedCompareExchange ((volatile long*)p,
                                                   *(long*)&newValue, 
                                                   oldLong);
}
#endif

#if PLANK_ANDROID
static inline PlankB pl_AtomicF_CompareAndSwap (PlankAtomicFRef p, PlankF oldValue, PlankF newValue)
{
    return __sync_bool_compare_and_swap ((volatile PlankI*)p,
                                         *(PlankI*)&oldValue,
                                         *(PlankI*)&newValue);
}
#endif

static inline PlankF pl_AtomicF_Subtract (PlankAtomicFRef p, PlankF operand)
{
    return pl_AtomicF_Add (p, -operand);
}

static inline PlankF pl_AtomicF_Increment (PlankAtomicFRef p)
{
    return pl_AtomicF_Add (p, 1.f);
}

static inline PlankF pl_AtomicF_Decrement (PlankAtomicFRef p)
{
    return pl_AtomicF_Add (p, -1.f);
}

//------------------------------------------------------------------------------

static inline PlankResult pl_AtomicD_Init (PlankAtomicDRef p)
{
    PlankResult result = PlankResult_OK;
    
    if (p == PLANK_NULL)
    {
        result = PlankResult_MemoryError;
        goto exit;
    }
    
#if PLANK_NOATOMIC64BIT
    pl_ThreadSpinLock_Init (&p->lock);
#endif
    
    //    pl_AtomicD_Set (p, 0.0);
    p->value = 0.0;
    
exit:
    return result;
}

static inline PlankResult pl_AtomicD_DeInit (PlankAtomicDRef p)
{
    PlankResult result = PlankResult_OK;
    
    if (p == PLANK_NULL)
    {
        result = PlankResult_MemoryError;
        goto exit;
    }
    
#if PLANK_NOATOMIC64BIT
    pl_ThreadSpinLock_DeInit (&p->lock);
#endif
    
exit:
    return result;
}

static inline PlankD pl_AtomicD_Get (PlankAtomicDRef p)
{
#if PLANK_32BIT
    PlankLL bits = pl_AtomicLL_Add ((PlankAtomicLLRef)p, (PlankLL)0); // use the LL version
    return *(PlankD*)&bits;
#endif
    
#if PLANK_64BIT
    return p->value; // should be aligned anyway and volatile so OK
#endif
}

static inline PlankD pl_AtomicD_GetUnchecked (PlankAtomicDRef p)
{
    return p->value;
}

static inline PlankD pl_AtomicD_Swap (PlankAtomicDRef p, PlankD newValue)
{
    PlankD oldValue;
    PlankB success;
    
    do {
        oldValue = *(PlankD*)p;
        success = pl_AtomicD_CompareAndSwap (p, oldValue, newValue);
    } while (!success);
    
    return oldValue;
}

static inline void pl_AtomicD_SwapOther (PlankAtomicDRef p1, PlankAtomicDRef p2)
{
    PlankD value1, value2;
    PlankB success;
    
    do {
        value1 = *(PlankD*)p1;
        value2 = *(PlankD*)p2;
        success = pl_AtomicD_CompareAndSwap (p1, value1, value2);
    } while (!success);
    
    *(PlankD*)p2 = value1;
}

static inline void pl_AtomicD_Set (PlankAtomicDRef p, PlankD newValue)
{
    pl_AtomicD_Swap (p, newValue);
}

static inline PlankD pl_AtomicD_Add (PlankAtomicDRef p, PlankD operand)
{
    PlankD newValue, oldValue;
    PlankB success;
    
    do {
        oldValue = *(PlankD*)p;
        newValue = oldValue + operand;
        success = pl_AtomicD_CompareAndSwap (p, oldValue, newValue);
    } while (!success);
    
    return newValue;
}

#if PLANK_APPLE
#if PLANK_X86 || PLANK_ARM // and PLANK_APPLE
static inline PlankB pl_AtomicD_CompareAndSwap (PlankAtomicDRef p, PlankD oldValue, PlankD newValue)
{    
    return OSAtomicCompareAndSwap64Barrier (*(int64_t*)&oldValue, 
                                            *(int64_t*)&newValue, 
                                            (volatile int64_t*)p);
}
#endif // PLANK_X86 and PLANK_APPLE

#if PLANK_PPC // and PLANK_APPLE
static inline PlankB pl_AtomicD_CompareAndSwap (PlankAtomicDRef p, PlankD oldValue, PlankD newValue)
{    
    // must use an integer compare since comparing floating point 
    // values doesn't do a bitwise comparison
    return pl_AtomicLL_CompareAndSwap ((PlankAtomicLLRef)p, 
                                       *(PlankLL*)&oldValue, 
                                       *(PlankLL*)&newValue);
}
#endif // PLANK_PPC and PLANK_APPLE
#endif // PLANK_APPLE

#if PLANK_WIN
inline PlankB pl_AtomicD_CompareAndSwap (PlankAtomicDRef p, PlankD oldValue, PlankD newValue)
{   
    PlankULL oldBits = *(PlankULL*)&oldValue;
    return oldBits == pl_InterlockedCompareExchange64 ((volatile PlankULL*)p,
                                                       *(PlankULL*)&newValue, 
                                                       oldBits);
}
#endif

#if PLANK_ANDROID
static inline PlankB pl_AtomicD_CompareAndSwap (PlankAtomicDRef p, PlankD oldValue, PlankD newValue)
{
    return __sync_bool_compare_and_swap ((volatile PlankLL*)p,
                                         *(PlankLL*)&oldValue,
                                         *(PlankLL*)&newValue);
}
#endif

static inline PlankD pl_AtomicD_Subtract (PlankAtomicDRef p, PlankD operand)
{
    return pl_AtomicD_Add (p, -operand);
}

static inline PlankD pl_AtomicD_Increment (PlankAtomicDRef p)
{
    return pl_AtomicD_Add (p, 1.0);
}

static inline PlankD pl_AtomicD_Decrement (PlankAtomicDRef p)
{
    return pl_AtomicD_Add (p, -1.0);
}

//------------------------------------------------------------------------------

static inline PlankP pl_AtomicP_Get (PlankAtomicPRef p)
{
    return p->ptr; // should be aligned anyway and volatile so OK // pl_AtomicP_Add (p, (PlankL)0);
}

static inline PlankP pl_AtomicP_GetUnchecked (PlankAtomicPRef p)
{
    return p->ptr;
}

static inline PlankP pl_AtomicP_Swap (PlankAtomicPRef p, PlankP newPtr)
{
    PlankP oldPtr;
    PlankB success;
    
    do {
        oldPtr = *(PlankP*)p;
        success = pl_AtomicP_CompareAndSwap (p, oldPtr, newPtr);
    } while (!success);
    
    return oldPtr;    
}

static inline void pl_AtomicP_SwapOther (PlankAtomicPRef p1, PlankAtomicPRef p2)
{
    PlankP value1, value2;
    PlankB success;
    
    do {
        value1 = *(PlankP*)p1;
        value2 = *(PlankP*)p2;
        success = pl_AtomicP_CompareAndSwap (p1, value1, value2);
    } while (!success);
    
    *(PlankP*)p2 = value1;
}

static inline void pl_AtomicP_Set (PlankAtomicPRef p, PlankP newPtr)
{
    pl_AtomicP_Swap (p, newPtr);
}

#if PLANK_APPLE
#if PLANK_32BIT // and PLANK_APPLE
static inline PlankP pl_AtomicP_Add (PlankAtomicPRef p, PlankL operand)
{
    return (PlankP)OSAtomicAdd32Barrier (*(int32_t*)&operand, (volatile int32_t*)p);
}

static inline PlankB pl_AtomicP_CompareAndSwap (PlankAtomicPRef p, PlankP oldValue, PlankP newValue)
{
    return OSAtomicCompareAndSwap32Barrier (*(int32_t*)&oldValue, 
                                            *(int32_t*)&newValue, 
                                            (volatile int32_t*)p);
}
#endif // PLANK_32BIT and PLANK_APPLE

#if PLANK_64BIT // and PLANK_APPLE
static inline PlankP pl_AtomicP_Add (PlankAtomicPRef p, PlankL operand)
{
    return (PlankP)OSAtomicAdd64Barrier (*(int64_t*)&operand, (volatile int64_t*)p);
}

static inline PlankB pl_AtomicP_CompareAndSwap (PlankAtomicPRef p, PlankP oldValue, PlankP newValue)
{
    return OSAtomicCompareAndSwap64Barrier (*(int64_t*)&oldValue, 
                                            *(int64_t*)&newValue, 
                                            (volatile int64_t*)p);
}
#endif // PLANK_64BIT and PLANK_APPLE
#endif // PLANK_APPLE

#if PLANK_WIN
#if PLANK_32BIT
static inline PlankP pl_AtomicP_Add (PlankAtomicPRef p, PlankL operand)
{
    return (PlankP)(_InterlockedExchangeAdd ((volatile long*)&p, operand) + operand);
}

static inline PlankB pl_AtomicP_CompareAndSwap (PlankAtomicPRef p, PlankP oldValue, PlankP newValue)
{    
	long oldLong = *(long*)&oldValue;
    return oldLong == _InterlockedCompareExchange ((volatile long*)p, 
                                                   *(long*)&newValue, 
                                                   oldLong);
}
#endif

#if PLANK_64BIT
static inline PlankP pl_AtomicP_Add (PlankAtomicPRef p, PlankL operand)
{
    PlankP oldValue, newValue;
    PlankB success;
    
    do {
        oldValue = *(PlankP*)p;
        newValue = (PlankUC*)oldValue + operand;
        success = pl_AtomicP_CompareAndSwap (p, oldValue, newValue);
    } while (!success);
    
    return newValue;        
}

static inline PlankB pl_AtomicP_CompareAndSwap (PlankAtomicPRef p, PlankP oldValue, PlankP newValue)
{    
    PlankULL oldBits = *(PlankULL*)&oldValue;
    return oldBits == pl_InterlockedCompareExchange64 ((volatile PlankULL*)p, 
                                                       *(PlankULL*)&newValue, 
                                                       *(PlankULL*)&oldValue);    
}
#endif
#endif


#if PLANK_ANDROID
static inline PlankP pl_AtomicP_Add (PlankAtomicPRef p, PlankL operand)
{
    return (PlankP)__sync_add_and_fetch ((volatile PlankL*)p, operand);
}

static inline PlankB pl_AtomicP_CompareAndSwap (PlankAtomicPRef p, PlankP oldValue, PlankP newValue)
{
    return __sync_bool_compare_and_swap ((volatile PlankL*)p,
                                         *(PlankL*)&oldValue,
                                         *(PlankL*)&newValue);
}
#endif


static inline PlankP pl_AtomicP_Subtract (PlankAtomicPRef p, PlankL operand)
{
    return pl_AtomicP_Add (p, -operand);
}

static inline PlankP pl_AtomicP_Increment (PlankAtomicPRef p)
{
    return pl_AtomicP_Add (p, (PlankL)1);
}

static inline PlankP pl_AtomicP_Decrement (PlankAtomicPRef p)
{
    return pl_AtomicP_Add (p, (PlankL)(-1));
}

//------------------------------------------------------------------------------

static inline PlankAtomicPXRef pl_AtomicPX_CreateAndInit()
{
    PlankAtomicPXRef p = pl_AtomicPX_Create();
    if (p != PLANK_NULL) pl_AtomicPX_Init (p);
    return p;
}

static inline PlankAtomicPXRef pl_AtomicPX_Create()
{
    PlankMemoryRef m;
    PlankAtomicPXRef p;
    
    m = pl_MemoryGlobal();
    p = (PlankAtomicPXRef)pl_Memory_AllocateBytes (m, sizeof (PlankAtomicPX));
    
    if (p != PLANK_NULL)
        pl_MemoryZero (p, sizeof (PlankAtomicPX));
    
    return p;
}

static inline PlankResult pl_AtomicPX_Init (PlankAtomicPXRef p)
{
    PlankResult result = PlankResult_OK;
    
    if (p == PLANK_NULL)
    {
        result = PlankResult_MemoryError;
        goto exit;
    }
    
#if PLANK_NOATOMIC64BIT
    pl_ThreadSpinLock_Init (&p->lock);
#endif
    
    //    pl_AtomicPX_SetAll (p, (PlankP)0, (PlankL)0);
    pl_MemoryZero (p, sizeof (PlankAtomicPX));
    
exit:
    return result;
}

static inline PlankResult pl_AtomicPX_DeInit (PlankAtomicPXRef p)
{
    PlankResult result = PlankResult_OK;
    
    if (p == PLANK_NULL)
    {
        result = PlankResult_MemoryError;
        goto exit;
    }
    
#if PLANK_NOATOMIC64BIT
    pl_ThreadSpinLock_DeInit (&p->lock);
#endif
    
exit:
    return result;
}

static inline PlankResult pl_AtomicPX_Destroy (PlankAtomicPXRef p)
{
    PlankResult result;
    PlankMemoryRef m;
    
    result = PlankResult_OK;
    m = pl_MemoryGlobal();
    
    if ((result = pl_AtomicPX_DeInit (p)) != PlankResult_OK)
        goto exit;
    
    result = pl_Memory_Free (m, p);
    
exit:
    return result;
}

static inline PlankP pl_AtomicPX_Get (PlankAtomicPXRef p)
{
    return p->ptr; // should be aligned anyway and volatile so OK // pl_AtomicP_Get ((PlankAtomicPRef)p);
}

static inline PlankP pl_AtomicPX_GetUnchecked (PlankAtomicPXRef p)
{
    return p->ptr;
}

static inline PlankL pl_AtomicPX_GetExtra (PlankAtomicPXRef p)
{
    return p->extra; // should be aligned anyway and volatile so OK // pl_AtomicL_Get ((PlankAtomicLRef)&(p->extra));
}

static inline PlankL pl_AtomicPX_GetExtraUnchecked (PlankAtomicPXRef p)
{
    return p->extra;
}

static inline PlankP pl_AtomicPX_SwapAll (PlankAtomicPXRef p, PlankP newPtr, PlankL newExtra, PlankL* oldExtraPtr)
{
    PlankP oldPtr;
    PlankL oldExtra;
    PlankB success;
    
    do {
        oldPtr = p->ptr;
        oldExtra = p->extra;
        success = pl_AtomicPX_CompareAndSwap (p, oldPtr, oldExtra, newPtr, newExtra);
    } while (!success);
    
    if (oldExtraPtr != PLANK_NULL)
        *oldExtraPtr = oldExtra;
    
    return oldPtr;
}

static inline PlankP pl_AtomicPX_Swap (PlankAtomicPXRef p, PlankP newPtr)
{
    PlankP oldPtr;
    PlankL oldExtra;
    PlankB success;
    
    do {
        oldPtr = p->ptr;
        oldExtra = p->extra;
        success = pl_AtomicPX_CompareAndSwap (p, oldPtr, oldExtra, newPtr, oldExtra + 1);
    } while (!success);
    
    return oldPtr;
}

static inline void pl_AtomicPX_SwapOther (PlankAtomicPXRef p1, PlankAtomicPXRef p2)
{
    PlankAtomicPX tmp1, tmp2;
    PlankB success;
    
    do {
        tmp1 = *p1;
        tmp2 = *p2;
        success = pl_AtomicPX_CompareAndSwap (p1, tmp1.ptr, tmp1.extra, tmp2.ptr, tmp1.extra + 1);
    } while (!success);
    
    pl_AtomicPX_Set (p2, tmp1.ptr);
}

static inline void pl_AtomicPX_SetAll (PlankAtomicPXRef p, PlankP newPtr, PlankL newExtra)
{
    pl_AtomicPX_SwapAll (p, newPtr, newExtra, (PlankL*)PLANK_NULL);
}

static inline void pl_AtomicPX_Set (PlankAtomicPXRef p, PlankP newPtr)
{
    PlankP oldPtr;
    PlankL oldExtra;
    PlankB success;
    
    do {
        oldPtr = p->ptr;
        oldExtra = p->extra;
        success = pl_AtomicPX_CompareAndSwap (p, oldPtr, oldExtra, newPtr, oldExtra + 1);
    } while (!success);
}

static inline PlankP pl_AtomicPX_Add (PlankAtomicPXRef p, PlankL operand)
{
    PlankP newPtr, oldPtr;
    PlankL oldExtra;
    PlankB success;
    
    do {
        oldPtr = p->ptr;
        oldExtra = p->extra;
        newPtr = (PlankUC*)oldPtr + operand;
        success = pl_AtomicPX_CompareAndSwap (p, oldPtr, oldExtra, newPtr, oldExtra + 1);
    } while (!success);
    
    return newPtr;
}

static inline PlankP pl_AtomicPX_Subtract (PlankAtomicPXRef p, PlankL operand)
{
    return pl_AtomicPX_Add (p, -operand);
}

static inline PlankP pl_AtomicPX_Increment (PlankAtomicPXRef p)
{
    return pl_AtomicPX_Add (p, (PlankL)1);
}

static inline PlankP pl_AtomicPX_Decrement (PlankAtomicPXRef p)
{
    return pl_AtomicPX_Add (p, (PlankL)(-1));
}


#if PLANK_APPLE
#if PLANK_X86
#if PLANK_32BIT
static inline  PlankB pl_AtomicPX_CompareAndSwap (PlankAtomicPXRef p, PlankP oldPtr, PlankL oldExtra, PlankP newPtr, PlankL newExtra)
{
    char success;
#if __PIC__
    /* If PIC is turned on, we can't use %ebx as it is reserved for the
     GOT pointer.  We can save and restore %ebx because GCC won't be
     using it for anything else (such as any of the m operands) */
    __asm__ __volatile__("pushl %%ebx;"   /* save ebx used for PIC GOT ptr */
                         "movl %6,%%ebx;" /* move new_val2 to %ebx */
                         "lock; cmpxchg8b %0; setz %1;"
                         "pop %%ebx;"     /* restore %ebx */
                         : "=m"(*p), "=a"(success)
                         : "m"(*p), "d" (oldExtra), "a" (oldPtr),
                         "c" (newExtra), "m" (newPtr) : "memory");
#else // !__PIC__
    /* We can't just do the same thing in non-PIC mode, because GCC
     * might be using %ebx as the memory operand.  We could have ifdef'd
     * in a clobber, but there's no point doing the push/pop if we don't
     * have to. */
    __asm__ __volatile__("lock; cmpxchg8b %0; setz %1;"
                         : "=m"(*p), "=a"(success)
                         : "m"(*p), "d" (oldExtra), "a" (oldPtr),
                         "c" (newExtra), "b" (newPtr) : "memory");
#endif // !__PIC__
    return success;
}
#endif // PLANK_32BIT

#if PLANK_64BIT
static inline  PlankB pl_AtomicPX_CompareAndSwap (PlankAtomicPXRef p, PlankP oldPtr, PlankL oldExtra, PlankP newPtr, PlankL newExtra)
{
    char success;
    __asm__ __volatile__("lock; cmpxchg16b %0; setz %1"
                         : "=m"(*p), "=a"(success)
                         : "m"(*p), "d" (oldExtra), "a" (oldPtr),
                         "c" (newExtra), "b" (newPtr) : "memory");
    return success;
}
#endif //PLANK_64BIT
#endif //PLANK_X86

#if PLANK_PPC // and PLANK_APPLE
static inline PlankB pl_AtomicPX_CompareAndSwap (PlankAtomicPXRef p, PlankP oldPtr, PlankL oldExtra, PlankP newPtr, PlankL newExtra)
{
    PlankAtomicPX oldAll = { oldPtr, oldExtra, p->lock };
    PlankAtomicPX newAll = { newPtr, newExtra, p->lock };
    
    return pl_AtomicLL_CompareAndSwap ((PlankAtomicLLRef)p,
                                       *(PlankLL*)&oldAll,
                                       *(PlankLL*)&newAll);
}
#endif // PLANK_PPC and PLANK_APPLE

#if PLANK_ARM // and PLANK_APPLE

#if PLANK_64BIT // ARM
#error pl_AtomicPX_CompareAndSwap need to implement 128-bit CAS for this platform
#endif // PLANK_64BIT

#if PLANK_32BIT // ARM
static inline  PlankB pl_AtomicPX_CompareAndSwap (PlankAtomicPXRef p, PlankP oldPtr, PlankL oldExtra, PlankP newPtr, PlankL newExtra)
{
    PlankAtomicPX oldAll = { oldPtr, oldExtra };
    PlankAtomicPX newAll = { newPtr, newExtra };
    
    return OSAtomicCompareAndSwap64Barrier (*(int64_t*)&oldAll,
                                            *(int64_t*)&newAll,
                                            (volatile int64_t*)p);
}
#endif // PLANK_32BIT ARM
#endif // PLANK_ARM and PLANK_APPLE

#endif // PLANK_APPLE

#if PLANK_WIN
#if PLANK_32BIT
static inline PlankB pl_AtomicPX_CompareAndSwap (PlankAtomicPXRef p, PlankP oldPtr, PlankL oldExtra, PlankP newPtr, PlankL newExtra)
{
    // can't use static init as MSVC C is C89
    PlankAtomicPX oldAll;
    PlankAtomicPX newAll;
    PlankULL oldAllValue;
    PlankULL newAllValue;
    
	oldAll.ptr   = oldPtr;
	oldAll.extra = oldExtra;
	newAll.ptr   = newPtr;
	newAll.extra = newExtra;
    oldAllValue  = *(PlankULL*)&oldAll;
	newAllValue  = *(PlankULL*)&newAll;
    
	return oldAllValue == pl_InterlockedCompareExchange64 ((volatile PlankULL*)p, newAllValue, oldAllValue);
}
#endif

#if PLANK_64BIT
static inline PlankB pl_AtomicPX_CompareAndSwap (PlankAtomicPXRef p, PlankP oldPtr, PlankL oldExtra, PlankP newPtr, PlankL newExtra)
{
    PlankAtomicPX oldAll;
	oldAll.ptr = oldPtr;
	oldAll.extra = oldExtra;
    return _InterlockedCompareExchange128 ((volatile __int64*)p,
                                           *(__int64*)&newExtra,
                                           *(__int64*)&newPtr,
                                           (__int64*)&oldAll);
}
#endif
#endif // PLANK_WIN

#if PLANK_ANDROID
#if PLANK_32BIT
static inline  PlankB pl_AtomicPX_CompareAndSwap (PlankAtomicPXRef p, PlankP oldPtr, PlankL oldExtra, PlankP newPtr, PlankL newExtra)
{
    PlankAtomicPX oldAll = { oldPtr, oldExtra };
    PlankAtomicPX newAll = { newPtr, newExtra };
    
    return __sync_bool_compare_and_swap ((volatile PlankLL*)p,
                                         *(PlankLL*)&oldAll,
                                         *(PlankLL*)&newAll);
}
#endif // PLANK_32BIT

#if PLANK_64BIT

#if PLANK_ARM
#error pl_AtomicPX_CompareAndSwap need to implement 128-bit CAS for this platform
#endif

#if PLANK_X86
static inline  PlankB pl_AtomicPX_CompareAndSwap (PlankAtomicPXRef p, PlankP oldPtr, PlankL oldExtra, PlankP newPtr, PlankL newExtra)
{
    char success;
    __asm__ __volatile__("lock; cmpxchg16b %0; setz %1"
                         : "=m"(*p), "=a"(success)
                         : "m"(*p), "d" (oldExtra), "a" (oldPtr),
                         "c" (newExtra), "b" (newPtr) : "memory");
    return success;
}
#endif

#endif // PLANK_64BIT
#endif // PLANK_ANDROID

//------------------------------------------------------------------------------

static inline PlankAtomicLXRef pl_AtomicLX_CreateAndInit()
{
    PlankAtomicLXRef p = pl_AtomicLX_Create();
    if (p != PLANK_NULL) pl_AtomicLX_Init (p);
    return p;
}

static inline PlankAtomicLXRef pl_AtomicLX_Create()
{
    PlankMemoryRef m;
    PlankAtomicLXRef p;
    
    m = pl_MemoryGlobal();
    p = (PlankAtomicLXRef)pl_Memory_AllocateBytes (m, sizeof (PlankAtomicLX));
    
    if (p != PLANK_NULL)
        pl_MemoryZero (p, sizeof (PlankAtomicLX));
    
    return p;
}

static inline PlankResult pl_AtomicLX_Init (PlankAtomicLXRef p)
{
    PlankResult result = PlankResult_OK;
    
    if (p == PLANK_NULL)
    {
        result = PlankResult_MemoryError;
        goto exit;
    }
    
#if PLANK_NOATOMIC64BIT
    pl_ThreadSpinLock_Init (&p->lock);
#endif
    
    //    pl_AtomicLX_SetAll (p, (PlankL)0, (PlankL)0);
    pl_MemoryZero (p, sizeof (PlankAtomicLX));
    
exit:
    return result;
}

static inline PlankResult pl_AtomicLX_DeInit (PlankAtomicLXRef p)
{
    PlankResult result = PlankResult_OK;
    
    if (p == PLANK_NULL)
    {
        result = PlankResult_MemoryError;
        goto exit;
    }
    
#if PLANK_NOATOMIC64BIT
    pl_ThreadSpinLock_DeInit (&p->lock);
#endif
    
exit:
    return result;
}

static inline PlankResult pl_AtomicLX_Destroy (PlankAtomicLXRef p)
{
    PlankResult result;
    PlankMemoryRef m;
    
    result = PlankResult_OK;
    m = pl_MemoryGlobal();
    
    if ((result = pl_AtomicLX_DeInit (p)) != PlankResult_OK)
        goto exit;
    
    result = pl_Memory_Free (m, p);
    
exit:
    return result;
}

static inline PlankL pl_AtomicLX_Get (PlankAtomicLXRef p)
{
    return p->value; // should be aligned anyway and volatile so OK // pl_AtomicL_Get ((PlankAtomicLRef)p);
}

static inline PlankL pl_AtomicLX_GetUnchecked (PlankAtomicLXRef p)
{
    return p->value;
}

static inline PlankL pl_AtomicLX_GetExtra (PlankAtomicLXRef p)
{
    return p->extra; // should be aligned anyway and volatile so OK // pl_AtomicL_Get ((PlankAtomicLRef)&(p->extra));
}

static inline PlankL pl_AtomicLX_GetExtraUnchecked (PlankAtomicLXRef p)
{
    return p->extra;
}

static inline PlankL pl_AtomicLX_SwapAll (PlankAtomicLXRef p, PlankL newValue, PlankL newExtra, PlankL* oldExtraPtr)
{
    PlankL oldValue;
    PlankL oldExtra;
    PlankB success;
    
    do {
        oldValue = p->value;
        oldExtra = p->extra;
        success = pl_AtomicLX_CompareAndSwap (p, oldValue, oldExtra, newValue, newExtra);
    } while (!success);
    
    if (oldExtraPtr != PLANK_NULL)
        *oldExtraPtr = oldExtra;
    
    return oldValue;
}

static inline PlankL pl_AtomicLX_Swap (PlankAtomicLXRef p, PlankL newValue)
{
    PlankL oldValue;
    PlankL oldExtra;
    PlankB success;
    
    do {
        oldValue = p->value;
        oldExtra = p->extra;
        success = pl_AtomicLX_CompareAndSwap (p, oldValue, oldExtra, newValue, oldExtra + 1);
    } while (!success);
    
    return oldValue;
}

static inline void pl_AtomicLX_SwapOther (PlankAtomicLXRef p1, PlankAtomicLXRef p2)
{
    PlankAtomicLX tmp1, tmp2;
    PlankB success;
    
    do {
        tmp1 = *p1;
        tmp2 = *p2;
        success = pl_AtomicLX_CompareAndSwap (p1, tmp1.value, tmp1.extra, tmp2.value, tmp1.extra + 1);
    } while (!success);
    
    pl_AtomicLX_Set (p2, tmp1.value);
}

static inline void pl_AtomicLX_SetAll (PlankAtomicLXRef p, PlankL newValue, PlankL newExtra)
{
    pl_AtomicLX_SwapAll (p, newValue, newExtra, (PlankL*)PLANK_NULL);
}

static inline void pl_AtomicLX_Set (PlankAtomicLXRef p, PlankL newValue)
{
    PlankL oldValue;
    PlankL oldExtra;
    PlankB success;
    
    do {
        oldValue = p->value;
        oldExtra = p->extra;
        success = pl_AtomicLX_CompareAndSwap (p, oldValue, oldExtra, newValue, oldExtra + 1);
    } while (!success);
}

static inline PlankL pl_AtomicLX_Add (PlankAtomicLXRef p, PlankL operand)
{
    PlankL newValue, oldValue;
    PlankL oldExtra;
    PlankB success;
    
    do {
        oldValue = p->value;
        oldExtra = p->extra;
        newValue = oldValue + operand;
        success = pl_AtomicLX_CompareAndSwap (p, oldValue, oldExtra, newValue, oldExtra + 1);
    } while (!success);
    
    return newValue;
}

static inline PlankL pl_AtomicLX_Subtract (PlankAtomicLXRef p, PlankL operand)
{
    return pl_AtomicLX_Add (p, -operand);
}

static inline PlankL pl_AtomicLX_Increment (PlankAtomicLXRef p)
{
    return pl_AtomicLX_Add (p, (PlankL)1);
}

static inline PlankL pl_AtomicLX_Decrement (PlankAtomicLXRef p)
{
    return pl_AtomicLX_Add (p, (PlankL)(-1));
}


#if PLANK_APPLE
#if PLANK_X86
#if PLANK_32BIT
static inline  PlankB pl_AtomicLX_CompareAndSwap (PlankAtomicLXRef p, PlankL oldValue, PlankL oldExtra, PlankL newValue, PlankL newExtra)
{
    char success;
#if __PIC__
    /* If PIC is turned on, we can't use %ebx as it is reserved for the
     GOT pointer.  We can save and restore %ebx because GCC won't be
     using it for anything else (such as any of the m operands) */
    __asm__ __volatile__("pushl %%ebx;"   /* save ebx used for PIC GOT ptr */
                         "movl %6,%%ebx;" /* move new_val2 to %ebx */
                         "lock; cmpxchg8b %0; setz %1;"
                         "pop %%ebx;"     /* restore %ebx */
                         : "=m"(*p), "=a"(success)
                         : "m"(*p), "d" (oldExtra), "a" (oldValue),
                         "c" (newExtra), "m" (newValue) : "memory");
#else // !__PIC__
    /* We can't just do the same thing in non-PIC mode, because GCC
     * might be using %ebx as the memory operand.  We could have ifdef'd
     * in a clobber, but there's no point doing the push/pop if we don't
     * have to. */
    __asm__ __volatile__("lock; cmpxchg8b %0; setz %1;"
                         : "=m"(*p), "=a"(success)
                         : "m"(*p), "d" (oldExtra), "a" (oldValue),
                         "c" (newExtra), "b" (newValue) : "memory");
#endif // !__PIC__
    return success;
}
#endif // PLANK_32BIT

#if PLANK_64BIT
static inline  PlankB pl_AtomicLX_CompareAndSwap (PlankAtomicLXRef p, PlankL oldValue, PlankL oldExtra, PlankL newValue, PlankL newExtra)
{
    char success;
    __asm__ __volatile__("lock; cmpxchg16b %0; setz %1"
                         : "=m"(*p), "=a"(success)
                         : "m"(*p), "d" (oldExtra), "a" (oldValue),
                         "c" (newExtra), "b" (newValue) : "memory");
    return success;
}
#endif //PLANK_64BIT
#endif //PLANK_X86

#if PLANK_PPC // and PLANK_APPLE
static inline  PlankB pl_AtomicLX_CompareAndSwap (PlankAtomicLXRef p, PlankL oldValue, PlankL oldExtra, PlankP newValue, PlankL newExtra)
{
    PlankAtomicLX oldAll = { oldValue, oldExtra, p->lock };
    PlankAtomicLX newAll = { newValue, newExtra, p->lock };
    
    return pl_AtomicLL_CompareAndSwap ((PlankAtomicLLRef)p,
                                       *(PlankLL*)&oldAll,
                                       *(PlankLL*)&newAll);
}
#endif // PLANK_PPC and PLANK_APPLE

#if PLANK_ARM // and PLANK_APPLE
static inline  PlankB pl_AtomicLX_CompareAndSwap (PlankAtomicLXRef p, PlankL oldValue, PlankL oldExtra, PlankL newValue, PlankL newExtra)
{
    PlankAtomicLX oldAll = { oldValue, oldExtra };
    PlankAtomicLX newAll = { newValue, newExtra };
    
    return OSAtomicCompareAndSwap64Barrier (*(int64_t*)&oldAll,
                                            *(int64_t*)&newAll,
                                            (volatile int64_t*)p);
}
#endif // PLANK_ARM and PLANK_APPLE

#endif // PLANK_APPLE

#if PLANK_WIN
#if PLANK_32BIT
static inline PlankB pl_AtomicLX_CompareAndSwap (PlankAtomicLXRef p, PlankL oldValue, PlankL oldExtra, PlankL newValue, PlankL newExtra)
{
    PlankAtomicLX oldAll;
    PlankAtomicLX newAll;
    PlankULL oldAllValue;
    PlankULL newAllValue;
    
	oldAll.value = oldValue;
	oldAll.extra = oldExtra;
    newAll.value = newValue;
	newAll.extra = newExtra;
	oldAllValue  = *(PlankULL*)&oldAll;
    newAllValue  = *(PlankULL*)&newAll;
    
	return oldAllValue == pl_InterlockedCompareExchange64 ((volatile PlankULL*)p, newAllValue, oldAllValue);
}
#endif

#if PLANK_64BIT
static inline PlankB pl_AtomicLX_CompareAndSwap (PlankAtomicLXRef p, PlankL oldValue, PlankL oldExtra, PlankL newValue, PlankL newExtra)
{
    PlankAtomicLX oldAll;
	oldAll.value = oldValue;
	oldAll.extra = oldExtra;
    return _InterlockedCompareExchange128 ((volatile __int64*)p,
                                           *(__int64*)&newExtra,
                                           *(__int64*)&newValue,
                                           (__int64*)&oldAll);
}
#endif
#endif

#if PLANK_ANDROID
#if PLANK_32BIT
static inline PlankB pl_AtomicLX_CompareAndSwap (PlankAtomicLXRef p, PlankL oldPtr, PlankL oldExtra, PlankL newPtr, PlankL newExtra)
{
    PlankAtomicLX oldAll = { oldPtr, oldExtra };
    PlankAtomicLX newAll = { newPtr, newExtra };
    
    return __sync_bool_compare_and_swap ((volatile PlankLL*)p,
                                         *(PlankLL*)&oldAll,
                                         *(PlankLL*)&newAll);
}
#endif // PLANK_32BIT

#if PLANK_64BIT

#if PLANK_ARM
#error pl_AtomicPX_CompareAndSwap need to implement 128-bit CAS for this platform
#endif

#if PLANK_X86
static inline PlankB pl_AtomicLX_CompareAndSwap (PlankAtomicLXRef p, PlankL oldPtr, PlankL oldExtra, PlankL newPtr, PlankL newExtra)
{
    char success;
    __asm__ __volatile__("lock; cmpxchg16b %0; setz %1"
                         : "=m"(*p), "=a"(success)
                         : "m"(*p), "d" (oldExtra), "a" (oldPtr),
                         "c" (newExtra), "b" (newPtr) : "memory");
    return success;
}
#endif

#endif // PLANK_64BIT
#endif // PLANK_ANDROID

#define PLANK_ATOMICS_DEFINED 1

#endif // PLANK_INLINING_FUNCTIONS
