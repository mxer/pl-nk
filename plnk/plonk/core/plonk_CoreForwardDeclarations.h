/*
 -------------------------------------------------------------------------------
 This file is part of the Plink, Plonk, Plank libraries
  by Martin Robinson
 
 https://github.com/0x4d52/pl-nk/
 
 Copyright University of the West of England, Bristol 2011-15
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

#ifndef PLONK_COREFORWARDDECLARATIONS_H
#define PLONK_COREFORWARDDECLARATIONS_H

class SmartPointer;
class SmartPointerCounter;
class WeakPointer;
class TypeCode;

template<class Type>                                class TypeUtility;
template<class OriginalType>                        class WeakPointerContainer;
template<class SmartPointerType>                    class SmartPointerContainerBase;
template<class SmartPointerType,
         bool enableWeak = true>                    class SmartPointerContainer;
template<class SenderContainerBaseType>             class ReceiverInternal;
template<class ReceiverInternalType>                class ReceiverContainer;
template<class SenderContainerBaseType>             class SenderInternal;
template<class SenderInternalType>                  class SenderContainer;
template<class Type>                                class TypeUtility;
template<class TypeA, class TypeB>                  class BinaryOpTypeUtility;
template<class OperandType>                         class BinaryOpFunctions;
template<class OperandType>                         class BinaryOpFunctionsHelper;
template<class OperandType>                         class UnaryOpFunctions;
template<class OperandType>                         class UnaryOpFunctionsHelper;

typedef SmartPointerContainerBase<SmartPointer> GenericSmartPointerContainer;

#endif // PLONK_COREFORWARDDECLARATIONS_H