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

#ifndef PLONK_IFFTCHANNEL_H
#define PLONK_IFFTCHANNEL_H

#include "../channel/plonk_ChannelInternalCore.h"
#include "../plonk_GraphForwardDeclarations.h"


/** IFFT channel. */
template<class SampleType>
class IFFTChannelInternal
:   public ChannelInternal<SampleType, ChannelInternalCore::Data>
{
public:
    typedef ChannelInternalCore::Data                           Data;
    typedef ChannelBase<SampleType>                             ChannelType;
    typedef IFFTChannelInternal<SampleType>                     IFFTInternal;
    typedef ChannelInternal<SampleType,Data>                    Internal;
    typedef ChannelInternalBase<SampleType>                     InternalBase;
    typedef UnitBase<SampleType>                                UnitType;
    typedef InputDictionary                                     Inputs;
    typedef NumericalArray<SampleType>                          Buffer;
    typedef FFTEngineBase<SampleType>                           FFTEngineType;
    
    IFFTChannelInternal (Inputs const& inputs,
                         Data const& data,
                         BlockSize const& blockSize,
                         SampleRate const& sampleRate) throw()
    :   Internal (inputs, data, blockSize, sampleRate)
    {
    }
    
    Text getName() const throw()
    {
        return "IFFT";
    }
    
    IntArray getInputKeys() const throw()
    {
        const IntArray keys (IOKey::FFTPacked);
        return keys;
    }
    
    InternalBase* getChannel (const int index) throw()
    {
        const Inputs channelInputs = this->getInputs().getChannel (index);
        return new IFFTInternal (channelInputs,
                                 this->getState(),
                                 this->getBlockSize(),
                                 this->getSampleRate());
    }
    
    void initChannel (const int channel) throw()
    {
        const UnitType& input = this->getInputAsUnit (IOKey::FFTPacked);
                
        this->setBlockSize (input.getBlockSize (channel));
        this->setSampleRate (input.getSampleRate (channel));
        this->setOverlap (input.getOverlap (channel));
        
        this->fft = FFTEngineType (input.getBlockSize (channel));
        
        this->initValue (SampleType (0)); // impossible to precalculate
    }
    
    void process (ProcessInfo& info, const int channel) throw()
    {
        /* Be careful optimising this with the new NumericalArray vector stuff */
                
        UnitType& inputUnit (this->getInputAsUnit (IOKey::FFTPacked));
        const Buffer& inputBuffer (inputUnit.process (info, channel));
        const SampleType* const inputSamples = inputBuffer.getArray();
        
        SampleType* const outputSamples = this->getOutputSamples();
        const int outputBufferLength = this->getOutputBuffer().length();
        
        plonk_assert (outputBufferLength == inputBuffer.length());
        
        if (outputBufferLength != this->fft.length())
            this->fft = FFTEngineType (outputBufferLength);
            
        this->fft.inverse (outputSamples, inputSamples);
    }
    
private:
    FFTEngineType fft;
};



//------------------------------------------------------------------------------

/** IFFT Unit.
 Takes frequency domain signal in real/imaginary format and outputs the time domain real signal.
 
 Each pair of imput channels generates one output channel. The even numbered channels should conatin
 the real data and the odd numbered channel should contain the imaginary data. (This is the same
 output format as the FFTUnit.)
 
 @par Factory functions:
 - ar (input)
 
 @par Inputs:
 - input: (unit, multi, fft) the input unit in FFT format
 
 
 @ingroup ConverterUnits FFTUnits */
template<class SampleType>
class IFFTUnit
{
public:
    typedef IFFTChannelInternal<SampleType>         IFFTInternal;
    typedef typename IFFTInternal::Data             Data;
    typedef ChannelBase<SampleType>                 ChannelType;
    typedef ChannelInternal<SampleType,Data>        Internal;
    typedef ChannelInternalBase<SampleType>         InternalBase;
    typedef UnitBase<SampleType>                    UnitType;
    typedef InputDictionary                         Inputs;
    typedef NumericalArray<SampleType>              Buffer;
    
    static PLONK_INLINE_LOW UnitInfos getInfo() throw()
    {
        return UnitInfo ("IFFT", "Transforms  frequency domain data to a time domain signal.",
                         
                         // output
                         ChannelCount::VariableChannelCount,
                         IOKey::Generic,       Measure::None,        IOInfo::NoDefault,  IOLimit::None,
                         IOKey::End,
                         
                         // inputs
                         IOKey::FFTPacked,     Measure::FFTPacked,   IOInfo::NoDefault,  IOLimit::None,
                         IOKey::End);
    }
        
    static PLONK_INLINE_LOW UnitType ar (UnitType const& input) throw()
    {
        // re: full templating - could say that FFT/IFFT are only supported with float (and eventually double)?
        
        Inputs inputs;
        inputs.put (IOKey::FFTPacked, input);
        
        Data data = { -1.0, -1.0 };
        
        return UnitType::template createFromInputs<IFFTInternal> (inputs,
                                                                  data,
                                                                  BlockSize::noPreference(),
                                                                  SampleRate::noPreference());
    }

};


typedef IFFTUnit<PLONK_TYPE_DEFAULT> IFFT;

#endif // PLONK_IFFTCHANNEL_H


