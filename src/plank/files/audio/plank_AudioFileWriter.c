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

#include "../../core/plank_StandardHeader.h"
#include "../plank_File.h"
#include "../plank_IffFileWriter.h"
#include "../../maths/plank_Maths.h"
#include "../../random/plank_RNG.h"
#include "plank_AudioFileWriter.h"
#include "plank_AudioFileMetaData.h"
#include "plank_AudioFileCuePoint.h"
#include "plank_AudioFileRegion.h"

#define PLANKAUDIOFILEWRITER_BUFFERLENGTH 256

// private

typedef PlankResult (*PlankAudioFileWriterWriteHeaderFunction)(PlankAudioFileWriterRef);
typedef PlankResult (*PlankAudioFileWriterWriteFramesFunction)(PlankAudioFileWriterRef, const int, const void*);

PlankResult pl_AudioFileWriter_WAV_Open (PlankAudioFileWriterRef p, const char* filepath);
PlankResult pl_AudioFileWriter_WAV_OpenWithFile (PlankAudioFileWriterRef p, PlankFileRef file);
PlankResult pl_AudioFileWriter_WAV_WriteHeader (PlankAudioFileWriterRef p);
PlankResult pl_AudioFileWriter_WAVEXT_WriteHeader (PlankAudioFileWriterRef p);
PlankResult pl_AudioFileWriter_WAVRF64_WriteHeader (PlankAudioFileWriterRef p);
PlankResult pl_AudioFileWriter_WAV_WriteFrames (PlankAudioFileWriterRef p, const int numFrames, const void* data);

PlankResult pl_AudioFileWriter_AIFF_Open (PlankAudioFileWriterRef p, const char* filepath);
PlankResult pl_AudioFileWriter_AIFF_OpenWithFile (PlankAudioFileWriterRef p, PlankFileRef file);
PlankResult pl_AudioFileWriter_AIFF_WriteHeader (PlankAudioFileWriterRef p);
PlankResult pl_AudioFileWriter_AIFF_WriteFrames (PlankAudioFileWriterRef p, const int numFrames, const void* data);

PlankResult pl_AudioFileWriter_AIFC_Open (PlankAudioFileWriterRef p, const char* filepath);
PlankResult pl_AudioFileWriter_AIFC_OpenWithFile (PlankAudioFileWriterRef p, PlankFileRef file);
PlankResult pl_AudioFileWriter_AIFC_WriteHeader (PlankAudioFileWriterRef p);
PlankResult pl_AudioFileWriter_AIFC_WriteFrames (PlankAudioFileWriterRef p, const int numFrames, const void* data);

PlankResult pl_AudioFileWriter_CAF_Open (PlankAudioFileWriterRef p, const char* filepath);
PlankResult pl_AudioFileWriter_CAF_OpenWithFile (PlankAudioFileWriterRef p, PlankFileRef file);
PlankResult pl_AudioFileWriter_CAF_WriteHeader (PlankAudioFileWriterRef p);
PlankResult pl_AudioFileWriter_CAF_WriteFrames (PlankAudioFileWriterRef p, const int numFrames, const void* data);

PlankResult pl_AudioFileWriter_W64_Open (PlankAudioFileWriterRef p, const char* filepath);
PlankResult pl_AudioFileWriter_W64_OpenWithFile (PlankAudioFileWriterRef p, PlankFileRef file);
PlankResult pl_AudioFileWriter_W64_WriteHeader (PlankAudioFileWriterRef p);
PlankResult pl_AudioFileWriter_W64_WriteFrames (PlankAudioFileWriterRef p, const int numFrames, const void* data);

PlankResult pl_AudioFileWriter_Iff_WriteFrames (PlankAudioFileWriterRef p, const char* chunkID, const int numFrames, const void* data);

PlankResult pl_AudioFileWriter_OggVorbis_Open (PlankAudioFileWriterRef p, const char* filepath);
PlankResult pl_AudioFileWriter_OggVorbis_OpenWithFile (PlankAudioFileWriterRef p, PlankFileRef file);
PlankResult pl_AudioFileWriter_OggVorbis_Close (PlankAudioFileWriterRef p);
PlankResult pl_AudioFileWriter_OggVorbis_WriteFrames (PlankAudioFileWriterRef p, const int numFrames, const void* data);

PlankResult pl_AudioFileWriter_Opus_Open (PlankAudioFileWriterRef p, const char* filepath);
PlankResult pl_AudioFileWriter_Opus_OpenWithFile (PlankAudioFileWriterRef p, PlankFileRef file);
PlankResult pl_AudioFileWriter_Opus_Close (PlankAudioFileWriterRef p);
PlankResult pl_AudioFileWriter_Opus_WriteFrames (PlankAudioFileWriterRef p, const int numFrames, const void* data);

PlankResult pl_AudioFileWriter_WAV_WriteMetaData (PlankAudioFileWriterRef p);
PlankResult pl_AudioFileWriter_AIFF_WriteMetaData (PlankAudioFileWriterRef p);
PlankResult pl_AudioFileWriter_AIFC_WriteMetaData (PlankAudioFileWriterRef p);
PlankResult pl_AudioFileWriter_CAF_WriteMetaData (PlankAudioFileWriterRef p);
PlankResult pl_AudioFileWriter_W64_WriteMetaData (PlankAudioFileWriterRef p);
PlankResult pl_AudioFileWriter_OggVorbis_WriteMetaData (PlankAudioFileWriterRef p);
PlankResult pl_AudioFileWriter_Opus_WriteMetaData (PlankAudioFileWriterRef p);

typedef struct PlankIffAudioFileWriter* PlankIffAudioFileWriterRef;
typedef struct PlankIffAudioFileWriter
{
    PlankIffFileWriter iff;
//    PlankDynamicArray temp;
} PlankIffAudioFileWriter;

static PlankResult pl_IffAudioFileWriter_DeInit (PlankIffAudioFileWriterRef p)
{
    PlankResult result = PlankResult_OK;
    
    if (p == PLANK_NULL)
    {
        result = PlankResult_MemoryError;
        goto exit;
    }
    
    if ((result = pl_IffFileWriter_DeInit (&p->iff)) != PlankResult_OK) goto exit;
//    if ((result = pl_DynamicArray_DeInit (&p->temp)) != PlankResult_OK) goto exit;
    
    pl_MemoryZero (p, sizeof (PlankIffAudioFileWriter));
    
exit:
    return result;
}

static PlankResult pl_IffAudioFileWriter_Destroy (PlankIffAudioFileWriterRef p)
{
    PlankResult result;
    PlankMemoryRef m;
    
    result = PlankResult_OK;
    m = pl_MemoryGlobal();
    
    if (p == PLANK_NULL)
    {
        result = PlankResult_MemoryError;
        goto exit;
    }
    
    if ((result = pl_IffAudioFileWriter_DeInit (p)) != PlankResult_OK)
        goto exit;
    
    result = pl_Memory_Free (m, p);
    
exit:
    return result;
}

static PlankIffAudioFileWriterRef pl_IffAudioFileWriter_Create()
{
    PlankMemoryRef m;
    PlankIffAudioFileWriterRef p;
    
    m = pl_MemoryGlobal();
    p = (PlankIffAudioFileWriterRef)pl_Memory_AllocateBytes (m, sizeof (PlankIffAudioFileWriter));
    
    if (p != PLANK_NULL)
        pl_MemoryZero (p, sizeof (PlankIffAudioFileWriter));
    
    return p;
}

static PlankIffAudioFileWriterRef pl_IffAudioFileWriter_CreateAndInit()
{
    PlankIffAudioFileWriterRef p;
    p = pl_IffAudioFileWriter_Create();
    
    if (p != PLANK_NULL)
    {
        if (pl_IffFileWriter_Init (&p->iff) != PlankResult_OK)
            pl_IffAudioFileWriter_Destroy (p);
        else
            return p;
    }
    
    return PLANK_NULL;
}


///---


PlankAudioFileWriterRef pl_AudioFileWriter_CreateAndInit()
{
    PlankAudioFileWriterRef p;
    p = pl_AudioFileWriter_Create();
    
    if (p != PLANK_NULL)
    {
        if (pl_AudioFileWriter_Init (p) != PlankResult_OK)
            pl_AudioFileWriter_Destroy (p);
        else
            return p;
    }
    
    return PLANK_NULL;
}

PlankAudioFileWriterRef pl_AudioFileWriter_Create()
{
    PlankMemoryRef m;
    PlankAudioFileWriterRef p;
    
    m = pl_MemoryGlobal();
    p = (PlankAudioFileWriterRef)pl_Memory_AllocateBytes (m, sizeof (PlankAudioFileWriter));
    
    if (p != PLANK_NULL)
        pl_MemoryZero (p, sizeof (PlankAudioFileWriter));
    
    return p;
}

PlankResult pl_AudioFileWriter_Init (PlankAudioFileWriterRef p)
{
    PlankResult result = PlankResult_OK;
    
    pl_MemoryZero (p, sizeof (PlankAudioFileWriter));
    
    p->peer                        = PLANK_NULL;
    p->formatInfo.format           = PLANKAUDIOFILE_FORMAT_INVALID;
    p->formatInfo.encoding         = PLANKAUDIOFILE_ENCODING_INVALID;
    p->formatInfo.bitsPerSample    = 0;
    p->formatInfo.bytesPerFrame    = 0;
    p->formatInfo.numChannels      = 0;
    p->formatInfo.sampleRate       = 0.0;
    
    p->numFrames                   = 0;
    p->dataPosition                = -1;
    p->metaDataChunkPosition       = -1;
    p->headerPad                   = 0;
    p->metaData                    = PLANK_NULL;
    
    p->writeFramesFunction         = PLANK_NULL;
    p->writeHeaderFunction         = PLANK_NULL;
    
    return result;
}

PlankResult pl_AudioFileWriter_DeInit (PlankAudioFileWriterRef p)
{
    PlankResult result = PlankResult_OK;
    
    if (p == PLANK_NULL)
    {
        result = PlankResult_MemoryError;
        goto exit;
    }
    
    if ((result = pl_AudioFileWriter_Close (p)) != PlankResult_OK) goto exit;
    
    pl_MemoryZero (p, sizeof (PlankAudioFileWriter));
    
exit:
    return result;
}

PlankResult pl_AudioFileWriter_Destroy (PlankAudioFileWriterRef p)
{
    PlankResult result = PlankResult_OK;
    PlankMemoryRef m = pl_MemoryGlobal();
    
    if (p == PLANK_NULL)
    {
        result = PlankResult_MemoryError;
        goto exit;
    }
    
    if ((result = pl_AudioFileWriter_DeInit (p)) != PlankResult_OK)
        goto exit;
    
    result = pl_Memory_Free (m, p);    
    
exit:
    return result;
}

PlankFileRef pl_AudioFileWriter_GetFile (PlankAudioFileWriterRef p)
{
    return (PlankFileRef)p->peer;
}

PlankAudioFileFormatInfo* pl_AudioFileWriter_GetFormatInfo (PlankAudioFileWriterRef p)
{
    return p->peer ? 0 : &p->formatInfo;
}

const PlankAudioFileFormatInfo* pl_AudioFileWriter_GetFormatInfoReadOnly (PlankAudioFileWriterRef p)
{
    return &p->formatInfo;
}

PlankAudioFileMetaDataRef pl_AudioFileWriter_GetMetaData (PlankAudioFileWriterRef p)
{
    if (!p->metaData)
        p->metaData = pl_AudioFileMetaData_CreateAndInit();
    
    return p->metaData;
}

PlankResult pl_AudioFileWriter_SetFormatWAV (PlankAudioFileWriterRef p, const int bitsPerSample, const int numChannels, const double sampleRate, const PlankB isFloat)
{
    if (p->peer)
        return PlankResult_UnknownError;
    
    if (isFloat)
    {
        if ((bitsPerSample != 32) && (bitsPerSample != 64))
            return PlankResult_AudioFileInavlidType;
    }
    
    p->formatInfo.format            = PLANKAUDIOFILE_FORMAT_WAV;
    p->formatInfo.encoding          = isFloat ? PLANKAUDIOFILE_ENCODING_FLOAT_LITTLEENDIAN : PLANKAUDIOFILE_ENCODING_PCM_LITTLEENDIAN;
    p->formatInfo.bitsPerSample     = bitsPerSample;
    p->formatInfo.numChannels       = numChannels;
    p->formatInfo.sampleRate        = sampleRate;
    p->formatInfo.bytesPerFrame     = (PlankI) (((bitsPerSample + (0x00000008 - 1)) & ~(0x00000008 - 1)) * numChannels / 8);
    p->formatInfo.nominalBitRate    = (int)(p->formatInfo.bytesPerFrame * p->formatInfo.sampleRate * p->formatInfo.numChannels * PLANKAUDIOFILE_CHARBITS);
    p->formatInfo.minimumBitRate    = p->formatInfo.nominalBitRate;
    p->formatInfo.maximumBitRate    = p->formatInfo.nominalBitRate;
    p->formatInfo.frameDuration     = 1.0 / p->formatInfo.sampleRate;
    p->formatInfo.quality           = p->formatInfo.bitsPerSample / 64.f;

    return PlankResult_OK;
}

PlankResult pl_AudioFileWriter_SetFormatAIFF (PlankAudioFileWriterRef p, const int bitsPerSample, const int numChannels, const double sampleRate)
{
    if (p->peer)
        return PlankResult_UnknownError;
        
    p->formatInfo.format            = PLANKAUDIOFILE_FORMAT_AIFF;
    p->formatInfo.encoding          = PLANKAUDIOFILE_ENCODING_PCM_BIGENDIAN;
    p->formatInfo.bitsPerSample     = bitsPerSample;
    p->formatInfo.numChannels       = numChannels;
    p->formatInfo.sampleRate        = sampleRate;
    p->formatInfo.bytesPerFrame     = (PlankI) (((bitsPerSample + (0x00000008 - 1)) & ~(0x00000008 - 1)) * numChannels / 8);
    p->formatInfo.nominalBitRate    = (int)(p->formatInfo.bytesPerFrame * p->formatInfo.sampleRate * p->formatInfo.numChannels * PLANKAUDIOFILE_CHARBITS);
    p->formatInfo.minimumBitRate    = p->formatInfo.nominalBitRate;
    p->formatInfo.maximumBitRate    = p->formatInfo.nominalBitRate;
    p->formatInfo.frameDuration     = 1.0 / p->formatInfo.sampleRate;
    p->formatInfo.quality           = p->formatInfo.bitsPerSample / 64.f;
    
    return PlankResult_OK;
}

PlankResult pl_AudioFileWriter_SetFormatAIFC (PlankAudioFileWriterRef p, const int bitsPerSample, const int numChannels, const double sampleRate, const PlankB isFloat, const PlankB isLittleEndian)
{
    if (p->peer)
        return PlankResult_UnknownError;
    
    if (isFloat)
    {
        if ((bitsPerSample != 32) && (bitsPerSample != 64))
            return PlankResult_AudioFileInavlidType;
        
        if (isLittleEndian)
            return PlankResult_AudioFileInavlidType;
    }
    
    p->formatInfo.format            = PLANKAUDIOFILE_FORMAT_AIFC;
    p->formatInfo.encoding          = isFloat ? PLANKAUDIOFILE_ENCODING_FLOAT_BIGENDIAN : isLittleEndian ? PLANKAUDIOFILE_ENCODING_PCM_LITTLEENDIAN : PLANKAUDIOFILE_ENCODING_PCM_BIGENDIAN;
    p->formatInfo.bitsPerSample     = bitsPerSample;
    p->formatInfo.numChannels       = numChannels;
    p->formatInfo.sampleRate        = sampleRate;
    p->formatInfo.bytesPerFrame     = (PlankI) (((bitsPerSample + (0x00000008 - 1)) & ~(0x00000008 - 1)) * numChannels / 8);
    p->formatInfo.nominalBitRate    = (int)(p->formatInfo.bytesPerFrame * p->formatInfo.sampleRate * p->formatInfo.numChannels * PLANKAUDIOFILE_CHARBITS);
    p->formatInfo.minimumBitRate    = p->formatInfo.nominalBitRate;
    p->formatInfo.maximumBitRate    = p->formatInfo.nominalBitRate;
    p->formatInfo.frameDuration     = 1.0 / p->formatInfo.sampleRate;
    p->formatInfo.quality           = p->formatInfo.bitsPerSample / 64.f;
    
    return PlankResult_OK;
}

PlankResult pl_AudioFileWriter_SetFormatCAF (PlankAudioFileWriterRef p, const int bitsPerSample, const int numChannels, const double sampleRate, const PlankB isFloat, const PlankB isLittleEndian)
{
    if (p->peer)
        return PlankResult_UnknownError;
    
    if (isFloat)
    {
        if ((bitsPerSample != 32) && (bitsPerSample != 64))
            return PlankResult_AudioFileInavlidType;        
    }
    
    p->formatInfo.format            = PLANKAUDIOFILE_FORMAT_CAF;
    p->formatInfo.encoding          = 0;
    
    if (isFloat)
        p->formatInfo.encoding     |= PLANKAUDIOFILE_ENCODING_FLOAT_FLAG;
    
    if (!isLittleEndian)
        p->formatInfo.encoding     |= PLANKAUDIOFILE_ENCODING_BIGENDIAN_FLAG;
    
    p->formatInfo.bitsPerSample     = bitsPerSample;
    p->formatInfo.numChannels       = numChannels;
    p->formatInfo.sampleRate        = sampleRate;
    p->formatInfo.bytesPerFrame     = (PlankI) (((bitsPerSample + (0x00000008 - 1)) & ~(0x00000008 - 1)) * numChannels / 8);
    p->formatInfo.nominalBitRate    = (int)(p->formatInfo.bytesPerFrame * p->formatInfo.sampleRate * p->formatInfo.numChannels * PLANKAUDIOFILE_CHARBITS);
    p->formatInfo.minimumBitRate    = p->formatInfo.nominalBitRate;
    p->formatInfo.maximumBitRate    = p->formatInfo.nominalBitRate;
    p->formatInfo.frameDuration     = 1.0 / p->formatInfo.sampleRate;
    p->formatInfo.quality           = p->formatInfo.bitsPerSample / 64.f;
    
    return PlankResult_OK;
}

PlankResult pl_AudioFileWriter_SetFormatW64 (PlankAudioFileWriterRef p, const int bitsPerSample, const int numChannels, const double sampleRate, const PlankB isFloat)
{
    if (p->peer)
        return PlankResult_UnknownError;
    
    if (isFloat)
    {
        if ((bitsPerSample != 32) && (bitsPerSample != 64))
            return PlankResult_AudioFileInavlidType;
    }
    
    p->formatInfo.format            = PLANKAUDIOFILE_FORMAT_W64;
    p->formatInfo.encoding          = isFloat ? PLANKAUDIOFILE_ENCODING_FLOAT_LITTLEENDIAN : PLANKAUDIOFILE_ENCODING_PCM_LITTLEENDIAN;
    p->formatInfo.bitsPerSample     = bitsPerSample;
    p->formatInfo.numChannels       = numChannels;
    p->formatInfo.sampleRate        = sampleRate;
    p->formatInfo.bytesPerFrame     = (PlankI) (((bitsPerSample + (0x00000008 - 1)) & ~(0x00000008 - 1)) * numChannels / 8);
    p->formatInfo.nominalBitRate    = (int)(p->formatInfo.bytesPerFrame * p->formatInfo.sampleRate * p->formatInfo.numChannels * PLANKAUDIOFILE_CHARBITS);
    p->formatInfo.minimumBitRate    = p->formatInfo.nominalBitRate;
    p->formatInfo.maximumBitRate    = p->formatInfo.nominalBitRate;
    p->formatInfo.frameDuration     = 1.0 / p->formatInfo.sampleRate;
    p->formatInfo.quality           = p->formatInfo.bitsPerSample / 64.f;
    
    return PlankResult_OK;    
}

PlankResult pl_AudioFileWriter_SetFormatOggVorbis (PlankAudioFileWriterRef p, const float quality, const int numChannels, const double sampleRate)
{
    int bitsPerSample = 32;
    
    if (p->peer)
        return PlankResult_UnknownError;
    
    p->formatInfo.format            = PLANKAUDIOFILE_FORMAT_OGGVORBIS;
    p->formatInfo.encoding          = PLANK_BIGENDIAN ? PLANKAUDIOFILE_ENCODING_FLOAT_BIGENDIAN : PLANKAUDIOFILE_ENCODING_FLOAT_LITTLEENDIAN;
    p->formatInfo.bitsPerSample     = bitsPerSample;
    p->formatInfo.numChannels       = numChannels;
    p->formatInfo.sampleRate        = sampleRate;
    p->formatInfo.bytesPerFrame     = (PlankI) (((bitsPerSample + (0x00000008 - 1)) & ~(0x00000008 - 1)) * numChannels / 8);
    p->formatInfo.nominalBitRate    = 0;
    p->formatInfo.minimumBitRate    = 0;
    p->formatInfo.maximumBitRate    = 0;
    p->formatInfo.frameDuration     = 0.f;
    p->formatInfo.quality           = pl_ClipF (quality, 0., 10.f);
    
    return PlankResult_OK;
}

PlankResult pl_AudioFileWriter_SetFormatOggVorbisManaged (PlankAudioFileWriterRef p, const int min, const int nominal, const int max, const int numChannels, const double sampleRate)
{
    int bitsPerSample = 32;

    if (p->peer)
        return PlankResult_UnknownError;
    
    if (max < min)
        return PlankResult_AudioFileInavlidType;
    

    p->formatInfo.format            = PLANKAUDIOFILE_FORMAT_OGGVORBIS;
    p->formatInfo.encoding          = PLANK_BIGENDIAN ? PLANKAUDIOFILE_ENCODING_FLOAT_BIGENDIAN : PLANKAUDIOFILE_ENCODING_FLOAT_LITTLEENDIAN;
    p->formatInfo.bitsPerSample     = bitsPerSample;
    p->formatInfo.numChannels       = numChannels;
    p->formatInfo.sampleRate        = sampleRate;
    p->formatInfo.bytesPerFrame     = (PlankI) (((bitsPerSample + (0x00000008 - 1)) & ~(0x00000008 - 1)) * numChannels / 8);
    p->formatInfo.nominalBitRate    = ((nominal < min) || (nominal > max)) ? (min + max) / 2 : nominal;
    p->formatInfo.minimumBitRate    = min;
    p->formatInfo.maximumBitRate    = max;
    p->formatInfo.frameDuration     = 0.f;
    p->formatInfo.quality           = 0.f;
    
    return PlankResult_OK;
}


PlankResult pl_AudioFileWriter_SetFormatOpus (PlankAudioFileWriterRef p, const float quality, const int numChannels, const double sampleRate, const double frameDuration)
{
    int bitsPerSample = 32;

    if (p->peer)
        return PlankResult_UnknownError;
    
    p->formatInfo.format            = PLANKAUDIOFILE_FORMAT_OPUS;
    p->formatInfo.encoding          = PLANK_BIGENDIAN ? PLANKAUDIOFILE_ENCODING_FLOAT_BIGENDIAN : PLANKAUDIOFILE_ENCODING_FLOAT_LITTLEENDIAN;
    p->formatInfo.bitsPerSample     = bitsPerSample;
    p->formatInfo.frameDuration     = frameDuration;
    p->formatInfo.numChannels       = numChannels;
    p->formatInfo.sampleRate        = sampleRate;
    p->formatInfo.bytesPerFrame     = (PlankI) (((bitsPerSample + (0x00000008 - 1)) & ~(0x00000008 - 1)) * numChannels / 8);
    p->formatInfo.nominalBitRate    = 0;
    p->formatInfo.minimumBitRate    = 0;
    p->formatInfo.maximumBitRate    = 0;
    p->formatInfo.frameDuration     = frameDuration;
    p->formatInfo.quality           = pl_ClipF (quality, 0., 10.f);

    return PlankResult_OK;
}

PlankResult pl_AudioFileWriter_SetFormatOpusManaged (PlankAudioFileWriterRef p, const int nominalBitRate, const int numChannels, const double sampleRate, const double frameDuration)
{
    int bitsPerSample = 32;

    if (p->peer)
        return PlankResult_UnknownError;
    
    p->formatInfo.format            = PLANKAUDIOFILE_FORMAT_OPUS;
    p->formatInfo.encoding          = PLANK_BIGENDIAN ? PLANKAUDIOFILE_ENCODING_FLOAT_BIGENDIAN : PLANKAUDIOFILE_ENCODING_FLOAT_LITTLEENDIAN;
    p->formatInfo.bitsPerSample     = bitsPerSample;
    p->formatInfo.frameDuration     = frameDuration;
    p->formatInfo.numChannels       = numChannels;
    p->formatInfo.sampleRate        = sampleRate;
    p->formatInfo.bytesPerFrame     = (PlankI) (((bitsPerSample + (0x00000008 - 1)) & ~(0x00000008 - 1)) * numChannels / 8);
    p->formatInfo.nominalBitRate    = nominalBitRate;
    p->formatInfo.minimumBitRate    = nominalBitRate;
    p->formatInfo.maximumBitRate    = nominalBitRate;
    p->formatInfo.frameDuration     = frameDuration;
    p->formatInfo.quality           = 0.f;
    
    return PlankResult_OK;
}

PlankResult pl_AudioFileWriter_Open (PlankAudioFileWriterRef p, const char* filepath)
{
    PlankResult result = PlankResult_OK;
    
    if (p->formatInfo.format == PLANKAUDIOFILE_FORMAT_WAV)
    {
        result = pl_AudioFileWriter_WAV_Open (p, filepath);
    }
    else if (p->formatInfo.format == PLANKAUDIOFILE_FORMAT_AIFF)
    {
        result = pl_AudioFileWriter_AIFF_Open (p, filepath);
    }
    else if (p->formatInfo.format == PLANKAUDIOFILE_FORMAT_AIFC)
    {
        result = pl_AudioFileWriter_AIFC_Open (p, filepath);
    }
    else if (p->formatInfo.format == PLANKAUDIOFILE_FORMAT_CAF)
    {
        result = pl_AudioFileWriter_CAF_Open (p, filepath);
    }
    else if (p->formatInfo.format == PLANKAUDIOFILE_FORMAT_W64)
    {
        result = pl_AudioFileWriter_W64_Open (p, filepath);
    }
#if PLANK_OGGVORBIS
    else if (p->formatInfo.format == PLANKAUDIOFILE_FORMAT_OGGVORBIS)
    {
        result = pl_AudioFileWriter_OggVorbis_Open (p, filepath);
    }
#endif
#if PLANK_OPUS
    else if (p->formatInfo.format == PLANKAUDIOFILE_FORMAT_OPUS)
    {
        result = pl_AudioFileWriter_Opus_Open (p, filepath);
    }
#endif
    else
    {
        result = PlankResult_AudioFileInavlidType;
    }
    
exit:
    return result;
}

PlankResult pl_AudioFileWriter_OpenWithFile (PlankAudioFileWriterRef p, PlankFileRef file)
{
    PlankResult result = PlankResult_OK;
    
    if (p->formatInfo.format == PLANKAUDIOFILE_FORMAT_WAV)
    {
        result = pl_AudioFileWriter_WAV_OpenWithFile (p, file);
    }
    else if (p->formatInfo.format == PLANKAUDIOFILE_FORMAT_AIFF)
    {
        result = pl_AudioFileWriter_AIFF_OpenWithFile (p, file);
    }
    else if (p->formatInfo.format == PLANKAUDIOFILE_FORMAT_AIFC)
    {
        result = pl_AudioFileWriter_AIFC_OpenWithFile (p, file);
    }
    else if (p->formatInfo.format == PLANKAUDIOFILE_FORMAT_CAF)
    {
        result = pl_AudioFileWriter_CAF_OpenWithFile (p, file);
    }
    else if (p->formatInfo.format == PLANKAUDIOFILE_FORMAT_W64)
    {
        result = pl_AudioFileWriter_W64_OpenWithFile (p, file);
    }
#if PLANK_OGGVORBIS
    else if (p->formatInfo.format == PLANKAUDIOFILE_FORMAT_OGGVORBIS)
    {
        result = pl_AudioFileWriter_OggVorbis_OpenWithFile (p, file);
    }
#endif
#if PLANK_OPUS
    else if (p->formatInfo.format == PLANKAUDIOFILE_FORMAT_OPUS)
    {
        result = pl_AudioFileWriter_Opus_OpenWithFile (p, file);
    }
#endif
    else
    {
        result = PlankResult_AudioFileInavlidType;
    }
    
exit:
    return result;
}

PlankResult pl_AudioFileWriter_Close (PlankAudioFileWriterRef p)
{
    PlankResult result = PlankResult_OK;
    
    if (p == PLANK_NULL)
    {
        result = PlankResult_MemoryError;
        goto exit;
    }
    
    if (p->peer == PLANK_NULL)
        return PlankResult_OK;
    
    if ((result = pl_AudioFileWriter_WriteHeader (p)) != PlankResult_OK) goto exit;
    
    switch (p->formatInfo.format)
    {
        case PLANKAUDIOFILE_FORMAT_WAV:
        case PLANKAUDIOFILE_FORMAT_AIFF:
        case PLANKAUDIOFILE_FORMAT_AIFC:
        case PLANKAUDIOFILE_FORMAT_CAF:
        case PLANKAUDIOFILE_FORMAT_UNKNOWNIFF:
            result = pl_IffAudioFileWriter_Destroy ((PlankIffAudioFileWriter*)p->peer);
            break;
#if PLANK_OGGVORBIS
        case PLANKAUDIOFILE_FORMAT_OGGVORBIS:
            result = pl_AudioFileWriter_OggVorbis_Close (p);
            break;
#endif
#if PLANK_OPUS
        case PLANKAUDIOFILE_FORMAT_OPUS:
            result = pl_AudioFileWriter_Opus_Close (p);
            break;
#endif
        default:
            if (p->peer != PLANK_NULL)
                result = PlankResult_UnknownError;
    }
    
    if (result != PlankResult_OK)
        goto exit;
    
    p->peer = PLANK_NULL;
    
    if (p->metaData != PLANK_NULL)
    {
        if ((result = pl_AudioFileMetaData_Destroy (p->metaData)) != PlankResult_OK) goto exit;
        p->metaData = (PlankAudioFileMetaDataRef)PLANK_NULL;
    }
        
exit:
    return result;
}

PlankResult pl_AudioFileWriter_WriteFrames (PlankAudioFileWriterRef p, const int numFrames, const void* data)
{
    PlankResult result = PlankResult_OK;
    
    if (!p->writeFramesFunction)
    {
        result = PlankResult_FunctionsInvalid;
        goto exit;
    }
    
    result = ((PlankAudioFileWriterWriteFramesFunction)p->writeFramesFunction) (p, numFrames, data);
    
    if (result == PlankResult_OK)
        p->numFrames += numFrames;
    
exit:
    return result;
}

PlankB pl_AudioFileWriter_IsEncodingNativeEndian (PlankAudioFileWriterRef p)
{
#if PLANK_BIGENDIAN
    return !! (p->formatInfo.encoding & PLANKAUDIOFILE_ENCODING_BIGENDIAN_FLAG);
#elif PLANK_LITTLEENDIAN
    return  ! (p->formatInfo.encoding & PLANKAUDIOFILE_ENCODING_BIGENDIAN_FLAG);
#else
#error Neither PLANK_BIGENDIAN or PLANK_LITTLEENDIAN are set to 1
#endif
}

PlankResult pl_AudioFileWriter_WriteHeader (PlankAudioFileWriterRef p)
{
    PlankResult result = PlankResult_OK;
    
    if (p->writeHeaderFunction) // not all formats have a header so this is OK
        result = ((PlankAudioFileWriterWriteHeaderFunction)p->writeHeaderFunction)(p);
    
    p->headerPad = 0;
    
    return PlankResult_OK;
}

PlankResult pl_AudioFileWriter_SetHeaderPad (PlankAudioFileWriterRef p, const PlankLL headerPad)
{
    if (headerPad < 0)
        return PlankResult_ItemCountInvalid;
    
    p->headerPad = headerPad;
    return PlankResult_OK;
}

static PlankResult pl_AudioFileWriter_WAV_OpenInternal (PlankAudioFileWriterRef p, const char* filepath, PlankFileRef file)
{
    PlankResult result = PlankResult_OK;
    PlankIffAudioFileWriterRef iff;

    if (((filepath) && (file)) || ((filepath == 0) && (file == 0)))
    {
        result = PlankResult_UnknownError;
        goto exit;
    }

    if (!((p->formatInfo.encoding == PLANKAUDIOFILE_ENCODING_PCM_LITTLEENDIAN) ||
          (p->formatInfo.encoding == PLANKAUDIOFILE_ENCODING_FLOAT_LITTLEENDIAN)))
    {
        result = PlankResult_AudioFileInavlidType;
        goto exit;
    }
    
    if (p->formatInfo.numChannels < 1)
    {
        result = PlankResult_AudioFileInavlidType;
        goto exit;
    }
    
    if ((p->formatInfo.bitsPerSample > 32) &&
        (p->formatInfo.encoding != PLANKAUDIOFILE_ENCODING_FLOAT_LITTLEENDIAN))
    {
        result = PlankResult_AudioFileInavlidType;
        goto exit;
    }
    
    if ((p->formatInfo.bitsPerSample < 8) || (p->formatInfo.bitsPerSample > 64))
    {
        result = PlankResult_AudioFileInavlidType;
        goto exit;
    }
    
    if (p->formatInfo.bytesPerFrame == 0)
    {
        result = PlankResult_AudioFileNotReady;
        goto exit;
    }
    
    if (p->formatInfo.sampleRate <= 0.0)
    {
        result = PlankResult_AudioFileNotReady;
        goto exit;
    }
        
    iff = pl_IffAudioFileWriter_CreateAndInit();
    
    if (!iff)
    {
        result = PlankResult_MemoryError;
        goto exit;
    }
    
    if (filepath)
    {
        result = pl_IffFileWriter_OpenReplacing ((PlankIffFileWriterRef)iff, filepath, PLANK_FALSE, "RIFF", "WAVE", PLANKIFFFILE_ID_FCC);
    }
    else
    {
        result = pl_IffFileWriter_OpenWithFile ((PlankIffFileWriterRef)iff, file, "RIFF", "WAVE", PLANKIFFFILE_ID_FCC);        
    }
    
    if (result != PlankResult_OK)
        goto exit;
    
    p->peer = iff;
    
    p->writeFramesFunction = pl_AudioFileWriter_WAV_WriteFrames;    
    p->writeHeaderFunction = pl_AudioFileWriter_WAV_WriteHeader;

    if ((result = pl_AudioFileWriter_WriteHeader (p)) != PlankResult_OK) goto exit;
    
exit:
    return result;
}


PlankResult pl_AudioFileWriter_WAV_Open (PlankAudioFileWriterRef p, const char* filepath)
{
    return pl_AudioFileWriter_WAV_OpenInternal (p, filepath, 0);
}

PlankResult pl_AudioFileWriter_WAV_OpenWithFile (PlankAudioFileWriterRef p, PlankFileRef file)
{
    return pl_AudioFileWriter_WAV_OpenInternal (p, 0, file);
}

PlankResult pl_AudioFileWriter_WAV_WriteHeader (PlankAudioFileWriterRef p)
{
    PlankIffFileWriterChunkInfoRef chunkInfo;
    PlankResult result = PlankResult_OK;
    PlankIffAudioFileWriterRef iff;
    const char* fmt;
    PlankUS encoding;
    
    iff = (PlankIffAudioFileWriterRef)p->peer;
    
    if ((p->formatInfo.numChannels > 2) || (iff->iff.common.headerInfo.mainLength > 0xffffffff))
        return pl_AudioFileWriter_WAVEXT_WriteHeader (p);
    
    fmt = "fmt ";
    
    switch (p->formatInfo.encoding)
    {
        case PLANKAUDIOFILE_ENCODING_PCM_LITTLEENDIAN:   encoding = PLANKAUDIOFILE_WAV_COMPRESSION_PCM;   break;
        case PLANKAUDIOFILE_ENCODING_FLOAT_LITTLEENDIAN: encoding = PLANKAUDIOFILE_WAV_COMPRESSION_FLOAT; break;
        default: result = PlankResult_AudioFileInavlidType; goto exit;
    }
    
    if ((result = pl_IffFileWriter_SeekChunk ((PlankIffFileWriterRef)iff, 0, fmt, &chunkInfo, 0)) != PlankResult_OK) goto exit;
    
    if (!chunkInfo)
    {
        if ((result = pl_IffFileWriter_WriteChunk ((PlankIffFileWriterRef)iff, 0, fmt, 0, PLANKAUDIOFILE_WAV_HEADER_LENGTH, PLANKIFFFILEWRITER_MODEAPPEND)) != PlankResult_OK) goto exit;
        
        if (p->headerPad > 0)
        {
            if ((result = pl_IffFileWriter_WriteChunk ((PlankIffFileWriterRef)iff, 0, "JUNK", 0, p->headerPad, PLANKIFFFILEWRITER_MODEAPPEND)) != PlankResult_OK) goto exit;
        }
        
        if ((result = pl_IffFileWriter_SeekChunk ((PlankIffFileWriterRef)iff, 0, fmt, &chunkInfo, 0)) != PlankResult_OK) goto exit;
        
        p->metaDataChunkPosition = chunkInfo->chunkPos + PLANKAUDIOFILE_WAV_HEADER_LENGTH;
        p->dataPosition = p->metaDataChunkPosition + (p->headerPad > 0 ? p->headerPad + 8 : 0) + 8;
        
        if (!chunkInfo)
        {
            result = PlankResult_FileReadError;
            goto exit;
        }
    }
    
    if ((result = pl_IffFileWriter_ResizeChunk ((PlankIffFileWriterRef)iff, 0, fmt, PLANKAUDIOFILE_WAV_FMT_LENGTH, PLANK_FALSE)) != PlankResult_OK) goto exit;
    
    if ((result = pl_File_WriteUS ((PlankFileRef)iff, encoding)) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteUS ((PlankFileRef)iff, (PlankUS)p->formatInfo.numChannels)) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteUI ((PlankFileRef)iff, (PlankUI)p->formatInfo.sampleRate)) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteUI ((PlankFileRef)iff, (PlankUI)(p->formatInfo.bytesPerFrame * (int)p->formatInfo.sampleRate))) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteUS ((PlankFileRef)iff, (PlankUS)p->formatInfo.bytesPerFrame)) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteUS ((PlankFileRef)iff, (PlankUS)p->formatInfo.bitsPerSample)) != PlankResult_OK) goto exit;

    // don't use up the junk reserved for the larger WAV header
    if ((result = pl_AudioFileWriter_WAV_WriteMetaData (p)) != PlankResult_OK) goto exit;

exit:
    return result;
}

static PlankUI pl_AudioFileWriter_WAVEXTChannelMapToMask (const PlankAudioFileFormatInfo* formatInfo)
{
    PlankUI channelMask, channelCode, lastChannelCode;
    int i;
    
    channelMask     = 0x00000000;
    lastChannelCode = 0;
    
    for (i = 0; i < formatInfo->numChannels; ++i)
    {
        channelCode = formatInfo->channelMap[i];
        
        if ((channelCode <= lastChannelCode) || // must be in order
            (channelCode > 18))                 // but could look for the addition RF64 ones...
            return 0; // can't encode this channel map to a valid 
        
        lastChannelCode = channelCode;
        
        channelMask |= ((PlankUI)1) << (channelCode - 1);
    }
    
    return channelMask;
}

PlankResult pl_AudioFileWriter_WAVEXT_WriteHeader (PlankAudioFileWriterRef p)
{
    PlankGUID ext;
    PlankIffFileWriterChunkInfoRef chunkInfo;
    PlankResult result = PlankResult_OK;
    PlankIffFileWriterRef iff;
    const char* fmt;
    
    iff  = (PlankIffFileWriterRef)p->peer;
    
    if (iff->common.headerInfo.mainLength > 0xffffffff)
        return pl_AudioFileWriter_WAVRF64_WriteHeader (p);
    
    fmt = "fmt ";

    switch (p->formatInfo.encoding)
    {
        case PLANKAUDIOFILE_ENCODING_PCM_LITTLEENDIAN:   pl_GUID_InitHexString (&ext, "00000001-0000-0010-8000-00aa00389b71"); break;
        case PLANKAUDIOFILE_ENCODING_FLOAT_LITTLEENDIAN: pl_GUID_InitHexString (&ext, "00000003-0000-0010-8000-00aa00389b71"); break;
        default: result = PlankResult_AudioFileInavlidType; goto exit;
    }
    
    if ((result = pl_IffFileWriter_SeekChunk (iff, 0, fmt, &chunkInfo, 0)) != PlankResult_OK) goto exit;
    
    if (!chunkInfo)
    {
        if ((result = pl_IffFileWriter_WriteChunk (iff, 0, fmt, 0, PLANKAUDIOFILE_WAV_HEADER_LENGTH + p->headerPad, PLANKIFFFILEWRITER_MODEAPPEND)) != PlankResult_OK) goto exit;
        
        if (p->headerPad > 0)
        {
            if ((result = pl_IffFileWriter_WriteChunk (iff, 0, "JUNK", 0, p->headerPad, PLANKIFFFILEWRITER_MODEAPPEND)) != PlankResult_OK) goto exit;
        }

        if ((result = pl_IffFileWriter_SeekChunk (iff, 0, fmt, &chunkInfo, 0)) != PlankResult_OK) goto exit;
        
        p->metaDataChunkPosition = chunkInfo->chunkPos + PLANKAUDIOFILE_WAV_HEADER_LENGTH;
        p->dataPosition = p->metaDataChunkPosition + (p->headerPad > 0 ? p->headerPad + 8 : 0) + 8;

        if (!chunkInfo)
        {
            result = PlankResult_FileReadError;
            goto exit;
        }
    }
    
    if ((result = pl_IffFileWriter_ResizeChunk (iff, 0, fmt, PLANKAUDIOFILE_WAV_FMT_EXTENSIBLE_LENGTH, PLANK_FALSE)) != PlankResult_OK) goto exit;
    
    // regular WAV fmt
    if ((result = pl_File_WriteUS ((PlankFileRef)iff, (PlankUS)PLANKAUDIOFILE_WAV_COMPRESSION_EXTENSIBLE)) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteUS ((PlankFileRef)iff, (PlankUS)p->formatInfo.numChannels)) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteUI ((PlankFileRef)iff, (PlankUI)p->formatInfo.sampleRate)) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteUI ((PlankFileRef)iff, (PlankUI)(p->formatInfo.bytesPerFrame * (int)p->formatInfo.sampleRate))) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteUS ((PlankFileRef)iff, (PlankUS)p->formatInfo.bytesPerFrame)) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteUS ((PlankFileRef)iff, (PlankUS)p->formatInfo.bitsPerSample)) != PlankResult_OK) goto exit;
    
    // extensible part
    if ((result = pl_File_WriteUS   ((PlankFileRef)iff, (PLANKAUDIOFILE_WAV_FMT_EXTENSIBLE_LENGTH - PLANKAUDIOFILE_WAV_FMT_LENGTH - sizeof (PlankUS)))) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteUS   ((PlankFileRef)iff, (PlankUS)p->formatInfo.bitsPerSample)) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteUI   ((PlankFileRef)iff, pl_AudioFileWriter_WAVEXTChannelMapToMask (&p->formatInfo))) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteGUID ((PlankFileRef)iff, &ext)) != PlankResult_OK) goto exit;
    
    if ((result = pl_AudioFileWriter_WAV_WriteMetaData (p)) != PlankResult_OK) goto exit;
    
exit:
    return result;    
}

PlankResult pl_AudioFileWriter_WAVRF64_WriteHeader (PlankAudioFileWriterRef p)
{
    PlankGUID ext;
    PlankIffFileWriterChunkInfoRef fmtChunkInfo, junkChunkInfo, ds64ChunkInfo, dataChunkInfo;
    PlankResult result = PlankResult_OK;
    PlankIffFileWriterRef iff;
    const char* fmt;
    const char* ds64;
    const char* RF64;
    const char* data;
    PlankLL dataLength;
    char JUNK[64];

    iff  = (PlankIffFileWriterRef)p->peer;
    fmt  = "fmt ";
    ds64 = "ds64";
    RF64 = "RF64";
    data = "data";
    pl_IffFile_ChunkIDString ((PlankIffFileRef)iff, &iff->common.headerInfo.junkID, JUNK);
    
    // update to RF64 header
    iff->common.headerInfo.mainID.fcc = pl_FourCharCode (RF64);
    if ((result = pl_IffFileWriter_WriteHeader (iff)) != PlankResult_OK) goto exit;
    
    switch (p->formatInfo.encoding)
    {
        case PLANKAUDIOFILE_ENCODING_PCM_LITTLEENDIAN:   pl_GUID_InitHexString (&ext, "00000001-0000-0010-8000-00aa00389b71"); break;
        case PLANKAUDIOFILE_ENCODING_FLOAT_LITTLEENDIAN: pl_GUID_InitHexString (&ext, "00000003-0000-0010-8000-00aa00389b71"); break;
        default: result = PlankResult_AudioFileInavlidType; goto exit;
    }
    
    if ((result = pl_IffFileWriter_SeekChunk (iff, 0, ds64, &ds64ChunkInfo, 0)) != PlankResult_OK) goto exit;
    if ((result = pl_IffFileWriter_SeekChunk (iff, 0, fmt,  &fmtChunkInfo,  0)) != PlankResult_OK) goto exit;
    if ((result = pl_IffFileWriter_SeekChunk (iff, 0, data, &dataChunkInfo, 0)) != PlankResult_OK) goto exit;

    dataLength = dataChunkInfo ? dataChunkInfo->chunkLength : 0;
        
    if (!fmtChunkInfo && !ds64ChunkInfo)
    {
        if ((result = pl_IffFileWriter_WriteChunk (iff, 0, ds64, 0, PLANKAUDIOFILE_WAV_DS64_MINIMUMLENGTH, PLANKIFFFILEWRITER_MODEAPPEND)) != PlankResult_OK) goto exit;
        if ((result = pl_IffFileWriter_WriteChunk (iff, 0, fmt, 0, PLANKAUDIOFILE_WAV_FMT_EXTENSIBLE_LENGTH, PLANKIFFFILEWRITER_MODEAPPEND)) != PlankResult_OK) goto exit;
    }
    else if (!ds64ChunkInfo)
    {
        // we have format but not ds64
        
        // ensure the fmt size if correct
        if ((result = pl_IffFileWriter_ResizeChunk (iff, 0, fmt, PLANKAUDIOFILE_WAV_FMT_EXTENSIBLE_LENGTH, PLANK_FALSE)) != PlankResult_OK) goto exit;

        // seek junk
        if ((result = pl_IffFileWriter_SeekChunk (iff, fmtChunkInfo->chunkPos, JUNK, &junkChunkInfo, 0)) != PlankResult_OK) goto exit;

        if (!junkChunkInfo)
        {
            // the junk isn't there
            result = PlankResult_UnknownError;
            goto exit;
        }
        
        if ((fmtChunkInfo->chunkPos + fmtChunkInfo->chunkLength + (fmtChunkInfo->chunkLength & 1)) != (junkChunkInfo->chunkPos - 8))
        {
            // the junk isn't next
            result = PlankResult_UnknownError;
            goto exit;
        }
        
        junkChunkInfo->chunkPos = fmtChunkInfo->chunkPos;
        fmtChunkInfo->chunkPos += junkChunkInfo->chunkLength + 4 + iff->common.headerInfo.lengthSize;
        junkChunkInfo->chunkID.fcc = pl_FourCharCode (ds64);
        
        // update chunk headers writing no other bytes
        if ((result = pl_IffFileWriter_WriteChunk (iff, 0, ds64, 0, 0, PLANKIFFFILEWRITER_MODEREPLACEGROW)) != PlankResult_OK) goto exit;
        if ((result = pl_IffFileWriter_WriteChunk (iff, 0, fmt,  0, 0, PLANKIFFFILEWRITER_MODEREPLACEGROW)) != PlankResult_OK) goto exit;
    }

    // "ds64"
    if ((result = pl_IffFileWriter_SeekChunk (iff, 0, ds64, &ds64ChunkInfo, 0)) != PlankResult_OK) goto exit;
    
    // write the ds64 stuff...
    if ((result = pl_File_WriteLL    ((PlankFileRef)iff, iff->common.headerInfo.mainLength)) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteLL    ((PlankFileRef)iff, dataLength)) != PlankResult_OK) goto exit;
    
    // could search all the chunks for those > 0xffffffff and inlcude a table
    // ... but the mininum size we use for ds64 isn't large enough to accomodate
    if ((result = pl_File_WriteZeros ((PlankFileRef)iff, 12)) != PlankResult_OK) goto exit;

    // "fmt "
    if ((result = pl_IffFileWriter_SeekChunk (iff, 0, fmt,  &fmtChunkInfo,  0)) != PlankResult_OK) goto exit;
    
    // regular WAV fmt
    if ((result = pl_File_WriteUS ((PlankFileRef)iff, (PlankUS)PLANKAUDIOFILE_WAV_COMPRESSION_EXTENSIBLE)) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteUS ((PlankFileRef)iff, (PlankUS)p->formatInfo.numChannels)) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteUI ((PlankFileRef)iff, (PlankUI)p->formatInfo.sampleRate)) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteUI ((PlankFileRef)iff, (PlankUI)(p->formatInfo.bytesPerFrame * (int)p->formatInfo.sampleRate))) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteUS ((PlankFileRef)iff, (PlankUS)p->formatInfo.bytesPerFrame)) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteUS ((PlankFileRef)iff, (PlankUS)p->formatInfo.bitsPerSample)) != PlankResult_OK) goto exit;

    // extensible part
    if ((result = pl_File_WriteUS   ((PlankFileRef)iff, (PLANKAUDIOFILE_WAV_FMT_EXTENSIBLE_LENGTH - PLANKAUDIOFILE_WAV_FMT_LENGTH - sizeof (PlankUS)))) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteUS   ((PlankFileRef)iff, (PlankUS)p->formatInfo.bitsPerSample)) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteUI   ((PlankFileRef)iff, pl_AudioFileWriter_WAVEXTChannelMapToMask (&p->formatInfo))) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteGUID ((PlankFileRef)iff, &ext)) != PlankResult_OK) goto exit;
    
exit:
    return result;
}

PlankResult pl_AudioFileWriter_WAV_WriteFrames (PlankAudioFileWriterRef p, const int numFrames, const void* data)
{
    PlankResult result;
    PlankIffFileWriterRef iff;
    
    result = PlankResult_OK;
    iff  = (PlankIffFileWriterRef)p->peer;
    
    if ((result = pl_AudioFileWriter_Iff_WriteFrames (p, "data", numFrames, data)) != PlankResult_OK) goto exit;
    
    if (iff->common.headerInfo.mainLength > 0xffffffff)
    {
        if ((result = pl_AudioFileWriter_WriteHeader (p)) != PlankResult_OK) goto exit;
    }
    
exit:
    return result;
}

static PlankResult pl_AudioFileWriter_AIFF_OpenInternal (PlankAudioFileWriterRef p, const char* filepath, PlankFileRef file)
{
    PlankResult result = PlankResult_OK;
    PlankIffAudioFileWriterRef iff;

    if (((filepath) && (file)) || ((filepath == 0) && (file == 0)))
    {
        result = PlankResult_UnknownError;
        goto exit;
    }

    if (p->formatInfo.encoding != PLANKAUDIOFILE_ENCODING_PCM_BIGENDIAN)
    {
        result = PlankResult_AudioFileInavlidType;
        goto exit;
    }
    
    if (p->formatInfo.numChannels < 1)
    {
        result = PlankResult_AudioFileInavlidType;
        goto exit;
    }
    
    if ((p->formatInfo.bitsPerSample < 8) || (p->formatInfo.bitsPerSample > 32))
    {
        result = PlankResult_AudioFileInavlidType;
        goto exit;
    }
    
    if (p->formatInfo.bytesPerFrame == 0)
    {
        result = PlankResult_AudioFileNotReady;
        goto exit;
    }
    
    if (p->formatInfo.sampleRate <= 0.0)
    {
        result = PlankResult_AudioFileNotReady;
        goto exit;
    }
     
    iff = pl_IffAudioFileWriter_CreateAndInit();
    
    if (filepath)
    {
        result = pl_IffFileWriter_OpenReplacing ((PlankIffFileWriterRef)iff, filepath, PLANK_TRUE, "FORM", "AIFF", PLANKIFFFILE_ID_FCC);
    }
    else
    {
        result = pl_IffFileWriter_OpenWithFile ((PlankIffFileWriterRef)iff, file, "FORM", "AIFF", PLANKIFFFILE_ID_FCC);
    }
    
    if (result != PlankResult_OK)
        goto exit;
    
    p->peer = iff;
        
    p->writeFramesFunction = pl_AudioFileWriter_AIFF_WriteFrames;
    p->writeHeaderFunction = pl_AudioFileWriter_AIFF_WriteHeader;
    
    if ((result = pl_AudioFileWriter_WriteHeader (p)) != PlankResult_OK) goto exit;
    
exit:
    return result;
}

PlankResult pl_AudioFileWriter_AIFF_Open (PlankAudioFileWriterRef p, const char* filepath)
{
    return pl_AudioFileWriter_AIFF_OpenInternal (p, filepath, 0);
}

PlankResult pl_AudioFileWriter_AIFF_OpenWithFile (PlankAudioFileWriterRef p, PlankFileRef file)
{
    return pl_AudioFileWriter_AIFF_OpenInternal (p, 0, file);
}

PlankResult pl_AudioFileWriter_AIFF_WriteHeader (PlankAudioFileWriterRef p)
{
    PlankIffFileWriterChunkInfoRef chunkInfo;
    PlankResult result = PlankResult_OK;
    PlankIffFileWriterRef iff;
    const char* COMM;
    PlankF80 sampleRate;
    
    iff = (PlankIffFileWriterRef)p->peer;
    COMM = "COMM";
        
    if ((result = pl_IffFileWriter_SeekChunk (iff, 0, COMM, &chunkInfo, 0)) != PlankResult_OK) goto exit;
    
    if (!chunkInfo)
    {
        if ((result = pl_IffFileWriter_WriteChunk (iff, 0, COMM, 0, PLANKAUDIOFILE_AIFF_COMM_LENGTH + p->headerPad, PLANKIFFFILEWRITER_MODEAPPEND)) != PlankResult_OK) goto exit;
        if ((result = pl_IffFileWriter_SeekChunk (iff, 0, COMM, &chunkInfo, 0)) != PlankResult_OK) goto exit;
        
        if (!chunkInfo)
        {
            result = PlankResult_FileReadError;
            goto exit;
        }
    }
        
    if (p->numFrames > 0xffffffff)
    {
        result = PlankResult_UnknownError;
        goto exit;
    }
    
    if ((result = pl_IffFileWriter_ResizeChunk (iff, 0, COMM, PLANKAUDIOFILE_AIFF_COMM_LENGTH, PLANK_FALSE)) != PlankResult_OK) goto exit;
    
    sampleRate = pl_I2F80 ((PlankUI)p->formatInfo.sampleRate);
    
    if ((result = pl_File_WriteS  ((PlankFileRef)iff, (PlankS)p->formatInfo.numChannels)) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteUI ((PlankFileRef)iff, (PlankUI)p->numFrames)) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteS  ((PlankFileRef)iff, (PlankS)p->formatInfo.bitsPerSample)) != PlankResult_OK) goto exit;
    if ((result = pl_File_Write   ((PlankFileRef)iff, sampleRate.data, sizeof (sampleRate))) != PlankResult_OK) goto exit;
    
exit:
    return result;    
}

PlankResult pl_AudioFileWriter_AIFF_WriteFrames (PlankAudioFileWriterRef p, const int numFrames, const void* data)
{
    return pl_AudioFileWriter_Iff_WriteFrames (p, "SSND", numFrames, data);
}

static PlankResult pl_AudioFileWriter_AIFC_OpenInternal (PlankAudioFileWriterRef p, const char* filepath, PlankFileRef file)
{
    PlankResult result = PlankResult_OK;
    PlankIffAudioFileWriterRef iff;
    int mode;
    
    if (((filepath) && (file)) || ((filepath == 0) && (file == 0)))
    {
        result = PlankResult_UnknownError;
        goto exit;
    }

    if (p->formatInfo.numChannels < 1)
    {
        result = PlankResult_AudioFileInavlidType;
        goto exit;
    }
    
    if ((p->formatInfo.bitsPerSample > 32) &&
        (p->formatInfo.encoding != PLANKAUDIOFILE_ENCODING_FLOAT_BIGENDIAN))
    {
        result = PlankResult_AudioFileInavlidType;
        goto exit;
    }
    
    if ((p->formatInfo.bitsPerSample < 8) || (p->formatInfo.bitsPerSample > 64))
    {
        result = PlankResult_AudioFileInavlidType;
        goto exit;
    }
    
    if (p->formatInfo.bytesPerFrame == 0)
    {
        result = PlankResult_AudioFileNotReady;
        goto exit;
    }
    
    if (p->formatInfo.sampleRate <= 0.0)
    {
        result = PlankResult_AudioFileNotReady;
        goto exit;
    }
    
    iff = pl_IffAudioFileWriter_CreateAndInit();
    
    if (filepath)
    {
        result = pl_IffFileWriter_OpenReplacing ((PlankIffFileWriterRef)iff, filepath, PLANK_TRUE, "FORM", "AIFC", PLANKIFFFILE_ID_FCC);
    }
    else
    {
        if ((result = pl_File_GetMode (file, &mode)) != PlankResult_OK) goto exit;
        
        if (!(mode & PLANKFILE_BIGENDIAN))
        {
            result = PlankResult_FileInvalid;
            goto exit;
        }

        result = pl_IffFileWriter_OpenWithFile ((PlankIffFileWriterRef)iff, file, "FORM", "AIFC", PLANKIFFFILE_ID_FCC);
    }
    
    if (result != PlankResult_OK)
        goto exit;
    
    p->peer = iff;
        
    p->writeFramesFunction = pl_AudioFileWriter_AIFC_WriteFrames;
    p->writeHeaderFunction = pl_AudioFileWriter_AIFC_WriteHeader;
    
    if ((result = pl_AudioFileWriter_WriteHeader (p)) != PlankResult_OK) goto exit;
    
exit:
    return result;
}

PlankResult pl_AudioFileWriter_AIFC_Open (PlankAudioFileWriterRef p, const char* filepath)
{
    return pl_AudioFileWriter_AIFC_OpenInternal (p, filepath, 0);
}

PlankResult pl_AudioFileWriter_AIFC_OpenWithFile (PlankAudioFileWriterRef p, PlankFileRef file)
{
    return pl_AudioFileWriter_AIFC_OpenInternal (p, 0, file);
}

PlankResult pl_AudioFileWriter_AIFC_WriteHeader (PlankAudioFileWriterRef p)
{
    PlankIffFileWriterChunkInfoRef chunkInfo;
    PlankResult result = PlankResult_OK;
    PlankIffFileWriterRef iff;
    const char* COMM;
    const char* FVER;
    PlankFourCharCode compressionID;
    PlankF80 sampleRate;
    PlankUI fver;
    
    iff = (PlankIffFileWriterRef)p->peer;
    COMM = "COMM";
    FVER = "FVER";
    fver = PLANKAUDIOFILE_AIFC_VERSION;
    
#if PLANK_LITTLEENDIAN
    pl_SwapEndianUI (&fver);
#endif
    
    if (p->numFrames > 0xffffffff)
    {
        result = PlankResult_UnknownError;
        goto exit;
    }
        
    if ((result = pl_IffFileWriter_WriteChunk (iff, 0, FVER, &fver, sizeof (fver), PLANKIFFFILEWRITER_MODEREPLACEGROW)) != PlankResult_OK) goto exit;
    if ((result = pl_IffFileWriter_SeekChunk (iff, 0, COMM, &chunkInfo, 0)) != PlankResult_OK) goto exit;
    
    if (!chunkInfo)
    {
        if ((result = pl_IffFileWriter_WriteChunk (iff, 0, COMM, 0, PLANKAUDIOFILE_AIFC_COMM_LENGTH + p->headerPad, PLANKIFFFILEWRITER_MODEAPPEND)) != PlankResult_OK) goto exit;
        if ((result = pl_IffFileWriter_SeekChunk (iff, 0, COMM, &chunkInfo, 0)) != PlankResult_OK) goto exit;
        
        if (!chunkInfo)
        {
            result = PlankResult_FileReadError;
            goto exit;
        }
    }
        
    sampleRate = pl_I2F80 ((PlankUI)p->formatInfo.sampleRate);
    
    switch (p->formatInfo.encoding) {
        case PLANKAUDIOFILE_ENCODING_PCM_BIGENDIAN:
            compressionID = pl_FourCharCode ("NONE");
            break;
        case PLANKAUDIOFILE_ENCODING_PCM_LITTLEENDIAN:
            compressionID = pl_FourCharCode ("sowt");
            break;
        case PLANKAUDIOFILE_ENCODING_FLOAT_BIGENDIAN:
            if (p->formatInfo.bitsPerSample == 32)
            {
                compressionID = pl_FourCharCode ("fl32");
            }
            else if (p->formatInfo.bitsPerSample == 64)
            {
                compressionID = pl_FourCharCode ("fl64");
            }
            else
            {
                result = PlankResult_AudioFileInavlidType;
                goto exit;
            }
                
            break;
    }
    
    if ((result = pl_IffFileWriter_ResizeChunk (iff, 0, COMM, PLANKAUDIOFILE_AIFC_COMM_LENGTH, PLANK_FALSE)) != PlankResult_OK) goto exit;
    
    if ((result = pl_File_WriteS  ((PlankFileRef)iff, (PlankS)p->formatInfo.numChannels)) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteUI ((PlankFileRef)iff, (PlankUI)p->numFrames)) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteS  ((PlankFileRef)iff, (PlankS)p->formatInfo.bitsPerSample)) != PlankResult_OK) goto exit;
    if ((result = pl_File_Write   ((PlankFileRef)iff, sampleRate.data, sizeof (sampleRate))) != PlankResult_OK) goto exit;
    
    if ((result = pl_File_WriteFourCharCode ((PlankFileRef)p->peer, compressionID)) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteUS ((PlankFileRef)iff, 0)) != PlankResult_OK) goto exit; // compression description as a pascal string
    
exit:
    return result;
}

PlankResult pl_AudioFileWriter_AIFC_WriteFrames (PlankAudioFileWriterRef p, const int numFrames, const void* data)
{
    return pl_AudioFileWriter_Iff_WriteFrames (p, "SSND", numFrames, data);
}

static PlankResult pl_AudioFileWriter_CAF_OpenInternal (PlankAudioFileWriterRef p, const char* filepath, PlankFileRef file)
{
    PlankResult result = PlankResult_OK;
    PlankIffAudioFileWriterRef iff;
    int mode;
    
    if (((filepath) && (file)) || ((filepath == 0) && (file == 0)))
    {
        result = PlankResult_UnknownError;
        goto exit;
    }
    
    if (p->formatInfo.numChannels < 1)
    {
        result = PlankResult_AudioFileInavlidType;
        goto exit;
    }
    
    if ((p->formatInfo.bitsPerSample > 32) &&
        !(p->formatInfo.encoding & PLANKAUDIOFILE_ENCODING_FLOAT_FLAG))
    {
        result = PlankResult_AudioFileInavlidType;
        goto exit;
    }
    
    if ((p->formatInfo.bitsPerSample < 8) || (p->formatInfo.bitsPerSample > 64))
    {
        result = PlankResult_AudioFileInavlidType;
        goto exit;
    }
    
    if (p->formatInfo.bytesPerFrame == 0)
    {
        result = PlankResult_AudioFileNotReady;
        goto exit;
    }
    
    if (p->formatInfo.sampleRate <= 0.0)
    {
        result = PlankResult_AudioFileNotReady;
        goto exit;
    }
    
    iff = pl_IffAudioFileWriter_CreateAndInit();
    
    if (filepath)
    {
        result = pl_IffFileWriter_OpenReplacing ((PlankIffFileWriterRef)iff, filepath, PLANK_TRUE, "caff", "", PLANKIFFFILE_ID_FCC);
    }
    else
    {
        if ((result = pl_File_GetMode (file, &mode)) != PlankResult_OK) goto exit;
        
        if (!(mode & PLANKFILE_BIGENDIAN))
        {
            result = PlankResult_FileInvalid;
            goto exit;
        }

        result = pl_IffFileWriter_OpenWithFile ((PlankIffFileWriterRef)iff, file, "caff", "", PLANKIFFFILE_ID_FCC);
    }
    
    if (result != PlankResult_OK)
        goto exit;
    
    p->peer = iff;
    
    p->writeFramesFunction = pl_AudioFileWriter_CAF_WriteFrames;
    p->writeHeaderFunction = pl_AudioFileWriter_CAF_WriteHeader;
    
    if ((result = pl_AudioFileWriter_WriteHeader (p)) != PlankResult_OK) goto exit;
    
exit:
    return result;    
}

PlankResult pl_AudioFileWriter_CAF_Open (PlankAudioFileWriterRef p, const char* filepath)
{
    return pl_AudioFileWriter_CAF_OpenInternal (p, filepath, 0);
}

PlankResult pl_AudioFileWriter_CAF_OpenWithFile (PlankAudioFileWriterRef p, PlankFileRef file)
{
    return pl_AudioFileWriter_CAF_OpenInternal (p, 0, file);
}

PlankResult pl_AudioFileWriter_CAF_WriteHeader (PlankAudioFileWriterRef p)
{
    PlankIffFileWriterChunkInfoRef chunkInfo;
    PlankResult result;
    PlankIffFileWriterRef iff;
    const char* desc;
    const char* data;
    PlankUI formatFlags;
    
    result = PlankResult_OK;
    iff    = (PlankIffFileWriterRef)p->peer;
    desc   = "desc";
    data   = "data";
    
    if ((result = pl_IffFileWriter_SeekChunk (iff, 0, desc, &chunkInfo, 0)) != PlankResult_OK) goto exit;
    
    if (!chunkInfo)
    {
        if ((result = pl_IffFileWriter_WriteChunk (iff, 0, desc, 0, PLANKAUDIOFILE_CAF_DESC_LENGTH + p->headerPad, PLANKIFFFILEWRITER_MODEAPPEND)) != PlankResult_OK) goto exit;
        if ((result = pl_IffFileWriter_SeekChunk (iff, 0, desc, &chunkInfo, 0)) != PlankResult_OK) goto exit;
        
        if (!chunkInfo)
        {
            result = PlankResult_FileReadError;
            goto exit;
        }
    }
    
    formatFlags = 0;
    
    if (p->formatInfo.encoding & PLANKAUDIOFILE_ENCODING_FLOAT_FLAG)
        formatFlags |= PLANKAUDIOFILE_CAF_FLOAT_FLAG;
    
    if (!(p->formatInfo.encoding & PLANKAUDIOFILE_ENCODING_BIGENDIAN_FLAG))
        formatFlags |= PLANKAUDIOFILE_CAF_LITTLEENDIAN_FLAG;
    
    if ((result = pl_IffFileWriter_ResizeChunk (iff, 0, desc, PLANKAUDIOFILE_CAF_DESC_LENGTH, PLANK_FALSE)) != PlankResult_OK) goto exit;
    
    if ((result = pl_File_WriteD ((PlankFileRef)iff, p->formatInfo.sampleRate)) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteFourCharCode ((PlankFileRef)iff, pl_FourCharCode ("lpcm"))) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteUI ((PlankFileRef)iff, formatFlags)) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteUI ((PlankFileRef)iff, p->formatInfo.bytesPerFrame)) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteUI ((PlankFileRef)iff, 1)) != PlankResult_OK) goto exit; // frames per packet
    if ((result = pl_File_WriteUI ((PlankFileRef)iff, p->formatInfo.numChannels)) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteUI ((PlankFileRef)iff, p->formatInfo.bitsPerSample)) != PlankResult_OK) goto exit;

    if ((result = pl_IffFileWriter_SeekChunk (iff, 0, data, &chunkInfo, 0)) != PlankResult_OK) goto exit;
    
    if (!chunkInfo)
    {
        // write the 4 bytes "edit count"
        if ((result = pl_IffFileWriter_WriteChunk (iff, 0, data, 0, 4, PLANKIFFFILEWRITER_MODEAPPEND)) != PlankResult_OK) goto exit;
        if ((result = pl_IffFileWriter_SeekChunk (iff, 0, data, &chunkInfo, 0)) != PlankResult_OK) goto exit;
        
        if (!chunkInfo)
        {
            result = PlankResult_FileReadError;
            goto exit;
        }
    }    
    
exit:
    return result;    
}

PlankResult pl_AudioFileWriter_CAF_WriteFrames (PlankAudioFileWriterRef p, const int numFrames, const void* data)
{
    return pl_AudioFileWriter_Iff_WriteFrames (p, "data", numFrames, data);
}

static PlankResult pl_AudioFileWriter_W64_OpenInternal (PlankAudioFileWriterRef p, const char* filepath, PlankFileRef file)
{
    PlankResult result = PlankResult_OK;
    PlankIffAudioFileWriterRef iff;
    
    if (((filepath) && (file)) || ((filepath == 0) && (file == 0)))
    {
        result = PlankResult_UnknownError;
        goto exit;
    }
    
    if (!((p->formatInfo.encoding == PLANKAUDIOFILE_ENCODING_PCM_LITTLEENDIAN) ||
          (p->formatInfo.encoding == PLANKAUDIOFILE_ENCODING_FLOAT_LITTLEENDIAN)))
    {
        result = PlankResult_AudioFileInavlidType;
        goto exit;
    }
    
    if (p->formatInfo.numChannels < 1)
    {
        result = PlankResult_AudioFileInavlidType;
        goto exit;
    }
    
    if ((p->formatInfo.bitsPerSample > 32) &&
        (p->formatInfo.encoding != PLANKAUDIOFILE_ENCODING_FLOAT_LITTLEENDIAN))
    {
        result = PlankResult_AudioFileInavlidType;
        goto exit;
    }
    
    if ((p->formatInfo.bitsPerSample < 8) || (p->formatInfo.bitsPerSample > 64))
    {
        result = PlankResult_AudioFileInavlidType;
        goto exit;
    }
    
    if (p->formatInfo.bytesPerFrame == 0)
    {
        result = PlankResult_AudioFileNotReady;
        goto exit;
    }
    
    if (p->formatInfo.sampleRate <= 0.0)
    {
        result = PlankResult_AudioFileNotReady;
        goto exit;
    }
    
    iff = pl_IffAudioFileWriter_CreateAndInit();
    
    if (!iff)
    {
        result = PlankResult_MemoryError;
        goto exit;
    }
    
    if (filepath)
    {
        result = pl_IffFileWriter_OpenReplacing ((PlankIffFileWriterRef)iff, filepath, PLANK_FALSE, PLANKAUDIOFILE_W64_RIFF_ID, PLANKAUDIOFILE_W64_WAVE_ID, PLANKIFFFILE_ID_GUID);
    }
    else
    {
        result = pl_IffFileWriter_OpenWithFile ((PlankIffFileWriterRef)iff, file, PLANKAUDIOFILE_W64_RIFF_ID, PLANKAUDIOFILE_W64_WAVE_ID, PLANKIFFFILE_ID_FCC);
    }
    
    if (result != PlankResult_OK)
        goto exit;
    
    p->peer = iff;
    
    p->writeFramesFunction = pl_AudioFileWriter_W64_WriteFrames;
    p->writeHeaderFunction = pl_AudioFileWriter_W64_WriteHeader;
    
    if ((result = pl_AudioFileWriter_WriteHeader (p)) != PlankResult_OK) goto exit;
    
exit:
    return result;    
}

PlankResult pl_AudioFileWriter_W64_Open (PlankAudioFileWriterRef p, const char* filepath)
{
    return pl_AudioFileWriter_W64_OpenInternal (p, filepath, 0);
}

PlankResult pl_AudioFileWriter_W64_OpenWithFile (PlankAudioFileWriterRef p, PlankFileRef file)
{
    return pl_AudioFileWriter_W64_OpenInternal (p, 0, file);
}

PlankResult pl_AudioFileWriter_W64_WriteHeader (PlankAudioFileWriterRef p)
{
    PlankGUID ext;
    PlankIffFileWriterChunkInfoRef chunkInfo;
    PlankResult result = PlankResult_OK;
    PlankIffFileWriterRef iff;
    
    iff  = (PlankIffFileWriterRef)p->peer;
            
    switch (p->formatInfo.encoding)
    {
        case PLANKAUDIOFILE_ENCODING_PCM_LITTLEENDIAN:   pl_GUID_InitHexString (&ext, "00000001-0000-0010-8000-00aa00389b71"); break;
        case PLANKAUDIOFILE_ENCODING_FLOAT_LITTLEENDIAN: pl_GUID_InitHexString (&ext, "00000003-0000-0010-8000-00aa00389b71"); break;
        default: result = PlankResult_AudioFileInavlidType; goto exit;
    }
    
    if ((result = pl_IffFileWriter_SeekChunk (iff, 0, PLANKAUDIOFILE_W64_FMT_ID, &chunkInfo, 0)) != PlankResult_OK) goto exit;
    
    if (!chunkInfo)
    {
        if ((result = pl_IffFileWriter_WriteChunk (iff, 0, PLANKAUDIOFILE_W64_FMT_ID, 0, PLANKAUDIOFILE_WAV_FMT_EXTENSIBLE_LENGTH + p->headerPad, PLANKIFFFILEWRITER_MODEAPPEND)) != PlankResult_OK) goto exit;
        if ((result = pl_IffFileWriter_SeekChunk (iff, 0, PLANKAUDIOFILE_W64_FMT_ID, &chunkInfo, 0)) != PlankResult_OK) goto exit;
        
        if (!chunkInfo)
        {
            result = PlankResult_FileReadError;
            goto exit;
        }
    }
    
    if ((result = pl_IffFileWriter_ResizeChunk (iff, 0, PLANKAUDIOFILE_W64_FMT_ID, PLANKAUDIOFILE_WAV_FMT_EXTENSIBLE_LENGTH, PLANK_FALSE)) != PlankResult_OK) goto exit;
    
    // regular WAV fmt
    if ((result = pl_File_WriteUS ((PlankFileRef)iff, (PlankUS)PLANKAUDIOFILE_WAV_COMPRESSION_EXTENSIBLE)) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteUS ((PlankFileRef)iff, (PlankUS)p->formatInfo.numChannels)) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteUI ((PlankFileRef)iff, (PlankUI)p->formatInfo.sampleRate)) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteUI ((PlankFileRef)iff, (PlankUI)(p->formatInfo.bytesPerFrame * (int)p->formatInfo.sampleRate))) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteUS ((PlankFileRef)iff, (PlankUS)p->formatInfo.bytesPerFrame)) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteUS ((PlankFileRef)iff, (PlankUS)p->formatInfo.bitsPerSample)) != PlankResult_OK) goto exit;
    
    // extensible part
    if ((result = pl_File_WriteUS   ((PlankFileRef)iff, (PLANKAUDIOFILE_WAV_FMT_EXTENSIBLE_LENGTH - PLANKAUDIOFILE_WAV_FMT_LENGTH - sizeof (PlankUS)))) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteUS   ((PlankFileRef)iff, (PlankUS)p->formatInfo.bitsPerSample)) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteUI   ((PlankFileRef)iff, pl_AudioFileWriter_WAVEXTChannelMapToMask (&p->formatInfo))) != PlankResult_OK) goto exit;
    if ((result = pl_File_WriteGUID ((PlankFileRef)iff, &ext)) != PlankResult_OK) goto exit;
    
exit:
    return result;    
}

PlankResult pl_AudioFileWriter_W64_WriteFrames (PlankAudioFileWriterRef p, const int numFrames, const void* data)
{
    return pl_AudioFileWriter_Iff_WriteFrames (p, PLANKAUDIOFILE_W64_DATA_ID, numFrames, data);
}

PlankResult pl_AudioFileWriter_Iff_WriteFrames (PlankAudioFileWriterRef p, const char* chunkID, const int numFrames, const void* data)
{
    PlankUC buffer[PLANKAUDIOFILEWRITER_BUFFERLENGTH];
    PlankIffFileWriterChunkInfoRef chunkInfo;
    PlankResult result = PlankResult_OK;
    PlankIffFileWriterRef iff;
    PlankUC* ptr;
    PlankUC* swapPtr;
    int numSamplesRemaining, numSamplesThisTime, numBufferSamples, bytesPerSample, numBytes, i, numChannels;
    
    iff = (PlankIffFileWriterRef)p->peer;
    numChannels = p->formatInfo.numChannels;
    bytesPerSample = p->formatInfo.bytesPerFrame / numChannels;

    if ((bytesPerSample == 1) || pl_AudioFileWriter_IsEncodingNativeEndian (p))
    {
        result = pl_IffFileWriter_WriteChunk (iff, 0, chunkID, data, numFrames * p->formatInfo.bytesPerFrame, PLANKIFFFILEWRITER_MODEAPPEND);
        if (result != PlankResult_OK) goto exit;
    }
    else
    {
        numBufferSamples = (PLANKAUDIOFILEWRITER_BUFFERLENGTH / p->formatInfo.bytesPerFrame) * p->formatInfo.numChannels;
        numSamplesRemaining = numFrames * p->formatInfo.numChannels;
        ptr = (PlankUC*)data;
        
        // this is unrolled to help optimisation as this is slower than we want anyway: having to swap endianness...!
        switch (bytesPerSample)
        {
            case 2:
                while (numSamplesRemaining > 0)
                {
                    numSamplesThisTime = pl_MinI (numSamplesRemaining, numBufferSamples);
                    numBytes = numSamplesThisTime * bytesPerSample;
                    pl_MemoryCopy (buffer, ptr, numBytes);
                    
                    swapPtr = buffer;
                    
                    for (i = 0; i < numSamplesThisTime; ++i)
                    {
                        pl_SwapEndianS ((PlankS*)swapPtr);
                        swapPtr += 2;
                    }
                    
                    result = pl_IffFileWriter_WriteChunk (iff, 0, chunkID, buffer, numBytes, PLANKIFFFILEWRITER_MODEAPPEND);
                    if (result != PlankResult_OK) goto exit;

                    numSamplesRemaining -= numSamplesThisTime;
                    ptr += numBytes;
                }
                break;
            case 3:
                while (numSamplesRemaining > 0)
                {
                    numSamplesThisTime = pl_MinI (numSamplesRemaining, numBufferSamples);
                    numBytes = numSamplesThisTime * bytesPerSample;
                    pl_MemoryCopy (buffer, ptr, numBytes);
                    
                    // not sure why!
//                    swapPtr = buffer;
//                    
//                    for (i = 0; i < numSamplesThisTime; ++i)
//                    {
//                        pl_SwapEndianI24 ((PlankI24*)swapPtr);
//                        swapPtr += 3;
//                    }
                    
                    result = pl_IffFileWriter_WriteChunk (iff, 0, chunkID, buffer, numBytes, PLANKIFFFILEWRITER_MODEAPPEND);
                    if (result != PlankResult_OK) goto exit;
                    
                    numSamplesRemaining -= numSamplesThisTime;
                    ptr += numBytes;
                }
                break;
            case 4:
                while (numSamplesRemaining > 0)
                {
                    numSamplesThisTime = pl_MinI (numSamplesRemaining, numBufferSamples);
                    numBytes = numSamplesThisTime * bytesPerSample;
                    pl_MemoryCopy (buffer, ptr, numBytes);
                    
                    swapPtr = buffer;
                    
                    for (i = 0; i < numSamplesThisTime; ++i)
                    {
                        pl_SwapEndianI ((PlankI*)swapPtr);
                        swapPtr += 4;
                    }
                    
                    result = pl_IffFileWriter_WriteChunk (iff, 0, chunkID, buffer, numBytes, PLANKIFFFILEWRITER_MODEAPPEND);
                    if (result != PlankResult_OK) goto exit;
                    
                    numSamplesRemaining -= numSamplesThisTime;
                    ptr += numBytes;
                }
                break;
            case 8:
                while (numSamplesRemaining > 0)
                {
                    numSamplesThisTime = pl_MinI (numSamplesRemaining, numBufferSamples);
                    numBytes = numSamplesThisTime * bytesPerSample;
                    pl_MemoryCopy (buffer, ptr, numBytes);
                    
                    swapPtr = buffer;
                    
                    for (i = 0; i < numSamplesThisTime; ++i)
                    {
                        pl_SwapEndianLL ((PlankLL*)swapPtr);
                        swapPtr += 8;
                    }
                    
                    result = pl_IffFileWriter_WriteChunk (iff, 0, chunkID, buffer, numBytes, PLANKIFFFILEWRITER_MODEAPPEND);
                    if (result != PlankResult_OK) goto exit;
                    
                    numSamplesRemaining -= numSamplesThisTime;
                    ptr += numBytes;
                }
                break;
            default:
                result = PlankResult_AudioFileInavlidType;
                goto exit;
        }        
    }
    
    if ((result = pl_IffFileWriter_SeekChunk (iff, p->dataPosition, chunkID, &chunkInfo, 0)) != PlankResult_OK) goto exit;
    
    if (chunkInfo)
        p->dataPosition = pl_MaxLL (p->dataPosition, chunkInfo->chunkPos);
    
exit:
    return result;
}

#if PLANK_OGGVORBIS || PLANK_OPUS

static PlankResult pl_AudioFileWriter_Ogg_WritePage (PlankFileRef file, const ogg_page* og)
{
    PlankResult result = PlankResult_OK;
    
    if ((result = pl_File_Write (file, og->header, og->header_len)) != PlankResult_OK) goto exit;
    if ((result = pl_File_Write (file, og->body,   og->body_len))   != PlankResult_OK) goto exit;
    
exit:
    return result;
}

#endif

#if PLANK_OGGVORBIS

#include "../../containers/plank_DynamicArray.h"
typedef struct PlankOggVorbisFileWriter
{
    PlankFile file;
    ogg_stream_state os;
    ogg_page og;
    ogg_packet op;
    vorbis_info vi;
    vorbis_comment vc;
    vorbis_dsp_state vd;
    vorbis_block vb;
} PlankOggVorbisFileWriter;

typedef PlankOggVorbisFileWriter* PlankOggVorbisFileWriterRef;

static PlankResult pl_AudioFileWriter_OggVorbis_CommentAddTag (PlankOggVorbisFileWriterRef p, const char* key, const char* string)
{    
    if (strlen (string) > 0)
    {
        vorbis_comment_add_tag (&p->vc, key, string);
    }
    
    return PlankResult_OK;
}

static PlankResult pl_AudioFileWriter_OggVorbis_Clear (PlankOggVorbisFileWriterRef p)
{
    PlankResult result;
    PlankMemoryRef m;
    
    result = PlankResult_OK;
    m = pl_MemoryGlobal();

    ogg_stream_clear (&p->os);
    vorbis_block_clear (&p->vb);
    vorbis_dsp_clear (&p->vd);
    vorbis_comment_clear (&p->vc);
    vorbis_info_clear (&p->vi);
    
    result = pl_Memory_Free (m, p);

    return result;
}

PlankResult pl_AudioFileWriter_OggVorbis_OpenInternal (PlankAudioFileWriterRef p, const char* filepath, PlankFileRef file)
{
    ogg_packet header;
    ogg_packet header_comm;
    ogg_packet header_code;

    PlankResult result;
    PlankOggVorbisFileWriterRef ogg;
    PlankRNGRef rng;
    PlankMemoryRef m;    
    int err, mode;
    
    result = PlankResult_OK;
    
    if (((filepath) && (file)) || ((filepath == 0) && (file == 0)))
    {
        result = PlankResult_UnknownError;
        goto exit;
    }
    
    if ((p->formatInfo.numChannels < 1) || (p->formatInfo.numChannels > 2))
    {
        result = PlankResult_AudioFileInavlidType;
        goto exit;
    }
    
    if (!(p->formatInfo.encoding & PLANKAUDIOFILE_ENCODING_FLOAT_FLAG))
    {
        result = PlankResult_AudioFileInavlidType;
        goto exit;
    }
        
    if (p->formatInfo.sampleRate <= 0.0)
    {
        result = PlankResult_AudioFileNotReady;
        goto exit;
    }
    
    m = pl_MemoryGlobal();
    ogg = (PlankOggVorbisFileWriterRef)pl_Memory_AllocateBytes (m, sizeof (PlankOggVorbisFileWriter));
    
    if (ogg == PLANK_NULL)
    {
        result = PlankResult_MemoryError;
        goto exit;
    }
    
    pl_MemoryZero (ogg, sizeof (PlankOggVorbisFileWriter));
    
    if (filepath)
    {
        if ((result = pl_File_Init ((PlankFileRef)ogg)) != PlankResult_OK) goto exit;
        if ((result = pl_File_OpenBinaryWrite ((PlankFileRef)ogg, filepath, PLANK_FALSE, PLANK_TRUE, PLANK_FALSE)) != PlankResult_OK) goto exit;
    }
    else
    {
        if ((result = pl_File_GetMode (file, &mode)) != PlankResult_OK) goto exit;

        if (mode & PLANKFILE_BIGENDIAN)
        {
            result = PlankResult_AudioFileInavlidType;
            goto exit;
        }
        
        if (!(mode & PLANKFILE_BINARY))
        {
            result = PlankResult_AudioFileInavlidType;
            goto exit;
        }
        
        if (!(mode & PLANKFILE_WRITE))
        {
            result = PlankResult_AudioFileInavlidType;
            goto exit;
        }

        pl_MemoryCopy (&ogg->file, file, sizeof (PlankFile));
        pl_MemoryZero (file, sizeof (PlankFile));
    }
    
    vorbis_info_init (&ogg->vi);
    
    if (p->formatInfo.nominalBitRate != 0)
    {
        err = vorbis_encode_init (&ogg->vi,
                                  p->formatInfo.numChannels,
                                  (int)p->formatInfo.sampleRate,
                                  p->formatInfo.maximumBitRate,
                                  p->formatInfo.nominalBitRate,
                                  p->formatInfo.minimumBitRate);
    }
    else
    {
        err = vorbis_encode_init_vbr (&ogg->vi,
                                      p->formatInfo.numChannels,
                                      (int)p->formatInfo.sampleRate,
                                      pl_ClipF (p->formatInfo.quality, 0.f, 1.f));
    }
    
    if (!err)
    {
        vorbis_comment_init (&ogg->vc);
        pl_AudioFileWriter_OggVorbis_CommentAddTag (ogg, "ENCODER", "Plink|Plonk|Plank");
        
        vorbis_analysis_init (&ogg->vd, &ogg->vi);
        vorbis_block_init (&ogg->vd, &ogg->vb);
        
        rng = pl_RNGGlobal();
        ogg_stream_init (&ogg->os, pl_RNG_Next (rng));
        
        vorbis_analysis_headerout (&ogg->vd, &ogg->vc, &header, &header_comm, &header_code);
        
        ogg_stream_packetin (&ogg->os, &header);
        ogg_stream_packetin (&ogg->os, &header_comm);
        ogg_stream_packetin (&ogg->os, &header_code);
        
        do
        {
            if ((err = ogg_stream_flush (&ogg->os, &ogg->og)) != 0)
            {
                if ((result = pl_AudioFileWriter_Ogg_WritePage ((PlankFileRef)ogg, &ogg->og)) != PlankResult_OK) goto exit;
            }
        } while (err != 0);
        
        p->peer = ogg;
        
        p->writeFramesFunction = pl_AudioFileWriter_OggVorbis_WriteFrames;
        p->writeHeaderFunction = 0;
    }

exit:
    if ((p->peer == 0) && (ogg != 0))
    {
        pl_AudioFileWriter_OggVorbis_Clear (ogg);
    }
    
    return result;
}

PlankResult pl_AudioFileWriter_OggVorbis_Open (PlankAudioFileWriterRef p, const char* filepath)
{
    return pl_AudioFileWriter_OggVorbis_OpenInternal (p, filepath, 0);
}

PlankResult pl_AudioFileWriter_OggVorbis_OpenWithFile (PlankAudioFileWriterRef p, PlankFileRef file)
{
    return pl_AudioFileWriter_OggVorbis_OpenInternal (p, 0, file);
}

static PlankResult pl_AudioFileWriter_OggVorbis_WriteData (PlankAudioFileWriterRef p, const int count)
{
    PlankResult result;
    PlankOggVorbisFileWriterRef ogg;
    int err;
    
    ogg = (PlankOggVorbisFileWriterRef)p->peer;

    vorbis_analysis_wrote (&ogg->vd, count);
    
    while (vorbis_analysis_blockout (&ogg->vd, &ogg->vb) == 1)
    {
        vorbis_analysis (&ogg->vb, 0);
        vorbis_bitrate_addblock (&ogg->vb);
        
        while (vorbis_bitrate_flushpacket (&ogg->vd, &ogg->op))
        {
            ogg_stream_packetin (&ogg->os, &ogg->op);
            
            do
            {
                if ((err = ogg_stream_pageout (&ogg->os, &ogg->og)) != 0)
                {
                    if ((result = pl_AudioFileWriter_Ogg_WritePage ((PlankFileRef)ogg, &ogg->og)) != PlankResult_OK) goto exit;
                }
            } while ((err != 0) && (ogg_page_eos (&ogg->og) == 0));
        }
    }
    
exit:
    return result;
}

PlankResult pl_AudioFileWriter_OggVorbis_Close (PlankAudioFileWriterRef p)
{
    PlankResult result;
    PlankOggVorbisFileWriterRef ogg;

    result = PlankResult_OK;
    ogg = (PlankOggVorbisFileWriterRef)p->peer;
    
    if (ogg)
    {
        pl_AudioFileWriter_OggVorbis_WriteData (p, 0);
        if ((result = pl_File_DeInit ((PlankFileRef)ogg)) != PlankResult_OK) goto exit;
        if ((result = pl_AudioFileWriter_OggVorbis_Clear (ogg)) != PlankResult_OK) goto exit;
    }
    
    
exit:
    return result;
}

PlankResult pl_AudioFileWriter_OggVorbis_WriteFrames (PlankAudioFileWriterRef p, const int numFrames, const void* data)
{
    PlankResult result;
    PlankOggVorbisFileWriterRef ogg;
    float** buffer;
    float* dst;
    const float* src;
    int channel, i, numChannels;
    
    result = PlankResult_OK;
    ogg = (PlankOggVorbisFileWriterRef)p->peer;

    if (numFrames > 0)
    {
        buffer = vorbis_analysis_buffer (&ogg->vd, numFrames);
        numChannels = (int)p->formatInfo.numChannels;
        
        for (channel = 0; channel < numChannels; ++channel)
        {
            src = (const float*)data + channel;
            dst = buffer[channel];
            
            for (i = 0; i < numFrames; ++i)
            {
                dst[i] = *src;
                src += numChannels;
            }
        }        
    }
    
    result = pl_AudioFileWriter_OggVorbis_WriteData (p, numFrames);
    
exit:
    return result;
}
#endif // PLANK_OGGVORBIS


#if PLANK_OPUS

#include "../../containers/plank_DynamicArray.h"
typedef struct PlankOpusFileWriter
{
    PlankFile file;
    OpusHeader header;
    ogg_stream_state os;
    ogg_page og;
    ogg_packet op;
    OpusMSEncoder* oe;
    
    int frameSize;
    int frameSize48kHz;
    PlankDynamicArray packet;
    PlankDynamicArray buffer;
    int bufferPos;
    
    int totalNumSegments;
    PlankLL totalPCMFrames;
    PlankLL currentGranulePos;
    PlankLL lastPageGranulePos;
    
} PlankOpusFileWriter;

typedef PlankOpusFileWriter* PlankOpusFileWriterRef;

static PlankResult pl_AudioFileWriter_Opus_Clear (PlankOpusFileWriterRef p)
{
    PlankResult result;
    PlankMemoryRef m;
    
    result = PlankResult_OK;
    m = pl_MemoryGlobal();
    
    ogg_stream_clear (&p->os);
    opus_multistream_encoder_destroy (p->oe);
    
    result = pl_Memory_Free (m, p);
    
    return result;
}

static void pl_AudioFileWriter_Opus_CommentAdd (PlankFileRef commentWriter, const char *tag, const char *value)
{
    int vendorLength, numComments, tagLength, valueLength;
    
    pl_File_SetPosition (commentWriter, 8);
    pl_File_ReadI (commentWriter, &vendorLength);
    
    pl_File_SetPosition (commentWriter, 8 + 4 + vendorLength);
    pl_File_ReadI (commentWriter, &numComments);

    tagLength = tag ? strlen (tag) + 1 : 0;
    valueLength = strlen (value);
    
    pl_File_SetPositionEnd (commentWriter);
    pl_File_WriteI (commentWriter, tagLength + valueLength);
    
    if (tag)
    {
        pl_File_Write  (commentWriter, tag, tagLength - 1);
        pl_File_WriteC (commentWriter, '=');
    }
    
    pl_File_Write (commentWriter, value, valueLength);

    pl_File_SetPosition (commentWriter, 8 + 4 + vendorLength);
    pl_File_WriteI (commentWriter, numComments + 1);
}

static PlankResult pl_AudioFileWriter_Opus_OpenInternal (PlankAudioFileWriterRef p, const char* filepath, PlankFileRef file)
{
    unsigned char headerData[100];
    const char* vendor;
    PlankResult result;
    PlankOpusFileWriterRef opus;
    PlankRNGRef rng;
    PlankDynamicArray comments;
    PlankFile commentWriter;
    PlankMemoryRef m;
    int err, delay, bitRate, streamCount, coupledStreamCount, quality, headerPacketSize, i, mode;
    opus_int32 sampleRate;
    
    result = PlankResult_OK;
    
    if (((filepath) && (file)) || ((filepath == 0) && (file == 0)))
    {
        result = PlankResult_UnknownError;
        goto exit;
    }
    
    if ((p->formatInfo.numChannels < 1) || (p->formatInfo.numChannels > 255))
    {
        result = PlankResult_AudioFileInavlidType;
        goto exit;
    }
    
    if (!(p->formatInfo.encoding & PLANKAUDIOFILE_ENCODING_FLOAT_FLAG))
    {
        result = PlankResult_AudioFileInavlidType;
        goto exit;
    }
        
    sampleRate = (opus_int32)p->formatInfo.sampleRate;
    
    switch (sampleRate)
    {
        case 8000: case 12000: case 16000: case 24000: case 48000: break;
        default:
            result = PlankResult_AudioFileUnsupportedType;
            goto exit;
    }
    
    m = pl_MemoryGlobal();
    opus = (PlankOpusFileWriterRef)pl_Memory_AllocateBytes (m, sizeof (PlankOpusFileWriter));
    
    if (opus == PLANK_NULL)
    {
        result = PlankResult_MemoryError;
        goto exit;
    }
    
    pl_MemoryZero (opus, sizeof (PlankOggVorbisFileWriter));
    
    if (filepath)
    {
        if ((result = pl_File_Init ((PlankFileRef)opus)) != PlankResult_OK) goto exit;
        if ((result = pl_File_OpenBinaryWrite ((PlankFileRef)opus, filepath, PLANK_FALSE, PLANK_TRUE, PLANK_FALSE)) != PlankResult_OK) goto exit;
    }
    else
    {
        if ((result = pl_File_GetMode (file, &mode)) != PlankResult_OK) goto exit;
        
        if (mode & PLANKFILE_BIGENDIAN)
        {
            result = PlankResult_AudioFileInavlidType;
            goto exit;
        }
        
        if (!(mode & PLANKFILE_BINARY))
        {
            result = PlankResult_AudioFileInavlidType;
            goto exit;
        }
        
        if (!(mode & PLANKFILE_WRITE))
        {
            result = PlankResult_AudioFileInavlidType;
            goto exit;
        }
        
        pl_MemoryCopy (&opus->file, file, sizeof (PlankFile));
        pl_MemoryZero (file, sizeof (PlankFile));
    }
    
    p->formatInfo.frameDuration = p->formatInfo.frameDuration == 0.f ? 0.02f : p->formatInfo.frameDuration;
    opus->frameSize48kHz        = (int)(p->formatInfo.frameDuration * 48000.f);
    
    switch (opus->frameSize48kHz) {
        case 120: case 240: case 480: case 960: case 1920: case 2880:
            break;
        default:
            opus->frameSize48kHz = 960;
            p->formatInfo.frameDuration = 0.02f;
    }
    
    streamCount             = p->formatInfo.numChannels;
    coupledStreamCount      = 0; // for now
    opus->frameSize         = opus->frameSize48kHz * sampleRate / 48000;
    quality                 = (int)p->formatInfo.quality;
    
    // for now..
    for (i = 0; i < p->formatInfo.numChannels; ++i)
        opus->header.stream_map[i] = i;
    
    if ((result = pl_DynamicArray_InitWithItemSizeAndSize (&opus->packet, 1, PLANKAUDIOFILE_OPUS_MAXPACKETSIZE * streamCount, PLANK_FALSE)) != PlankResult_OK) goto exit;
    if ((result = pl_DynamicArray_InitWithItemSizeAndSize (&opus->buffer, sizeof (float), p->formatInfo.numChannels * opus->frameSize, PLANK_FALSE)) != PlankResult_OK) goto exit;
    
    opus->bufferPos          = 0;
    opus->totalPCMFrames     = 0;
    opus->totalNumSegments   = 0;
    opus->currentGranulePos  = 0;
    opus->lastPageGranulePos = opus->currentGranulePos;
    
    opus->oe = opus_multistream_encoder_create (sampleRate,
                                                p->formatInfo.numChannels,  // channels
                                                p->formatInfo.numChannels,  // streams
                                                0,                          // coupled streams
                                                opus->header.stream_map,
                                                OPUS_APPLICATION_AUDIO,
                                                &err);
    if (err)
    {
        result = PlankResult_UnknownError;
        goto exit;
    }
    
    bitRate = (p->formatInfo.nominalBitRate == 0) ? 64000 * streamCount + 32000 * coupledStreamCount : p->formatInfo.nominalBitRate;

    if ((p->formatInfo.nominalBitRate != 0) &&
        (p->formatInfo.nominalBitRate == p->formatInfo.minimumBitRate) &&
        (p->formatInfo.nominalBitRate == p->formatInfo.maximumBitRate))
    {
        if ((err = opus_multistream_encoder_ctl (opus->oe, OPUS_SET_BITRATE (bitRate))) != 0) { result = PlankResult_UnknownError; goto exit; }
        if ((err = opus_multistream_encoder_ctl (opus->oe, OPUS_SET_VBR (0))) != 0) { result = PlankResult_UnknownError; goto exit; }
    }
    else
    {
        if ((err = opus_multistream_encoder_ctl (opus->oe, OPUS_SET_BITRATE (bitRate))) != 0) { result = PlankResult_UnknownError; goto exit; }
        if ((err = opus_multistream_encoder_ctl (opus->oe, OPUS_SET_COMPLEXITY (quality))) != 0) { result = PlankResult_UnknownError; goto exit; }
    }
    
    if ((err = opus_multistream_encoder_ctl (opus->oe, OPUS_GET_LOOKAHEAD (&delay))) != 0) { result = PlankResult_UnknownError; goto exit; }

    opus->header.version           = 1;
    opus->header.channels          = p->formatInfo.numChannels;
    opus->header.preskip           = delay * 48000 / sampleRate;
    opus->header.input_sample_rate = sampleRate;
    opus->header.gain              = 0;
    opus->header.channel_mapping   = 0;
    opus->header.nb_streams        = streamCount;
    opus->header.nb_coupled        = coupledStreamCount;
        
    rng = pl_RNGGlobal();
    ogg_stream_init (&opus->os, pl_RNG_Next (rng));

    headerPacketSize = opus_header_to_packet (&opus->header, headerData, 100);
    
    opus->op.packet     = headerData;
    opus->op.bytes      = headerPacketSize;
    opus->op.b_o_s      = 1;
    opus->op.e_o_s      = 0;
    opus->op.granulepos = opus->currentGranulePos;
    opus->op.packetno   = 0;
    ogg_stream_packetin (&opus->os, &opus->op);
    
    do
    {
        if ((err = ogg_stream_flush (&opus->os, &opus->og)) != 0)
        {
            if ((result = pl_AudioFileWriter_Ogg_WritePage ((PlankFileRef)opus, &opus->og)) != PlankResult_OK) goto exit;
        }
    } while (err != 0);
    
    pl_DynamicArray_InitWithItemSizeAndCapacity (&comments, 1, 128);
    pl_File_Init (&commentWriter);
    pl_File_OpenDynamicArray (&commentWriter, &comments, PLANKFILE_BINARY | PLANKFILE_WRITE | PLANKFILE_READ);
    
    pl_File_WriteString (&commentWriter, "OpusTags");
    vendor = PLANKAUDIOFILE_OPUS_VENDOR;//opus_get_version_string();
    
    pl_File_WriteI (&commentWriter, strlen (vendor));
    pl_File_WriteString (&commentWriter, vendor);
    pl_File_WriteI (&commentWriter, 0);

    pl_AudioFileWriter_Opus_CommentAdd (&commentWriter, "ENCODER", "Plink|Plonk|Plank");
    
    opus->op.packet     = (unsigned char *)pl_DynamicArray_GetArray (&comments);
    opus->op.bytes      = pl_DynamicArray_GetSize(&comments);
    opus->op.b_o_s      = 0;
    opus->op.e_o_s      = 0;
    opus->op.granulepos = opus->currentGranulePos;
    opus->op.packetno   = 1;
    ogg_stream_packetin (&opus->os, &opus->op);
    
    do
    {
        if ((err = ogg_stream_flush (&opus->os, &opus->og)) != 0)
        {
            if ((result = pl_AudioFileWriter_Ogg_WritePage ((PlankFileRef)opus, &opus->og)) != PlankResult_OK) goto exit;
        }
    } while (err != 0);
    
    pl_File_DeInit (&commentWriter);
    pl_DynamicArray_DeInit (&comments);
    
    p->peer = opus;
    
    p->writeFramesFunction = pl_AudioFileWriter_Opus_WriteFrames;
    p->writeHeaderFunction = 0; // no header that needs rewriting later
    
exit:
    if ((p->peer == 0) && (opus != 0))
    {
        pl_AudioFileWriter_Opus_Clear (opus);
    }
    
    return result;
}

PlankResult pl_AudioFileWriter_Opus_Open (PlankAudioFileWriterRef p, const char* filepath)
{
    return pl_AudioFileWriter_Opus_OpenInternal (p, filepath, 0);
}

PlankResult pl_AudioFileWriter_Opus_OpenWithFile (PlankAudioFileWriterRef p, PlankFileRef file)
{
    return pl_AudioFileWriter_Opus_OpenInternal (p, 0, file);
}

static PlankResult pl_AudioFileWriter_Opus_WritePage (PlankOpusFileWriterRef opus)
{
    PlankResult result = PlankResult_OK;
    
    if (ogg_page_packets (&opus->og) != 0)
        opus->lastPageGranulePos = ogg_page_granulepos (&opus->og);
    
    opus->totalNumSegments -= opus->og.header[PALNKAUDIOFILE_OPUS_HEADERNUMSEGSPOS];
    
    if ((result = pl_AudioFileWriter_Ogg_WritePage ((PlankFileRef)opus, &opus->og)) != PlankResult_OK) goto exit;
    
exit:
    return result;
}

static PlankResult pl_AudioFileWriter_Opus_WriteBuffer (PlankOpusFileWriterRef opus)
{
    PlankResult result;
    int ret, bufferLength, numSegments;
    float* buffer;
    unsigned char* packetData;
    opus_int32 packetLength;
    ogg_uint32_t sampleRate;
    int maxOggDelay;
    int numFramesAt48K;
    
    maxOggDelay = PLANKAUDIOFILE_OPUS_MAXFRAMESIZE;  // for now
    
    result       = PlankResult_OK;
    buffer       = (float*)pl_DynamicArray_GetArray (&opus->buffer);
    bufferLength = (int)pl_DynamicArray_GetSize (&opus->buffer);
    packetData   = (unsigned char*)pl_DynamicArray_GetArray (&opus->packet);
    packetLength = (opus_int32)pl_DynamicArray_GetSize (&opus->packet);
    sampleRate   = opus->header.input_sample_rate;

    opus->totalPCMFrames += opus->bufferPos;
    
    if (opus->bufferPos < bufferLength)
    {
        // is last buffer - pad with zeros
        pl_MemoryZero (buffer + opus->bufferPos, (bufferLength - opus->bufferPos) * sizeof (float));
    }

    ret = opus_multistream_encode_float (opus->oe, (float*)buffer, bufferLength, packetData, packetLength);
    
    if (ret < 0)
    {
        result = PlankResult_UnknownError;
        goto exit;
    }
    
    numSegments = (ret + PLANKAUDIOFILE_OPUS_MAXSEGMENTS) / PLANKAUDIOFILE_OPUS_MAXSEGMENTS;
    numFramesAt48K = opus->frameSize * 48000 / sampleRate;
    opus->currentGranulePos += numFramesAt48K;
    
    while ((((numSegments <= PLANKAUDIOFILE_OPUS_MAXSEGMENTS) && (opus->totalNumSegments + numSegments > PLANKAUDIOFILE_OPUS_MAXSEGMENTS)) || (opus->currentGranulePos - opus->lastPageGranulePos > maxOggDelay)) &&
          ogg_stream_flush_fill (&opus->os, &opus->og, PLANKAUDIOFILE_OPUS_FLUSHFILLSIZE))
    {        
        if ((result = pl_AudioFileWriter_Ogg_WritePage ((PlankFileRef)opus, &opus->og)) != PlankResult_OK) goto exit;
    }
    
    opus->op.packet     = packetData;
    opus->op.bytes      = ret;
    opus->op.b_o_s      = 0;
    opus->op.e_o_s      = opus->bufferPos < bufferLength ? 1 : 0;
    opus->op.granulepos = opus->op.e_o_s ? ((opus->totalPCMFrames * 48000 + sampleRate - 1) / sampleRate) + opus->header.preskip : opus->currentGranulePos;
        
    opus->op.packetno++;
    ogg_stream_packetin (&opus->os, &opus->op);
    opus->totalNumSegments += numSegments;
    
    while ((opus->op.e_o_s || (opus->currentGranulePos + numFramesAt48K - opus->lastPageGranulePos > maxOggDelay) || (opus->totalNumSegments >= PLANKAUDIOFILE_OPUS_MAXSEGMENTS)) ?
           ogg_stream_flush_fill (&opus->os, &opus->og, PLANKAUDIOFILE_OPUS_FLUSHFILLSIZE) : ogg_stream_pageout_fill (&opus->os, &opus->og, PLANKAUDIOFILE_OPUS_FLUSHFILLSIZE))
    {
        if ((result = pl_AudioFileWriter_Opus_WritePage (opus)) != PlankResult_OK) goto exit;
    }
    
exit:
    return result;
}

PlankResult pl_AudioFileWriter_Opus_Close (PlankAudioFileWriterRef p)
{
    PlankResult result;
    PlankOpusFileWriterRef opus;
    
    result = PlankResult_OK;
    opus = (PlankOpusFileWriterRef)p->peer;
    
    if (opus)
    {
        if ((result = pl_AudioFileWriter_Opus_WriteBuffer (opus)) != PlankResult_OK) goto exit;

        if ((result = pl_File_DeInit ((PlankFileRef)opus)) != PlankResult_OK) goto exit;
        if ((result = pl_DynamicArray_DeInit (&opus->packet)) != PlankResult_OK) goto exit;
        if ((result = pl_DynamicArray_DeInit (&opus->buffer)) != PlankResult_OK) goto exit;
        if ((result = pl_AudioFileWriter_Opus_Clear (opus)) != PlankResult_OK) goto exit;
    }
    
exit:
    return result;    
}

PlankResult pl_AudioFileWriter_Opus_WriteFrames (PlankAudioFileWriterRef p, const int numFrames, const void* data)
{
    PlankResult result;
    PlankOpusFileWriterRef opus;
    float* buffer;
    const float* src;
    int bufferLength, bufferRemaining, numSamplesRemaining, bufferThisTime;
    int numChannels, numSamples;
    
    result = PlankResult_OK;
    opus = (PlankOpusFileWriterRef)p->peer;
    
    numChannels         = p->formatInfo.numChannels;
    buffer              = (float*)pl_DynamicArray_GetArray (&opus->buffer);
    bufferLength        = (int)pl_DynamicArray_GetSize (&opus->buffer);
    bufferRemaining     = bufferLength - opus->bufferPos;
    numSamples          = numFrames * numChannels;
    numSamplesRemaining = numSamples;
    src                 = (const float*)data;
    
    while (numSamplesRemaining > 0)
    {
        bufferThisTime = pl_MinI (bufferRemaining, numSamplesRemaining);
        pl_MemoryCopy (buffer + opus->bufferPos, src, bufferThisTime * sizeof (float));
        
        bufferRemaining -= bufferThisTime;
        opus->bufferPos += bufferThisTime;
        numSamplesRemaining -= bufferThisTime;
        src += bufferThisTime;
        
        if (opus->bufferPos == bufferLength)
        {
            result = pl_AudioFileWriter_Opus_WriteBuffer (opus);
            bufferRemaining = bufferLength;
            opus->bufferPos = 0;
        }
    }
        
exit:
    return result;
}

#endif // PLANK_OPUS

static PlankResult pl_AudioFileWriter_WAV_WriteChunk_bext (PlankAudioFileWriterRef p)
{
//    PlankIffFileWriterChunkInfoRef chunkInfo;
    PlankResult result = PlankResult_OK;
    PlankIffFileWriterRef iff;
    char description [257];
    char originator [33];
    char originatorRef [33];
    char originationDate [11];
    char originationTime [9];
    const char* string;
    const char* codingHistory;
    const char* bext;
    int stringLength, codingHistoryLength;
    PlankUI chunkSize;
    PlankLL timeRef;
    
    if (!p->metaData)
        goto exit;
    
    iff  = (PlankIffFileWriterRef)p->peer;
    
    pl_MemoryZero (description, 257);
    pl_MemoryZero (originator, 33);
    pl_MemoryZero (originatorRef, 33);
    pl_MemoryZero (originationDate, 11);
    pl_MemoryZero (originationTime, 9);
    
    if ((string = pl_AudioFileMetaData_GetDescriptionComment (p->metaData, 0)))
    {
        stringLength = strlen (string);
        
        if (stringLength)
            pl_MemoryCopy (description, string, pl_MinI (stringLength, 256));
    }

    if ((string = pl_AudioFileMetaData_GetOriginatorArtist (p->metaData)))
    {
        stringLength = strlen (string);
        
        if (stringLength)
            pl_MemoryCopy (originator, string, pl_MinI (stringLength, 32));
    }

    if ((string = pl_AudioFileMetaData_GetOriginatorRef (p->metaData)))
    {
        stringLength = strlen (string);
        
        if (stringLength)
            pl_MemoryCopy (originatorRef, string, pl_MinI (stringLength, 32));
    }
    
    if ((string = pl_AudioFileMetaData_GetOriginationDate (p->metaData)))
    {
        stringLength = strlen (string);
        
        if (stringLength)
            pl_MemoryCopy (originationDate, string, pl_MinI (stringLength, 10));
    }

    if ((string = pl_AudioFileMetaData_GetOriginationTime (p->metaData)))
    {
        stringLength = strlen (string);
        
        if (stringLength)
            pl_MemoryCopy (originationTime, string, pl_MinI (stringLength, 8));
    }

    if ((description[0] + originator[0] + originatorRef[0] + originationDate[0] + originationTime[0]) > 0)
    {
        // assume we have BEXT
        bext = "bext";
        
        chunkSize = 256 + 32 + 32 + 10 + 8 + 4 + 4 + 4 + 64 + 190;
        
        codingHistory       = pl_AudioFileMetaData_GetCodingHistory (p->metaData);
        codingHistoryLength = codingHistory ? strlen (codingHistory) + 1 : 0;
        chunkSize          += codingHistoryLength;

        if ((result = pl_IffFileWriter_PrepareChunk (iff, p->metaDataChunkPosition, bext, 0, chunkSize)) != PlankResult_OK) goto exit;

        
//        if ((result = pl_IffFileWriter_SeekChunk (iff, p->metaDataChunkPosition, JUNK, &chunkInfo, 0)) != PlankResult_OK) goto exit;
//        
//        if (chunkInfo)
//        {
//            if ((result = pl_IffFileWriter_ResizeChunk (iff, p->metaDataChunkPosition, JUNK, chunkSize, PLANK_FALSE)) != PlankResult_OK) goto exit;
//            if ((result = pl_IffFileWriter_RenameChunk (iff, p->metaDataChunkPosition, JUNK, bext)) != PlankResult_OK) goto exit;
//            if ((result = pl_IffFileWriter_SeekChunk (iff,  p->metaDataChunkPosition, bext, &chunkInfo, 0)) != PlankResult_OK) goto exit;
//        }
//        else
//        {
//            if ((result = pl_IffFileWriter_WriteChunk (iff, p->metaDataChunkPosition, bext, 0, chunkSize, PLANKIFFFILEWRITER_MODEREPLACEGROW)) != PlankResult_OK) goto exit;
//            if ((result = pl_IffFileWriter_SeekChunk (iff,  p->metaDataChunkPosition, bext, &chunkInfo, 0)) != PlankResult_OK) goto exit;
//        }

        if ((result = pl_File_Write ((PlankFileRef)iff, description, 256)) != PlankResult_OK) goto exit;
        if ((result = pl_File_Write ((PlankFileRef)iff, originator, 32)) != PlankResult_OK) goto exit;
        if ((result = pl_File_Write ((PlankFileRef)iff, originatorRef, 32)) != PlankResult_OK) goto exit;
        if ((result = pl_File_Write ((PlankFileRef)iff, originationDate, 10)) != PlankResult_OK) goto exit;
        if ((result = pl_File_Write ((PlankFileRef)iff, originationTime, 8)) != PlankResult_OK) goto exit;
        
        timeRef = pl_AudioFileMetaData_GetTimeRef (p->metaData);
        
        if ((result = pl_File_WriteUI ((PlankFileRef)iff, (PlankUI)((*(PlankULL*)&timeRef) & 0xFFFFFFFF))) != PlankResult_OK) goto exit;
        if ((result = pl_File_WriteUI ((PlankFileRef)iff, (PlankUI)((*(PlankULL*)&timeRef) >> 32))) != PlankResult_OK) goto exit;
        if ((result = pl_File_WriteUS ((PlankFileRef)iff, 1)) != PlankResult_OK) goto exit; // Version 1?
        
        if ((result = pl_File_Write ((PlankFileRef)iff, pl_AudioFileMetaData_GetUMID (p->metaData), 64)) != PlankResult_OK) goto exit;
        if ((result = pl_File_WriteZeros ((PlankFileRef)iff, 190)) != PlankResult_OK) goto exit;
        
        if (codingHistoryLength > 0)
        {
            if ((result = pl_File_Write ((PlankFileRef)iff, codingHistory, codingHistoryLength)) != PlankResult_OK) goto exit;

            if (codingHistoryLength & 1)
            {
                if ((result = pl_File_WriteUC ((PlankFileRef)iff, 0)) != PlankResult_OK) goto exit;
            }
        }
        
        p->dataPosition = pl_MaxLL (p->dataPosition, pl_AlignLL (pl_IffFileWriter_GetCurrentChunk (iff)->chunkPos + chunkSize, iff->common.headerInfo.alignment) + 8);
    }
    
exit:
    return result;
}

static PlankResult pl_AudioFileWriter_WAV_WriteChunk_smpl (PlankAudioFileWriterRef p)
{
//    PlankIffFileWriterChunkInfoRef chunkInfo;
    PlankResult result = PlankResult_OK;
    PlankIffFileWriterRef iff;
    const char* smpl;
    PlankUI smplChunkSize;
    PlankUI manufacturer, product, samplePeriod, smpteFormat, smpteOffset;
    PlankI baseNote, detune;
    PlankDynamicArrayRef loopPoints;
    PlankAudioFileRegion* loopArray;
    PlankDynamicArrayRef extraSamplerData;
    PlankUI numLoops, i, extraSamplerDataSize;

    if (!p->metaData)
        goto exit;
    
    iff  = (PlankIffFileWriterRef)p->peer;

    loopPoints = pl_AudioFileMetaData_GetLoopPoints (p->metaData);
    numLoops   = pl_DynamicArray_GetSize (loopPoints);
    
    if ((result = pl_AudioFileMetaData_GetSamplerData (p->metaData, &manufacturer, &product, &samplePeriod, &smpteFormat, &smpteOffset)) != PlankResult_OK) goto exit;
    if ((result = pl_AudioFileMetaData_GetInstrumentData (p->metaData, &baseNote, &detune, 0, 0, 0, 0, 0)) != PlankResult_OK) goto exit;
    
    if ((manufacturer | product | samplePeriod | smpteFormat | smpteOffset | numLoops) != 0)
    {
        smpl = "smpl";
        
        extraSamplerData = pl_AudioFileMetaData_GetExtraSamplerData (p->metaData);
        extraSamplerDataSize = pl_DynamicArray_GetSize (extraSamplerData);
        
        smplChunkSize = 36 + (numLoops * 24) + extraSamplerDataSize;

        if ((result = pl_IffFileWriter_PrepareChunk (iff, p->metaDataChunkPosition, smpl, 0, smplChunkSize)) != PlankResult_OK) goto exit;

//        if ((result = pl_IffFileWriter_SeekChunk (iff, p->metaDataChunkPosition, JUNK, &chunkInfo, 0)) != PlankResult_OK) goto exit;
//        
//        if (chunkInfo)
//        {
//            if ((result = pl_IffFileWriter_ResizeChunk (iff, p->metaDataChunkPosition, JUNK, smplChunkSize, PLANK_FALSE)) != PlankResult_OK) goto exit;
//            if ((result = pl_IffFileWriter_RenameChunk (iff, p->metaDataChunkPosition, JUNK, smpl)) != PlankResult_OK) goto exit;
//            if ((result = pl_IffFileWriter_SeekChunk (iff,  p->metaDataChunkPosition, smpl, &chunkInfo, 0)) != PlankResult_OK) goto exit;
//        }
//        else
//        {
//            if ((result = pl_IffFileWriter_WriteChunk (iff, p->metaDataChunkPosition, smpl, 0, smplChunkSize, PLANKIFFFILEWRITER_MODEREPLACEGROW)) != PlankResult_OK) goto exit;
//            if ((result = pl_IffFileWriter_SeekChunk (iff,  p->metaDataChunkPosition, smpl, &chunkInfo, 0)) != PlankResult_OK) goto exit;
//        }
        
        if (!samplePeriod)
            samplePeriod = 1000000000.0 / p->formatInfo.sampleRate;
    
        if ((result = pl_File_WriteUI ((PlankFileRef)iff, manufacturer)) != PlankResult_OK) goto exit;
        if ((result = pl_File_WriteUI ((PlankFileRef)iff, product)) != PlankResult_OK) goto exit;
        if ((result = pl_File_WriteUI ((PlankFileRef)iff, samplePeriod)) != PlankResult_OK) goto exit;
        if ((result = pl_File_WriteUI ((PlankFileRef)iff, baseNote)) != PlankResult_OK) goto exit;
        if ((result = pl_File_WriteUI ((PlankFileRef)iff, (pl_ClipI (detune, -50, 50) * 32768) / 50)) != PlankResult_OK) goto exit;
        if ((result = pl_File_WriteUI ((PlankFileRef)iff, smpteFormat)) != PlankResult_OK) goto exit;
        if ((result = pl_File_WriteUI ((PlankFileRef)iff, smpteOffset)) != PlankResult_OK) goto exit;
        if ((result = pl_File_WriteUI ((PlankFileRef)iff, numLoops)) != PlankResult_OK) goto exit;
        if ((result = pl_File_WriteUI ((PlankFileRef)iff, 0)) != PlankResult_OK) goto exit; // sampler data

        loopArray = (PlankAudioFileRegion*)pl_DynamicArray_GetArray (loopPoints);

        for (i = 0; i < numLoops; ++i)
        {
            if ((result = pl_File_WriteUI ((PlankFileRef)iff, loopArray[i].anchor.cueID)) != PlankResult_OK) goto exit;
            if ((result = pl_File_WriteUI ((PlankFileRef)iff, (PlankUI)loopArray[i].regionOptions)) != PlankResult_OK) goto exit;
            if ((result = pl_File_WriteUI ((PlankFileRef)iff, (PlankUI)loopArray[i].start.position)) != PlankResult_OK) goto exit;
            if ((result = pl_File_WriteUI ((PlankFileRef)iff, (PlankUI)loopArray[i].end.position)) != PlankResult_OK) goto exit;
            if ((result = pl_File_WriteUI ((PlankFileRef)iff, (PlankUI)loopArray[i].fraction)) != PlankResult_OK) goto exit;
            if ((result = pl_File_WriteUI ((PlankFileRef)iff, (PlankUI)loopArray[i].playCount)) != PlankResult_OK) goto exit;
        }

        if (extraSamplerDataSize > 0)
        {
            if ((result = pl_File_Write ((PlankFileRef)iff, pl_DynamicArray_GetSize (extraSamplerData), extraSamplerDataSize)) != PlankResult_OK) goto exit;
            
            if (extraSamplerDataSize & 1)
            {
                if ((result = pl_File_WriteUC ((PlankFileRef)iff, 0)) != PlankResult_OK) goto exit;
            }
        }
        
        p->dataPosition = pl_MaxLL (p->dataPosition, pl_AlignLL (pl_IffFileWriter_GetCurrentChunk (iff)->chunkPos + smplChunkSize, iff->common.headerInfo.alignment) + 8);
    }
    
exit:
    return result;
}

static PlankResult pl_AudioFileWriter_WAV_WriteChunk_inst (PlankAudioFileWriterRef p)
{
//    PlankIffFileWriterChunkInfoRef chunkInfo;
    PlankResult result = PlankResult_OK;
    PlankIffFileWriterRef iff;
    const char* inst;
    PlankUI instChunkSize;
    PlankI baseNote, detune, gain, lowNote, highNote, lowVelocity, highVelocity;
    
    if (!p->metaData)
        goto exit;
    
    iff  = (PlankIffFileWriterRef)p->peer;
        
    if ((result = pl_AudioFileMetaData_GetInstrumentData (p->metaData, &baseNote, &detune, &gain, &lowNote, &highNote, &lowVelocity, &highVelocity)) != PlankResult_OK) goto exit;
    
    if (baseNote > 0)
    {
        inst = "inst";
        
        instChunkSize = 7;

        if ((result = pl_IffFileWriter_PrepareChunk (iff, p->metaDataChunkPosition, inst, 0, instChunkSize)) != PlankResult_OK) goto exit;

//        if ((result = pl_IffFileWriter_SeekChunk (iff, p->metaDataChunkPosition, JUNK, &chunkInfo, 0)) != PlankResult_OK) goto exit;
//        
//        if (chunkInfo)
//        {
//            if ((result = pl_IffFileWriter_ResizeChunk (iff, p->metaDataChunkPosition, JUNK, instChunkSize, PLANK_FALSE)) != PlankResult_OK) goto exit;
//            if ((result = pl_IffFileWriter_RenameChunk (iff, p->metaDataChunkPosition, JUNK, inst)) != PlankResult_OK) goto exit;
//            if ((result = pl_IffFileWriter_SeekChunk (iff,  p->metaDataChunkPosition, inst, &chunkInfo, 0)) != PlankResult_OK) goto exit;
//        }
//        else
//        {
//            if ((result = pl_IffFileWriter_WriteChunk (iff, p->metaDataChunkPosition, inst, 0, instChunkSize, PLANKIFFFILEWRITER_MODEREPLACEGROW)) != PlankResult_OK) goto exit;
//            if ((result = pl_IffFileWriter_SeekChunk (iff,  p->metaDataChunkPosition, inst, &chunkInfo, 0)) != PlankResult_OK) goto exit;
//        }
        
        if ((result = pl_File_ReadC ((PlankFileRef)iff, (PlankC)pl_ClipI (baseNote, 0, 127))) != PlankResult_OK) goto exit;
        if ((result = pl_File_ReadC ((PlankFileRef)iff, (PlankC)pl_ClipI (detune, -50, 50))) != PlankResult_OK) goto exit;
        if ((result = pl_File_ReadC ((PlankFileRef)iff, (PlankC)pl_ClipI (gain, -64, 64))) != PlankResult_OK) goto exit;
        if ((result = pl_File_ReadC ((PlankFileRef)iff, (PlankC)pl_ClipI (lowNote, 0, 127))) != PlankResult_OK) goto exit;
        if ((result = pl_File_ReadC ((PlankFileRef)iff, (PlankC)pl_ClipI (highNote, 0, 127))) != PlankResult_OK) goto exit;
        if ((result = pl_File_ReadC ((PlankFileRef)iff, (PlankC)pl_ClipI (lowVelocity, 0, 127))) != PlankResult_OK) goto exit;
        if ((result = pl_File_ReadC ((PlankFileRef)iff, (PlankC)pl_ClipI (highVelocity, 0, 127))) != PlankResult_OK) goto exit;
        
        if ((result = pl_File_WriteUC ((PlankFileRef)iff, 0)) != PlankResult_OK) goto exit; // align to 8 bytes
        
        p->dataPosition = pl_MaxLL (p->dataPosition, pl_AlignLL (pl_IffFileWriter_GetCurrentChunk (iff)->chunkPos + instChunkSize, iff->common.headerInfo.alignment) + 8);
    }
    
exit:
    return result;
}

static PlankResult pl_AudioFileWriter_WAV_WriteChunk_cue (PlankAudioFileWriterRef p)
{
//    PlankIffFileWriterChunkInfoRef chunkInfo;
    PlankResult result = PlankResult_OK;
    PlankIffFileWriterRef iff;
    PlankDynamicArrayRef cuePoints;
    PlankAudioFileCuePoint* cueArray;
    PlankDynamicArrayRef regions;
    PlankAudioFileRegion* regionArray;
    PlankDynamicArrayRef loopPoints;
    PlankAudioFileRegion* loopArray;
    PlankLL cueChunkSize;
    int numCues, numRegions, numLoops, totalCues, i;
    const char* cue;
    PlankFourCharCode dataID;
    
    if (!p->metaData)
        goto exit;
    
    iff  = (PlankIffFileWriterRef)p->peer;
    
    cuePoints  = pl_AudioFileMetaData_GetCuePoints (p->metaData);
    numCues    = pl_DynamicArray_GetSize (cuePoints);
    regions    = pl_AudioFileMetaData_GetRegions (p->metaData);
    numRegions = pl_DynamicArray_GetSize (regions);
    loopPoints = pl_AudioFileMetaData_GetLoopPoints (p->metaData);
    numLoops   = pl_DynamicArray_GetSize (loopPoints);
    totalCues  = numCues + numRegions + numLoops;
        
    if (totalCues > 0)
    {
        cue  = "cue ";
        cueChunkSize = totalCues * 24 + 4;

        if ((result = pl_IffFileWriter_PrepareChunk (iff, p->metaDataChunkPosition, cue, 0, cueChunkSize)) != PlankResult_OK) goto exit;

//        if ((result = pl_IffFileWriter_SeekChunk (iff, p->metaDataChunkPosition, JUNK, &chunkInfo, 0)) != PlankResult_OK) goto exit;
//        
//        if (chunkInfo)
//        {
//            if ((result = pl_IffFileWriter_ResizeChunk (iff, p->metaDataChunkPosition, JUNK, cueChunkSize, PLANK_FALSE)) != PlankResult_OK) goto exit;
//            if ((result = pl_IffFileWriter_RenameChunk (iff, p->metaDataChunkPosition, JUNK, cue)) != PlankResult_OK) goto exit;
//            if ((result = pl_IffFileWriter_SeekChunk (iff,  p->metaDataChunkPosition, cue, &chunkInfo, 0)) != PlankResult_OK) goto exit;
//        }
//        else
//        {
//            if ((result = pl_IffFileWriter_WriteChunk (iff, p->metaDataChunkPosition, cue, 0, cueChunkSize, PLANKIFFFILEWRITER_MODEREPLACEGROW)) != PlankResult_OK) goto exit;
//            if ((result = pl_IffFileWriter_SeekChunk (iff,  p->metaDataChunkPosition, cue, &chunkInfo, 0)) != PlankResult_OK) goto exit;
//        }
        
        cueArray = (PlankAudioFileCuePoint*)pl_DynamicArray_GetArray (cuePoints);
        
        if ((result = pl_File_WriteUI ((PlankFileRef)iff, (PlankUI)totalCues)) != PlankResult_OK) goto exit;
        
        dataID = pl_FourCharCode ("data");
        
        for (i = 0; i < numCues; ++i)
        {
            if ((result = pl_File_WriteUI ((PlankFileRef)iff, cueArray[i].cueID)) != PlankResult_OK) goto exit;
            if ((result = pl_File_WriteUI ((PlankFileRef)iff, 0 /*order*/)) != PlankResult_OK) goto exit;
            if ((result = pl_File_WriteFourCharCode ((PlankFileRef)iff, dataID)) != PlankResult_OK) goto exit;
            if ((result = pl_File_WriteUI ((PlankFileRef)iff, 0 /*chunkStart*/)) != PlankResult_OK) goto exit;
            if ((result = pl_File_WriteUI ((PlankFileRef)iff, 0 /*blockStart*/)) != PlankResult_OK) goto exit;
            if ((result = pl_File_WriteUI ((PlankFileRef)iff, (PlankUI)cueArray[i].position)) != PlankResult_OK) goto exit;
        }
        
        regionArray = (PlankAudioFileRegion*)pl_DynamicArray_GetArray (regions);
        
        for (i = 0; i < numRegions; ++i)
        {
            if ((result = pl_File_WriteUI ((PlankFileRef)iff, regionArray[i].start.cueID)) != PlankResult_OK) goto exit;
            if ((result = pl_File_WriteUI ((PlankFileRef)iff, 0 /*order*/)) != PlankResult_OK) goto exit;
            if ((result = pl_File_WriteFourCharCode ((PlankFileRef)iff, dataID)) != PlankResult_OK) goto exit;
            if ((result = pl_File_WriteUI ((PlankFileRef)iff, 0 /*chunkStart*/)) != PlankResult_OK) goto exit;
            if ((result = pl_File_WriteUI ((PlankFileRef)iff, 0 /*blockStart*/)) != PlankResult_OK) goto exit;
            if ((result = pl_File_WriteUI ((PlankFileRef)iff, (PlankUI)regionArray[i].start.position)) != PlankResult_OK) goto exit;
        }
        
        loopArray = (PlankAudioFileRegion*)pl_DynamicArray_GetArray (loopPoints);
        
        for (i = 0; i < numLoops; ++i)
        {
            if ((result = pl_File_WriteUI ((PlankFileRef)iff, loopArray[i].anchor.cueID)) != PlankResult_OK) goto exit;
            if ((result = pl_File_WriteUI ((PlankFileRef)iff, 0 /*order*/)) != PlankResult_OK) goto exit;
            if ((result = pl_File_WriteFourCharCode ((PlankFileRef)iff, dataID)) != PlankResult_OK) goto exit;
            if ((result = pl_File_WriteUI ((PlankFileRef)iff, 0 /*chunkStart*/)) != PlankResult_OK) goto exit;
            if ((result = pl_File_WriteUI ((PlankFileRef)iff, 0 /*blockStart*/)) != PlankResult_OK) goto exit;
            if ((result = pl_File_WriteUI ((PlankFileRef)iff, (PlankUI)loopArray[i].anchor.position)) != PlankResult_OK) goto exit;
        }
        
        p->dataPosition = pl_MaxLL (p->dataPosition, pl_AlignLL (pl_IffFileWriter_GetCurrentChunk (iff)->chunkPos + cueChunkSize, iff->common.headerInfo.alignment) + 8);
    }
    
exit:
    return result;
}

static PlankResult pl_AudioFileWriter_WAV_WriteChunk_list (PlankAudioFileWriterRef p)
{
    PlankResult result = PlankResult_OK;
    PlankIffFileWriterRef iff;
    PlankDynamicArrayRef cuePoints;
    PlankAudioFileCuePoint* cueArray;
    PlankDynamicArrayRef regions;
    PlankAudioFileRegion* regionArray;
    PlankDynamicArray chunkData;
    PlankFile chunkStream;
    PlankLL listChunkSize;
    int numCues, numRegions, i;
    const char* list;
    const char* adtl;
    const char* labl;
    const char* ltxt;
    const char* string;
    unsigned int stringSize;
    
    if (!p->metaData)
        goto exit;
    
    list = "list";
    adtl = "adtl";
    labl = "labl";
    ltxt = "ltxt";
    
    iff  = (PlankIffFileWriterRef)p->peer;
    
    cuePoints  = pl_AudioFileMetaData_GetCuePoints (p->metaData);
    numCues    = pl_DynamicArray_GetSize (cuePoints);
    regions    = pl_AudioFileMetaData_GetRegions (p->metaData);
    numRegions = pl_DynamicArray_GetSize (regions);
    
    if ((numCues + numRegions) > 0)
    {
        if ((result = pl_DynamicArray_InitWithItemSizeAndCapacity (&chunkData, 1, 512)) != PlankResult_OK) goto exit;
        
        pl_File_Init (&chunkStream);
        pl_File_OpenDynamicArray (&chunkStream, &chunkData, PLANKFILE_BINARY | PLANKFILE_READ | PLANKFILE_WRITE);
        
        if ((result = pl_File_WriteFourCharCode (&chunkStream, pl_FourCharCode (adtl))) != PlankResult_OK) goto exit;
        
        cueArray = (PlankAudioFileCuePoint*)pl_DynamicArray_GetArray (cuePoints);
        
        for (i = 0; i < numCues; ++i)
        {
            string = pl_AudioFileCuePoint_GetLabel (&cueArray[i]);
            if (!string) continue;
            
            stringSize = strlen (string) + 1;
            if (!stringSize) continue;
            
            if ((result = pl_File_WriteFourCharCode (&chunkStream, pl_FourCharCode (labl))) != PlankResult_OK) goto exit;
            if ((result = pl_File_WriteUI (&chunkStream, 4 + stringSize + (stringSize & 1))) != PlankResult_OK) goto exit;
            if ((result = pl_File_WriteUI (&chunkStream, pl_AudioFileCuePoint_GetID (&cueArray[i]))) != PlankResult_OK) goto exit;
            if ((result = pl_File_Write (&chunkStream, string, stringSize)) != PlankResult_OK) goto exit;
            
            if (stringSize & 1)
            {
                if ((result = pl_File_WriteC (&chunkStream, 0)) != PlankResult_OK) goto exit;
            }
        }
        
        regionArray = (PlankAudioFileRegion*)pl_DynamicArray_GetArray (regions);
        
        for (i = 0; i < numRegions; ++i)
        {
            string = pl_AudioFileRegion_GetLabel (&regionArray[i]);
            if (!string) continue;
            
            stringSize = strlen (string) + 1;
            if (!stringSize) continue;
            
            if ((result = pl_File_WriteFourCharCode (&chunkStream, pl_FourCharCode (ltxt))) != PlankResult_OK) goto exit;
            if ((result = pl_File_WriteUI (&chunkStream, 4 + 4 + 4 + 2 + 2 + 2 + 2 + stringSize + (stringSize & 1))) != PlankResult_OK) goto exit;
            if ((result = pl_File_WriteUI (&chunkStream, regionArray[i].start.cueID)) != PlankResult_OK) goto exit;
            if ((result = pl_File_WriteUI (&chunkStream, (PlankUI)(regionArray[i].end.position - regionArray[i].start.position))) != PlankResult_OK) goto exit;
            if ((result = pl_File_WriteUI (&chunkStream, pl_FourCharCode (labl))) != PlankResult_OK) goto exit; // purpose
            if ((result = pl_File_WriteUS (&chunkStream, 0)) != PlankResult_OK) goto exit; // country
            if ((result = pl_File_WriteUS (&chunkStream, 0)) != PlankResult_OK) goto exit; // language
            if ((result = pl_File_WriteUS (&chunkStream, 0)) != PlankResult_OK) goto exit; // dialect
            if ((result = pl_File_WriteUS (&chunkStream, 20127)) != PlankResult_OK) goto exit; // code page ASCII
            if ((result = pl_File_Write (&chunkStream, string, stringSize)) != PlankResult_OK) goto exit;
            
            if (stringSize & 1)
            {
                if ((result = pl_File_WriteC (&chunkStream, 0)) != PlankResult_OK) goto exit;
            }
        }
        
        listChunkSize = pl_DynamicArray_GetSize (&chunkData);
        
        if ((result = pl_IffFileWriter_PrepareChunk (iff, p->metaDataChunkPosition, list, pl_DynamicArray_GetArray (&chunkData), listChunkSize)) != PlankResult_OK) goto exit;
                
        p->dataPosition = pl_MaxLL (p->dataPosition, pl_AlignLL (pl_IffFileWriter_GetCurrentChunk (iff)->chunkPos + listChunkSize, iff->common.headerInfo.alignment) + 8);
        
        pl_File_DeInit (&chunkStream);
        pl_DynamicArray_DeInit (&chunkData);
    }
    
exit:
    return result;
}

PlankResult pl_AudioFileWriter_WAV_WriteMetaData (PlankAudioFileWriterRef p)
{
    PlankResult result = PlankResult_OK;
    PlankIffFileWriterRef iff;
    const char* JUNK;
    PlankIffFileWriterChunkInfoRef chunkInfo;
    PlankLL metaDataJunkSize;

    iff  = (PlankIffFileWriterRef)p->peer;
    JUNK = "JUNK";
    
    // junk the meta data - dont check errors, some may not exist
    pl_IffFileWriter_RenameChunk (iff, p->metaDataChunkPosition, "bext", JUNK);
    pl_IffFileWriter_RenameChunk (iff, p->metaDataChunkPosition, "smpl", JUNK);
    pl_IffFileWriter_RenameChunk (iff, p->metaDataChunkPosition, "inst", JUNK);
    pl_IffFileWriter_RenameChunk (iff, p->metaDataChunkPosition, "cue ", JUNK);
    pl_IffFileWriter_RenameChunk (iff, p->metaDataChunkPosition, "list", JUNK);
        
    if ((result = pl_IffFileWriter_SeekChunk (iff, p->metaDataChunkPosition, JUNK, &chunkInfo, 0)) != PlankResult_OK) goto exit;
    
    if (chunkInfo)
    {
        metaDataJunkSize = p->dataPosition - p->metaDataChunkPosition - 8 - 8;
        
        if ((result = pl_IffFileWriter_ResizeChunk (iff, p->metaDataChunkPosition, JUNK, metaDataJunkSize, PLANK_TRUE)) != PlankResult_OK) goto exit;
    }
    
    pl_IffFileWriter_PurgeChunkInfos (iff);
    
    if (p->metaData)
    {
        if ((result = pl_AudioFileWriter_WAV_WriteChunk_bext (p)) != PlankResult_OK) goto exit;
        if ((result = pl_AudioFileWriter_WAV_WriteChunk_smpl (p)) != PlankResult_OK) goto exit;
        if ((result = pl_AudioFileWriter_WAV_WriteChunk_inst (p)) != PlankResult_OK) goto exit;
        if ((result = pl_AudioFileWriter_WAV_WriteChunk_cue  (p)) != PlankResult_OK) goto exit;
        if ((result = pl_AudioFileWriter_WAV_WriteChunk_list (p)) != PlankResult_OK) goto exit;
    }
    
exit:
    return result;
}

PlankResult pl_AudioFileWriter_AIFF_WriteMetaData (PlankAudioFileWriterRef p)
{
    return PlankResult_UnknownError;
}

PlankResult pl_AudioFileWriter_AIFC_WriteMetaData (PlankAudioFileWriterRef p)
{
    return PlankResult_UnknownError;
}

PlankResult pl_AudioFileWriter_CAF_WriteMetaData (PlankAudioFileWriterRef p)
{
    return PlankResult_UnknownError;
}

PlankResult pl_AudioFileWriter_W64_WriteMetaData (PlankAudioFileWriterRef p)
{
    return PlankResult_UnknownError;
}

#if PLANK_OGGVORBIS
PlankResult pl_AudioFileWriter_OggVorbis_WriteMetaData (PlankAudioFileWriterRef p)
{
    return PlankResult_UnknownError;
}
#endif // PLANK_OGGVORBIS

#if PLANK_OPUS
PlankResult pl_AudioFileWriter_Opus_WriteMetaData (PlankAudioFileWriterRef p)
{
    return PlankResult_UnknownError;
}
#endif // PLANK_OPUS



