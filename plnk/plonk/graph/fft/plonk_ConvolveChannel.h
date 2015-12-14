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

#ifndef PLONK_CONVOLVECHANNEL_H
#define PLONK_CONVOLVECHANNEL_H

#include "../channel/plonk_ChannelInternalCore.h"
#include "../plonk_GraphForwardDeclarations.h"

#define PLONK_DEBUG_CONVOLVE 0//1

/** Convolve channel. */
template<class SampleType, typename FFTBuffersAccess>
class ConvolveChannelInternal
:   public ChannelInternal<SampleType, ChannelInternalCore::Data>
{
public:
    typedef FFTBuffersAccess                                        FFTBuffersAccessType;

    typedef ChannelInternalCore::Data                               Data;
    typedef ChannelBase<SampleType>                                 ChannelType;
    typedef ObjectArray<ChannelType>                                ChannelArrayType;
    typedef ConvolveChannelInternal<SampleType,FFTBuffersAccess>    ConvolveInternal;
    typedef ChannelInternal<SampleType,Data>                        Internal;
    typedef ChannelInternalBase<SampleType>                         InternalBase;
    typedef UnitBase<SampleType>                                    UnitType;
    typedef InputDictionary                                         Inputs;
    typedef NumericalArray<SampleType>                              Buffer;
    typedef FFTEngineBase<SampleType>                               FFTEngineType;
    typedef FFTBuffersBase<SampleType>                              FFTBuffersType;
    typedef Variable<FFTBuffersType&>                               FFTBuffersVariableType;
    
    ConvolveChannelInternal (Inputs const& inputs,
                             Data const& data,
                             BlockSize const& blockSize,
                             SampleRate const& sampleRate) throw()
    :   Internal (inputs, data, blockSize, sampleRate),
        processCountersCurrent(),
        processCountersPrevious(),
        irCountersCurrent(),
        irCountersPrevious(),
        countDown (0),
        position0 (0),
        position1 (0),
        fftAltSelect (0),
        dummyIRBuffers(),
        previousIRBuffers (dummyIRBuffers),
        currentIRBuffers (this->getInputAsFFTBuffers (IOKey::FFTBuffers).getValue())
    {
        const FFTBuffers& irBuffers (currentIRBuffers);
        const FFTEngineType& fftEngine (irBuffers.getFFTEngine());
        const int fftSize = fftEngine.length();
        const int fftSize2 = fftSize;// * 2;

        processBuffers = Buffer::withSize (fftSize2 * 7);
        SampleType* const processBuffersBase = processBuffers.getArray();
        
        fftAltBuffer0      = processBuffersBase + fftSize2 * 0;
        fftAltBuffer1      = processBuffersBase + fftSize2 * 1;
        fftTransformBuffer = processBuffersBase + fftSize2 * 2;
        fftOverlapBuffer   = processBuffersBase + fftSize2 * 3;
        fftBufferCurrent   = processBuffersBase + fftSize2 * 4;
        fftBufferPrevious  = processBuffersBase + fftSize2 * 5;
        fftCalcBuffer      = processBuffersBase + fftSize2 * 6;
        
#if 0 // PLONK_DEBUG
        printf ("fftAltBuffer0      = %p (%d)\n", fftAltBuffer0, fftSize2);
        printf ("fftAltBuffer1      = %p (%d)\n", fftAltBuffer1, fftSize2);
        printf ("fftTransformBuffer = %p (%d)\n", fftTransformBuffer, fftSize2);
        printf ("fftOverlapBuffer   = %p (%d)\n", fftOverlapBuffer, fftSize2);
        printf ("fftBufferCurrent   = %p (%d)\n", fftBufferCurrent, fftSize2);
        printf ("fftBufferPrevious  = %p (%d)\n", fftBufferPrevious, fftSize2);
        printf ("fftCalcBuffer      = %p (%d)\n", fftCalcBuffer, fftSize2);
        printf ("inputBuffer        = %p (%d)\n", inputBuffer.getArray(), inputBuffer.length());
#endif
    }
    
    Text getName() const throw()
    {
        return "Convolve";
    }
    
    IntArray getInputKeys() const throw()
    {
        const IntArray keys (IOKey::Generic, IOKey::FFTBuffers);
        return keys;
    }
    
    InternalBase* getChannel (const int index) throw()
    {
        const Inputs channelInputs = this->getInputs().getChannel (index);
        return new ConvolveInternal (channelInputs,
                                     this->getState(),
                                     this->getBlockSize(),
                                     this->getSampleRate());
    }
    
    void initChannel (const int channel) throw()
    {
        const UnitType& input = this->getInputAsUnit (IOKey::Generic);
        
        this->setBlockSize (input.getBlockSize (channel));
        this->setSampleRate (input.getSampleRate (channel));
        this->setOverlap (input.getOverlap (channel));
        
        this->initValue (SampleType (0));
        
        const FFTBuffersAccessType irBuffers (this->getInputAsFFTBuffers (IOKey::FFTBuffers).getValue());
        const FFTEngineType& fftEngine (irBuffers.getFFTEngine());
        
        reset (irBuffers, fftEngine, channel);
    }
    
    void reset (FFTBuffersType const& irBuffers, FFTEngineType const& fftEngine, const int channel) throw()
    {
        irCountersCurrent.reset();
        processCountersCurrent.zero();

        countDown                       = irBuffers.getCountDownStart (channel);
        position0                       = fftEngine.halfLength() - countDown;
        position1                       = position0 + countDown;
        fftAltSelect                    = 0;
        
        const UnsignedLong fftSize2 = (UnsignedLong) fftEngine.length() * 2;
        
        zeroSamples (fftAltBuffer0,      fftSize2);
        zeroSamples (fftAltBuffer1,      fftSize2);
        zeroSamples (fftTransformBuffer, fftSize2);
        zeroSamples (fftOverlapBuffer,   fftSize2);
        zeroSamples (fftBufferCurrent,      fftSize2);
    }

    static inline void complexMultiplyAccumulate (SampleType* const output,
                                                  const SampleType* const left, const SampleType* const right,
                                                  const UnsignedLong halfLength) throw()
    {
        NumericalArrayComplex<SampleType>::zpmulaccum (output, left, right, halfLength);
    }

    static inline void moveSamples (SampleType* const dst, const SampleType* const src, const UnsignedLong numItems) throw()
    {
        NumericalArrayUnaryOp<SampleType, plonk::move>::calc (dst, src, numItems);
    }
    
    static inline void accumulateSamples (SampleType* const dst, const SampleType* const src, const UnsignedLong numItems) throw()
    {
        NumericalArrayBinaryOp<SampleType, plonk::addop>::calcNN (dst, dst, src, numItems);
    }
    
    static inline void zeroSamples (SampleType* const dst, const UnsignedLong numItems) throw()
    {
        NumericalArray<SampleType>::zeroData (dst, numItems);
    }
    
    void processContinue (ProcessInfo& info, const int channel, FFTBuffersType& irBuffers, FFTEngineType& fftEngine) throw()
    {
        UnitType& inputUnit (this->getInputAsUnit (IOKey::Generic));
        const Buffer& inputBuffer (inputUnit.process (info, channel));
        const SampleType* inputSamples = inputBuffer.getArray();
        
        SampleType* outputSamples             = this->getOutputSamples();
        const UnsignedLong outputBufferLength = (UnsignedLong) this->getOutputBuffer().length();
        
        plonk_assert (outputBufferLength == inputBuffer.length());
        
        const int numIRDivisions      = irBuffers.getNumIRDivisions();
        const int numProcessDivisions = irBuffers.getNumProcessDivisions (channel);
        
        if (numIRDivisions == 0)
        {
            plonk_assertfalse;
            zeroSamples (outputSamples, outputBufferLength);
            return;
        }
        
        const UnsignedLong fftSize           = (UnsignedLong) fftEngine.length();
        const UnsignedLong fftSizeHalved     = (UnsignedLong) fftEngine.halfLength();
        const int divisionRatio1             = (fftSizeHalved / outputBufferLength) - 1;
        SampleType* const fftAltBuffers[]    = { fftAltBuffer0, fftAltBuffer1 };
        SampleType* const processBufferBase  = irBuffers.getProcessDivision (channel, 0);
        const SampleType* const irBufferBase = irBuffers.getIRDivision (channel, 0);

        int samplesRemaining                 = outputBufferLength;
        

#if PLONK_DEBUG_CONVOLVE
        printf ("convolve: %p(%d) start numDivisions=%d numProcessDivisions=%d\n", this, callbackCount, numIRDivisions, numProcessDivisions);
#endif

        while (samplesRemaining > 0)
        {
            int hop;
            
#if PLONK_DEBUG_CONVOLVE
            printf ("convolve: %p(%d) while (samplesRemaining) samplesRemaining=%d\n", this, callbackCount, samplesRemaining);
#endif
            
            if ((samplesRemaining - countDown) > 0)
            {
                hop = countDown;
                samplesRemaining -= countDown;
                countDown = 0;
            }
            else
            {
                hop = samplesRemaining;
                countDown -= samplesRemaining;
                samplesRemaining = 0;
            }
            
#if PLONK_DEBUG_CONVOLVE
            printf ("convolve: %p(%d) countDown=%d hop=%d position0=%d position1=%d\n", this, callbackCount, countDown, hop, position0, position1);
#endif
            
            if (hop > 0)
            {
#if PLONK_DEBUG_CONVOLVE
                printf ("convolve: %p(%d) ### INPUT SAMPLES ###\n", this, callbackCount);
#endif
                moveSamples (fftAltBuffer0 + position0, inputSamples, hop);
                moveSamples (fftAltBuffer1 + position1, inputSamples, hop);
                moveSamples (outputSamples, fftOverlapBuffer + position0, hop);
                
                inputSamples  += hop;
                outputSamples += hop;
                position0     += hop;
                position1     += hop;
            }
            
            hop = (irCountersCurrent.written >= irCountersCurrent.read - 1) ? 0 : 1;
            ++irCountersCurrent.count;
            
#if PLONK_DEBUG_CONVOLVE
            printf ("convolve: %p(%d) hop=%d countIRDivisions=%d position0=%d position1=%d\n", this, callbackCount, hop, irCountersCurrent.count, position0, position1);
#endif

            
            while (hop != 0)
            {
                int divisionsRemaining = irCountersCurrent.count >= divisionRatio1
                                       ? (irCountersCurrent.read - irCountersCurrent.written) - 1
                                       : (int) ((SampleType) ((irCountersCurrent.count * (irCountersCurrent.read - 1)) / (SampleType) (divisionRatio1)) - irCountersCurrent.written);
                
                const int nextProcessBufferDivision = processCountersCurrent.previousDivision >= numProcessDivisions ? 0 : processCountersCurrent.previousDivision;
                processCountersCurrent.previousDivision = nextProcessBufferDivision + divisionsRemaining;
                
                if (processCountersCurrent.previousDivision > numProcessDivisions)
                {
                    processCountersCurrent.previousDivision = numProcessDivisions;
                    divisionsRemaining = processCountersCurrent.previousDivision - nextProcessBufferDivision;
                }
                else
                {
                    hop = 0;
                }
                
#if PLONK_DEBUG_CONVOLVE
                printf ("convolve: %p(%d) divisionsRemaining=%d nextProcessBufferDivision=%d previousProcessBufferDivision=%d writtenIRDivisions=%d\n", this, callbackCount, divisionsRemaining, nextProcessBufferDivision, processCountersCurrent.previousDivision, irCountersCurrent.written);
#endif
                
                if (divisionsRemaining > 0)
                {
                    const SampleType* irSamples            = irBufferBase + (irCountersCurrent.written + 1) * fftSize;
                    const SampleType* processBufferSamples = processBufferBase + nextProcessBufferDivision * fftSize;
                    
                    for (int i = 0; i < divisionsRemaining; i++)
                    {
                        complexMultiplyAccumulate (fftBufferCurrent, processBufferSamples, irSamples, fftSizeHalved);
                        
                        processBufferSamples += fftSize;
                        irSamples          += fftSize;
                    }
                    
                    irCountersCurrent.written += divisionsRemaining;
                }
            }
            
            if (countDown == 0)
            {
#if PLONK_DEBUG_CONVOLVE
                printf ("convolve: %p(%d) ### OVERLAP SAVE ###\n", this, callbackCount);
#endif

                irCountersCurrent.read = plonk::min (irCountersCurrent.read + 1, numIRDivisions);
                
#if PLONK_DEBUG_CONVOLVE
                printf ("convolve: %p(%d) countDown=0 divisionsRead=%d currentProcessBuffersDivision=%d\n", this, callbackCount, irCountersCurrent.read, processCountersCurrent.currentDivision);
#endif
                
                const SampleType* const irSamples      = irBufferBase;
                SampleType* const processBufferSamples = processBufferBase + processCountersCurrent.currentDivision * fftSize;
                
                fftEngine.forward (processBufferSamples, fftAltBuffers[fftAltSelect]);
                complexMultiplyAccumulate (fftBufferCurrent, processBufferSamples, irSamples, fftSizeHalved);
                
                fftEngine.inverse (fftTransformBuffer, fftBufferCurrent);
                
                hop = fftSizeHalved;
                SampleType* const overlap1 = fftOverlapBuffer + (hop * (1 - fftAltSelect));
                SampleType* const overlap2 = fftOverlapBuffer + (hop * fftAltSelect);
                
                moveSamples (overlap1, fftTransformBuffer, hop);
                accumulateSamples (overlap2, fftTransformBuffer + hop, fftSize - hop);
                
                if (fftAltSelect != 0)
                {
                    position0    = 0;
                    fftAltSelect = 0;
                }
                else
                {
                    position1    = 0;
                    fftAltSelect = 1;
                }
                
                processCountersCurrent.previousDivision = processCountersCurrent.currentDivision;
                processCountersCurrent.currentDivision  = plonk::wrap (processCountersCurrent.currentDivision - 1, 0, numProcessDivisions);
                irCountersCurrent.count                 = 0;
                irCountersCurrent.written               = 0;
                countDown                               = hop;
                
#if PLONK_DEBUG_CONVOLVE
                printf ("convolve: %p(%d) previousProcessBufferDivision=%d currentProcessBuffersDivision=%d countDown=%d fftAltSelect=%d\n",
                        this, callbackCount, processCountersCurrent.previousDivision, processCountersCurrent.currentDivision, countDown, fftAltSelect);
#endif
                
                zeroSamples (fftBufferCurrent, fftSize);
            }
        }
        
#if PLONK_DEBUG_CONVOLVE
        printf ("convolve: %p(%d) end \n", this, callbackCount);
#endif

    }
    
//    void processChanged (ProcessInfo& info, const int channel, FFTBuffersType& irBuffers, FFTEngineType& fftEngine) throw()
//    {
//        if (previousIRBuffers == dummyIRBuffers)
//        {
//            previousIRBuffers = currentIRBuffers;
//            currentIRBuffers = irBuffers;
//            
//            // when we have completed the switch we can set previousIRBuffers back to dummyIRBuffers
//            
//            const int numProcessDivisionsCurrent  = currentIRBuffers.getNumProcessDivisions (channel);
//            const int numProcessDivisionsPrevious = previousIRBuffers.getNumProcessDivisions (channel);
//
//            if (numProcessDivisionsCurrent > numProcessDivisionsPrevious)
//            {
//#if PLONK_DEBUG_CONVOLVE
//                printf ("convolve: %p(%d) processChanged - new IR is longer\n", this, callbackCount);
//#endif
//                Buffer& currentProcessBuffer (currentIRBuffers.getProcessBuffer (channel));
//                Buffer& previousProcessBuffer (previousIRBuffers.getProcessBuffer (channel));
//                
//                // copy data (should amortise this) from the previous process buffer...
//                moveSamples (currentProcessBuffer.getArray(),
//                             previousProcessBuffer.getArray(),
//                             previousProcessBuffer.length());
//                
//                // ...and zero the rest
//                zeroSamples (currentProcessBuffer.getArray() + previousProcessBuffer.length(),
//                             currentProcessBuffer.length() - previousProcessBuffer.length());
//            }
//            else
//            {
//#if PLONK_DEBUG_CONVOLVE
//                printf ("convolve: %p(%d) processChanged - new IR is same length or shorter\n", this, callbackCount);
//#endif
//
//                // just keep the old process buffer
//                currentIRBuffers.getProcessBuffer (channel) = previousIRBuffers.getProcessBuffer (channel);
//            }
//            
//            irCountersPrevious = irCountersCurrent;
//            irCountersCurrent.reset();
//            
//            processCountersPrevious = processCountersCurrent;
////            processCountersCurrent.zero();
//        }
//        
//        // use both buffers to cross fade... but how!!!???
//        
//        /*
//         
//         - copy the input buffer input into the larger of the two process buffers
//         - some how mark the divisionFlags for each buffer with info that helps use know when the switch is complete.
//         
//         */
//        
//        UnitType& inputUnit (this->getInputAsUnit (IOKey::Generic));
//        const Buffer& inputBuffer (inputUnit.process (info, channel));
//        const SampleType* inputSamples = inputBuffer.getArray();
//        
//        SampleType* outputSamples             = this->getOutputSamples();
//        const UnsignedLong outputBufferLength = (UnsignedLong) this->getOutputBuffer().length();
//        
//        plonk_assert (outputBufferLength == inputBuffer.length());
//        
//        const int numCurrentIRDivisions  = currentIRBuffers.getNumIRDivisions();
//        const int numPreviousIRDivisions = previousIRBuffers.getNumIRDivisions();
//        const int numProcessDivisions    = currentIRBuffers.getNumProcessDivisions (channel);
//        
//        if (numCurrentIRDivisions == 0 || numPreviousIRDivisions == 0)
//        {
//            plonk_assertfalse;
//            zeroSamples (outputSamples, outputBufferLength);
//            return;
//        }
//        
//        const UnsignedLong fftSize                   = (UnsignedLong) fftEngine.length();
//        const UnsignedLong fftSizeHalved             = (UnsignedLong) fftEngine.halfLength();
//        const int divisionRatio1                     = (fftSizeHalved / outputBufferLength) - 1;
//        SampleType* const fftAltBuffers[]            = { fftAltBuffer0, fftAltBuffer1 };
//        SampleType* const processBufferBase          = currentIRBuffers.getProcessDivision (channel, 0);
//        const SampleType* const irCurrentBufferBase  = currentIRBuffers.getIRDivision (channel, 0);
//        const SampleType* const irPreviousBufferBase = previousIRBuffers.getIRDivision (channel, 0);
//        int samplesRemaining                         = outputBufferLength;
//        
//        while (samplesRemaining > 0)
//        {
//            int hop;
//            
//            if ((samplesRemaining - countDown) > 0)
//            {
//                hop = countDown;
//                samplesRemaining -= countDown;
//                countDown = 0;
//            }
//            else
//            {
//                hop = samplesRemaining;
//                countDown -= samplesRemaining;
//                samplesRemaining = 0;
//            }
//            
//            if (hop > 0)
//            {
//                moveSamples (fftAltBuffer0 + position0, inputSamples, hop);
//                moveSamples (fftAltBuffer1 + position1, inputSamples, hop);
//                moveSamples (outputSamples, fftOverlapBuffer + position0, hop);
//                
//                inputSamples  += hop;
//                outputSamples += hop;
//                position0     += hop;
//                position1     += hop;
//            }
//            
//            hop = (irCountersPrevious.written >= irCountersPrevious.read - 1) ? 0 : 1;
//            ++irCountersPrevious.count;
//            
//            while (hop != 0)
//            {
//                int previousDivisionsRemaining = irCountersPrevious.count >= divisionRatio1
//                                               ? (irCountersPrevious.read - irCountersPrevious.written) - 1
//                                               : (int) ((SampleType) ((irCountersPrevious.count * (irCountersPrevious.read - 1)) / (SampleType) (divisionRatio1)) - irCountersPrevious.written);
//                
//                const int nextProcessBufferDivision = processCountersPrevious.previousDivision >= numProcessDivisions ? 0 : processCountersPrevious.previousDivision;
//                processCountersPrevious.previousDivision = nextProcessBufferDivision + previousDivisionsRemaining;
//                
//                if (processCountersPrevious.previousDivision > numProcessDivisions)
//                {
//                    processCountersPrevious.previousDivision = numProcessDivisions;
//                    previousDivisionsRemaining = processCountersPrevious.previousDivision - nextProcessBufferDivision;
//                }
//                else
//                {
//                    hop = 0;
//                }
//                
//                if (previousDivisionsRemaining > 0)
//                {
//                    const SampleType* irSamples            = irPreviousBufferBase + (irCountersPrevious.written + 1) * fftSize;
//                    const SampleType* processBufferSamples = processBufferBase + nextProcessBufferDivision * fftSize;
//                    
//                    for (int i = 0; i < previousDivisionsRemaining; i++)
//                    {
//                        complexMultiplyAccumulate (fftBufferPrevious, processBufferSamples, irSamples, fftSizeHalved);
//                        
//                        processBufferSamples += fftSize;
//                        irSamples          += fftSize;
//                    }
//                    
//                    irCountersPrevious.written += previousDivisionsRemaining;
//                }
//            }
//            
//            hop = (irCountersCurrent.written >= irCountersCurrent.read - 1) ? 0 : 1;
//            ++irCountersCurrent.count;
//            
//            while (hop != 0)
//            {
//                int currentDivisionsRemaining = irCountersCurrent.count >= divisionRatio1
//                                              ? (irCountersCurrent.read - irCountersCurrent.written) - 1
//                                              : (int) ((SampleType) ((irCountersCurrent.count * (irCountersCurrent.read - 1)) / (SampleType) (divisionRatio1)) - irCountersCurrent.written);
//                
//                const int nextProcessBufferDivision = processCountersCurrent.previousDivision >= numProcessDivisions ? 0 : processCountersCurrent.previousDivision;
//                processCountersCurrent.previousDivision = nextProcessBufferDivision + currentDivisionsRemaining;
//                
//                if (processCountersCurrent.previousDivision > numProcessDivisions)
//                {
//                    processCountersCurrent.previousDivision = numProcessDivisions;
//                    currentDivisionsRemaining = processCountersCurrent.previousDivision - nextProcessBufferDivision;
//                }
//                else
//                {
//                    hop = 0;
//                }
//                
//                if (currentDivisionsRemaining > 0)
//                {
//                    const SampleType* irSamples            = irCurrentBufferBase + (irCountersCurrent.written + 1) * fftSize;
//                    const SampleType* processBufferSamples = processBufferBase + nextProcessBufferDivision * fftSize;
//                    
//                    for (int i = 0; i < currentDivisionsRemaining; i++)
//                    {
//                        complexMultiplyAccumulate (fftBufferCurrent, processBufferSamples, irSamples, fftSizeHalved);
//                        
//                        processBufferSamples += fftSize;
//                        irSamples            += fftSize;
//                    }
//                    
//                    irCountersCurrent.written += currentDivisionsRemaining;
//                }
//            }
//            
////            if (countDown == 0)
////            {
////                irCountersCurrent.read = plonk::min (irCountersCurrent.read + 1, numCurrentIRDivisions);
////                irCountersPrevious.read = plonk::min (irCountersPrevious.read + 1, numPreviousIRDivisions);
////                
////                const SampleType* const irCurrentSamples    = irCurrentBufferBase;
////                const SampleType* const irPreviousSamples   = irPreviousBufferBase;
////
////                SampleType* const processBufferSamplesCurrrent = processBufferBase + processCountersCurrent.currentDivision * fftSize;
////                SampleType* const processBufferSamplesPrevious = processBufferBase + processCountersPrevious.currentDivision * fftSize;
////                
////                fftEngine.forward (processBufferSamplesCurrrent, fftAltBuffers[fftAltSelect]);
////                complexMultiplyAccumulate (fftBufferCurrent, processBufferSamplesCurrrent, irCurrentSamples, fftSizeHalved);
////                fftEngine.inverse (fftTransformBuffer, fftBufferCurrent);
////                
////                fftEngine.forward (processBufferSamplesPrevious, fftAltBuffers[fftAltSelect]);
////                complexMultiplyAccumulate (fftBufferCurrent, processBufferSamplesPrevious, irPreviousSamples, fftSizeHalved);
////                fftEngine.inverse (fftTransformBuffer, fftBufferPrevious);
////
////                
////                hop = fftSizeHalved;
////                SampleType* const overlap1 = fftOverlapBuffer + (hop * (1 - fftAltSelect));
////                SampleType* const overlap2 = fftOverlapBuffer + (hop * fftAltSelect);
////                
////                moveSamples (overlap1, fftTransformBuffer, fftSize);
////                accumulateSamples (overlap2, fftTransformBuffer + hop, fftSize);
////                
////                if (fftAltSelect != 0)
////                {
////                    position0    = 0;
////                    fftAltSelect = 0;
////                }
////                else
////                {
////                    position1    = 0;
////                    fftAltSelect = 1;
////                }
////                
////                processCountersCurrent.previousDivision = processCountersCurrent.currentDivision;
////                processCountersCurrent.currentDivision = plonk::wrap (processCountersCurrent.currentDivision - 1, 0, numProcessDivisions);
////                
////                processCountersPrevious.previousDivision = processCountersPrevious.currentDivision;
////                processCountersPrevious.currentDivision = plonk::wrap (processCountersPrevious.currentDivision - 1, 0, numProcessDivisions);
////
////                irCountersCurrent.count     = 0;
////                irCountersCurrent.written   = 0;
////                irCountersPrevious.count    = 0;
////                irCountersPrevious.written  = 0;
////
////                countDown                   = hop;
////                zeroSamples (fftBufferCurrent, fftSize);
////            }
//        }
//    }
    
    void process (ProcessInfo& info, const int channel) throw()
    {
        FFTBuffersAccessType irBuffers (this->getInputAsFFTBuffers (IOKey::FFTBuffers).getValue());
        FFTEngineType& fftEngine (irBuffers.getFFTEngine());
        
        if (previousIRBuffers == dummyIRBuffers && irBuffers == currentIRBuffers)
        {
            processContinue (info, channel, irBuffers, fftEngine);
        }
        else if (fftEngine.length() != currentIRBuffers.getFFTEngine().length())
        {
            plonk_assertfalse; // new FFT buffers need to use the same FFT size as the old one
            processContinue (info, channel, currentIRBuffers, currentIRBuffers.getFFTEngine());
        }
        else
        {
//            processChanged (info, channel, irBuffers, fftEngine);
            processContinue (info, channel, irBuffers, fftEngine);
        }
        
#if PLONK_DEBUG_CONVOLVE
        ++callbackCount;
#endif
    }

private:
    struct ProcessDivisionCounters
    {
        ProcessDivisionCounters() throw()
        :   currentDivision (0), previousDivision (0)
        {
        }
        
        void zero() throw()
        {
            currentDivision  = 0;
            previousDivision = 0;
        }
        
        int currentDivision;
        int previousDivision;
    };
    
    struct IRDivisionsCounters
    {
        IRDivisionsCounters() throw()
        :   written (0), read (0), count (0)
        {
        }
        
        void zero() throw()
        {
            memset (this, 0, sizeof (IRDivisionsCounters));
        }
        
        void reset() throw()
        {
            written = 0;
            read    = 1;
            count   = 0;
        }
        
        int written;             // divisionsWritten
        int read;                // divisionsRead
        int count;               // divisionsCounter
    };
    
    
    Buffer processBuffers;
    
    SampleType* fftAltBuffer0;
    SampleType* fftAltBuffer1;
    SampleType* fftTransformBuffer;
    SampleType* fftOverlapBuffer;
    SampleType* fftBufferCurrent;
    SampleType* fftBufferPrevious;
    SampleType* fftCalcBuffer;
    
    ProcessDivisionCounters processCountersCurrent;
    ProcessDivisionCounters processCountersPrevious;
    IRDivisionsCounters irCountersCurrent;
    IRDivisionsCounters irCountersPrevious;
    
    int countDown;
    int position0, position1;
    int fftAltSelect;
    
    FFTBuffers dummyIRBuffers;
    FFTBuffersAccessType previousIRBuffers;
    FFTBuffersAccessType currentIRBuffers;
    
#if PLONK_DEBUG_CONVOLVE
    int callbackCount = { 0 };
#endif

};



//------------------------------------------------------------------------------

/** Convolve Unit.
 
 @par Factory functions:
 - ar (input, fftBuffers)
 
 @par Inputs:
 - input: (unit, multi) the unit to convolve
 - fftBuffers: (fft-buffers) the impulse response

 
 @ingroup FFTUnits */
template<class SampleType, typename FFTBuffersAccess = FFTBuffersBase<SampleType>&>
class ConvolveUnit
{
public:
    typedef ConvolveChannelInternal<SampleType,FFTBuffersAccess>    ConvolveInternal;
    typedef typename ConvolveInternal::Data                         Data;
    typedef UnitBase<SampleType>                                    UnitType;
    typedef InputDictionary                                         Inputs;
    typedef FFTBuffersBase<SampleType>                              FFTBuffersType;
    typedef Variable<FFTBuffersType&>                               FFTBuffersVariableType;

    typedef ConvolveUnit<SampleType,FFTBuffersType>                 Dyn;
    
    static PLONK_INLINE_LOW UnitInfos getInfo() throw()
    {
        return UnitInfo ("Convolve", "Convolve with an impulse response.",
                         
                         // output
                         ChannelCount::VariableChannelCount,
                         IOKey::Generic,            Measure::None,      0.0,            IOLimit::None,
                         IOKey::End,

                         // inputs
                         IOKey::Generic,            Measure::None,
                         IOKey::FFTBuffers,         Measure::FFTPacked,
                         IOKey::End);
    }
    
    static PLONK_INLINE_LOW UnitType ar (UnitType const& input, FFTBuffersVariableType const& fftBuffers) throw()
    {
        Inputs inputs;
        inputs.put (IOKey::Generic, input);
        inputs.put (IOKey::FFTBuffers, fftBuffers);
        
        Data data = { -1.0, -1.0 };
        
        return UnitType::template createFromInputs<ConvolveInternal> (inputs,
                                                                      data,
                                                                      BlockSize::noPreference(),
                                                                      SampleRate::noPreference());
    }
};


typedef ConvolveUnit<PLONK_TYPE_DEFAULT> Convolve;

//#if PLONK_INSTANTIATE_TEMPLATES
template class ConvolveUnit<Float>;
//#endif


#endif // PLONK_CONVOLVECHANNEL_H


