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

#include "../core/plank_StandardHeader.h"
#include "plank_FFT.h"
#include "../maths/vectors/plank_Vectors.h"

// undefine vDSP unless on Mac or iOS
#if !defined(PLANK_MAC) && !defined(PLANK_IOS)
    #ifdef PLANK_FFT_VDSP
    #undef PLANK_FFT_VDSP
    #endif
#endif

#ifdef PLANK_FFT_VDSP
    #ifdef PLANK_FFT_CUSTOM
    #undef PLANK_FFT_CUSTOM
    #endif

    #ifdef PLANK_FFT_OOURA
    #undef PLANK_FFT_OOURA
    #endif

    #ifdef PLANK_FFT_PFFFT
    #undef PLANK_FFT_PFFFT
    #endif

    typedef struct PLANK_FFT_VDSP_Point
    {
        int evil;
    } PLANK_FFT_VDSP_Point;

    #define Point PLANK_FFT_VDSP_Point
    #define Component PLANK_FFT_VDSP_Component
    #include <Accelerate/Accelerate.h>
    #undef Point
    #undef Component
    #else
    #include "fftreal/plank_FFTRealInternal.h"
#endif


#ifdef PLANK_FFT_PFFFT
    #ifdef PLANK_FFT_OOURA
    #undef PLANK_FFT_OOURA
    #endif

    #include "../../ext/pffft/pffft.c"
#endif

#ifdef PLANK_FFT_OOURA
    #include "../../ext/ooura/ooura.c"
#endif

#if !DOXYGEN
typedef struct PlankFFTF
{
    void* peer;
    PlankL length;
    PlankL halfLength;
    PlankL lengthLog2;
    float fftScale;
    float ifftScale;
    float* buffer;
#ifdef PLANK_FFT_VDSP
    DSPSplitComplex bufferComplex;
#endif
} PlankFFTF;
#endif

const char* pl_FFTF_GetInternalEngineName()
{
#if defined(PLANK_FFT_VDSP)
    return "vDSP";
#elif defined(PLANK_FFT_PFFFT)
    return "PFFFT";
#elif defined(PLANK_FFT_OOURA)
    return "OURA";
#elif defined(PLANK_FFT_CUSTOM)
    return pl_FFTCustomF_GetName();
#else
    return "FFTReal";
#endif
}

PlankFFTFRef pl_FFTF_CreateAndInit()
{
    PlankFFTFRef p;
    p = pl_FFTF_Create();
    
    if (p != PLANK_NULL)
    {
        if (pl_FFTF_Init (p) != PlankResult_OK)
            pl_FFTF_Destroy (p);
        else
            return p;
    }
    
    return (PlankFFTFRef)PLANK_NULL;
}

PlankFFTFRef pl_FFTF_Create()
{
    PlankMemoryRef m;
    PlankFFTFRef p;
    
    m = pl_MemoryGlobal();
    p = (PlankFFTFRef)pl_Memory_AllocateBytes (m, sizeof (PlankFFTF));
    
    if (p != NULL)
        pl_MemoryZero (p, sizeof (PlankFFTF));
    
    return p;
}

PlankResult pl_FFTF_Init (PlankFFTFRef p)
{
    return pl_FFTF_InitWithLength (p, 0);
}

PlankResult pl_FFTF_InitWithLength (PlankFFTFRef p, const PlankL length)
{
    PlankResult result = PlankResult_OK;
    PlankMemoryRef m;
    m = pl_MemoryGlobal();
    
    if (p == PLANK_NULL)
    {
        result = PlankResult_MemoryError;
        goto exit;
    }
    
    p->buffer = PLANK_NULL;
    p->length = length;
    
    if (p->length <= 0)
        p->length = PLANKFFTF_DEFAULTLENGTH;
    else if (p->length < 16)
        p->length = (PlankL)1 << p->length; // less than 16 use it as a power of 2
    
    p->halfLength = p->length / 2;
    
    p->lengthLog2 = 4;
    while (((PlankL)1 << p->lengthLog2) < p->length)
        PLANK_INC (p->lengthLog2);
    
#if defined(PLANK_FFT_VDSP)
    p->peer = vDSP_create_fftsetup (p->lengthLog2, 0);
    p->ifftScale = 1.0f / p->length;
    p->fftScale = 0.5f;
#elif defined(PLANK_FFT_PFFFT)
    p->peer = pffft_new_setup ((int)p->length, PFFFT_REAL);
    p->ifftScale = 1.0f / (int)p->length;
    p->fftScale = 1.0f;
#elif defined(PLANK_FFT_OOURA)
    p->ifftScale = 2.0f / (int)p->length;
    p->fftScale = 1.0f;
    p->peer = ooura_create (p->length);
#elif defined(PLANK_FFT_CUSTOM)
    p->peer = pl_FFTCustomF_CreateAndInitWithLength ((int)p->length, &p->fftScale, &p->ifftScale);
#else
    p->peer = pl_FFTRealF_CreateAndInitWithLength (p->length);
    p->ifftScale = 1.0f / (int)p->length;
    p->fftScale = 1.0f;
#endif
    
    if (p->peer == PLANK_NULL)
    {
        result = PlankResult_MemoryError;
        goto exit;
    }
    
    p->buffer = (float*)pl_Memory_AllocateBytes (m, sizeof (float) * p->length);
    
    if (p->buffer == PLANK_NULL)
    {
        result = PlankResult_MemoryError;
        goto exit;
    }
    
#if defined(PLANK_FFT_VDSP)
    p->bufferComplex.realp = p->buffer;
    p->bufferComplex.imagp = p->buffer + p->halfLength;
#endif
    
exit:
    return result;
}

PlankResult pl_FFTF_DeInit (PlankFFTFRef p)
{
    PlankResult result = PlankResult_OK;
    PlankMemoryRef m;
    m = pl_MemoryGlobal();
    
    if (p->peer == PLANK_NULL)
    {
        result = PlankResult_MemoryError;
        goto exit;
    }
    
#if defined(PLANK_FFT_VDSP)
    vDSP_destroy_fftsetup ((FFTSetup)p->peer);
#elif defined(PLANK_FFT_PFFFT)
    pffft_destroy_setup ((PFFFT_Setup*)p->peer);
#elif defined(PLANK_FFT_OOURA)
    ooura_destroy((OouraFFT*)p->peer);
#elif defined(PLANK_FFT_CUSTOM)
    pl_FFTCustomF_Destroy (p->peer);
#else
    pl_FFTRealF_Destroy (p->peer);
#endif
    
    p->peer = PLANK_NULL;
    result = pl_Memory_Free (m, p->buffer);
    
    pl_MemoryZero (p, sizeof (PlankFFTF));
    
exit:
    return result;
}

PlankResult pl_FFTF_Destroy (PlankFFTFRef p)
{
    PlankResult result;
    PlankMemoryRef m;
    
    result = PlankResult_OK;
    m = pl_MemoryGlobal();
    
    if ((result = pl_FFTF_DeInit (p)) != PlankResult_OK)
        goto exit;
    
    result = pl_Memory_Free (m, p);
    
exit:
    return result;
}

#if defined (PLANK_FFT_OOURA) || defined (PLANK_FFT_PFFFT)
static void pl_FFT_Pack (int halfLength, float* packedOutput, const float* interleavedInput)
{
    const float* input;
    float* real;
    float* imag;
    
    input = interleavedInput;
    real = packedOutput;
    imag = packedOutput + halfLength;
    
    while (halfLength--)
    {
        *real++ = *input++;
        *imag++ = *input++;
    }
}

static void pl_FFT_Unpack (int halfLength, float* interleavedOutput, const float* packedInput)
{
    float* output;
    const float* real;
    const float* imag;
    
    output = interleavedOutput;
    real = packedInput;
    imag = packedInput + halfLength;
    
    while (halfLength--)
    {
        *output++ = *real++;
        *output++ = *imag++;
    }
}
#endif // defined (PLANK_FFT_OOURA) || defined (PLANK_FFT_PFFFT)

void pl_FFTF_Forward (PlankFFTFRef p, float* output, const float* input)
{
#if defined(PLANK_FFT_VDSP)
    const PlankL N = p->length;
    const float scale = p->fftScale;

    FFTSetup fftvDSP = (FFTSetup)p->peer;
    const PlankL N2 = p->halfLength;
    const PlankL Nlog2 = p->lengthLog2;
    float* buffer = p->buffer;
    
    DSPSplitComplex outputComplex;
    outputComplex.realp = output;
    outputComplex.imagp = output + N2;
    
    if (scale != 1.0f)
        pl_VectorMulF_NN1 (buffer, input, scale, N);
    
    vDSP_ctoz ((DSPComplex*)buffer, 2, &outputComplex, 1, N2);
    vDSP_fft_zrip (fftvDSP, &outputComplex, 1, Nlog2, FFT_FORWARD);
    
   #ifdef PLANK_FFT_VDSP_FLIPIMAG
    float* flip = output + N2;
    float nyquist = flip[0];
    
    pl_VectorNegF_NN (flip, flip, N2);
    
    flip[0] = nyquist;
   #endif
#elif defined(PLANK_FFT_PFFFT)
    pffft_transform ((PFFFT_Setup*)p->peer, input, output, p->buffer, PFFFT_FORWARD);
    pffft_zreorder ((PFFFT_Setup*)p->peer, output, p->buffer, PFFFT_FORWARD);
    pl_FFT_Pack (p->halfLength, output, p->buffer);
#elif defined(PLANK_FFT_OOURA)
    pl_MemoryCopy (p->buffer, input, sizeof(float) * p->length);
    ooura_forward ((OouraFFT*)p->peer, p->buffer);
    pl_FFT_Pack (p->halfLength, output, p->buffer);
#elif defined(PLANK_FFT_CUSTOM)
    const PlankL N = p->length;
    const float scale = p->fftScale;

    pl_FFTCustomF_Forward (p->peer, output, input, p->buffer);

    if (scale != 1.0f)
        pl_VectorMulF_NN1 (output, output, scale, N);

#else
    pl_FFTRealF_Forward (p->peer, output, input);
#endif
}

void pl_FFTF_Inverse (PlankFFTFRef p, float* output, const float* input)
{
    const PlankL N = p->length;
    const float scale = p->ifftScale;
    
#if defined(PLANK_FFT_VDSP)
    FFTSetup fftvDSP = (FFTSetup)p->peer;
    float* buffer = p->buffer;
    DSPSplitComplex* bufferComplex = &p->bufferComplex;
    const PlankL N2 = p->halfLength;
    const PlankL Nlog2 = p->lengthLog2;
    
    pl_MemoryCopy (buffer, input, sizeof (float) * N);
    
   #ifdef PLANK_FFT_VDSP_FLIPIMAG
    float* flip = buffer + N2;
    float nyquist = flip[0];
    
    pl_VectorNegF_NN (flip, flip, N2);
    
    flip[0] = nyquist;
   #endif
    
    vDSP_fft_zrip (fftvDSP, bufferComplex, 1, Nlog2, FFT_INVERSE);
    vDSP_ztoc (bufferComplex, 1, (DSPComplex*)output, 2, N2);
#elif defined(PLANK_FFT_PFFFT)
    pl_FFT_Unpack (p->halfLength, p->buffer, input);
    pffft_zreorder ((PFFFT_Setup*)p->peer, p->buffer, output, PFFFT_BACKWARD);
    pffft_transform ((PFFFT_Setup*)p->peer, output, output, p->buffer, PFFFT_BACKWARD);
#elif defined(PLANK_FFT_OOURA)
    pl_FFT_Unpack (p->halfLength, output, input);
    ooura_inverse ((OouraFFT*)p->peer, output);
#elif defined(PLANK_FFT_CUSTOM)
    pl_FFTCustomF_Inverse (p->peer, output, input, p->buffer);
#else
    pl_MemoryCopy (p->buffer, input, sizeof (float) * N);
    pl_FFTRealF_Inverse (p->peer, output, p->buffer);
#endif
    
    if (scale != 1.0f)
        pl_VectorMulF_NN1 (output, output, scale, N);
}

PlankL pl_FFTF_Length (PlankFFTFRef p)
{
    return p->length;
}

PlankL pl_FFTF_HalfLength (PlankFFTFRef p)
{
    return p->halfLength;
}

PlankL pl_FFTF_LengthLog2 (PlankFFTFRef p)
{
    return p->lengthLog2;
}

float* pl_FFTF_Temp (PlankFFTFRef p)
{
    return p->buffer;
}

