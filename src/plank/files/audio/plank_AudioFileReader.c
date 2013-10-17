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
#include "../../maths/plank_Maths.h"
#include "../plank_File.h"
#include "../plank_MultiFileReader.h"
#include "../plank_IffFileReader.h"
#include "plank_AudioFileReader.h"
#include "plank_AudioFileMetaData.h"
#include "plank_AudioFileCuePoint.h"
#include "plank_AudioFileRegion.h"


// private structures

// private functions and data
typedef PlankResult (*PlankAudioFileReaderReadFramesFunction)(PlankAudioFileReaderRef, const int, void*, int *);
typedef PlankResult (*PlankAudioFileReaderSetFramePositionFunction)(PlankAudioFileReaderRef, const PlankLL);
typedef PlankResult (*PlankAudioFileReaderGetFramePositionFunction)(PlankAudioFileReaderRef, PlankLL *);


#if PLANK_APPLE
#pragma mark Private Function Declarations
#endif

PlankResult pl_AudioFileReader_OpenInternalInternal (PlankAudioFileReaderRef p, const char* filepath, PlankFileRef file, const PlankB readMetaData);

PlankResult pl_AudioFileReader_WAV_ParseFormat (PlankAudioFileReaderRef p, const PlankLL chunkLength, const PlankLL chunkDataPos);
PlankResult pl_AudioFileReader_WAV_ParseMetaData (PlankAudioFileReaderRef p);
PlankResult pl_AudioFileReader_WAV_ParseData (PlankAudioFileReaderRef p, const PlankLL chunkLength, const PlankLL chunkDataPos);

PlankResult pl_AudioFileReader_AIFF_ParseFormat (PlankAudioFileReaderRef p, const PlankLL chunkLength, const PlankLL chunkDataPos);
PlankResult pl_AudioFileReader_AIFF_ParseMetaData (PlankAudioFileReaderRef p);
PlankResult pl_AudioFileReader_AIFF_ParseData(PlankAudioFileReaderRef p, const PlankLL chunkLength, const PlankLL chunkDataPos);

PlankResult pl_AudioFileReader_AIFC_ParseVersion (PlankAudioFileReaderRef p, const PlankLL chunkLength, const PlankLL chunkDataPos);
PlankResult pl_AudioFileReader_AIFC_ParseFormat (PlankAudioFileReaderRef p, const PlankLL chunkLength, const PlankLL chunkDataPos);
PlankResult pl_AudioFileReader_AIFF_ParseMetaData (PlankAudioFileReaderRef p);
PlankResult pl_AudioFileReader_AIFC_ParseData (PlankAudioFileReaderRef p, const PlankLL chunkLength, const PlankLL chunkDataPos);

PlankResult pl_AudioFileReader_CAF_ParseFormat (PlankAudioFileReaderRef p, const PlankLL chunkLength, const PlankLL chunkDataPos);
PlankResult pl_AudioFileReader_CAF_ParseMetaData (PlankAudioFileReaderRef p);
PlankResult pl_AudioFileReader_CAF_ParseData (PlankAudioFileReaderRef p, const PlankLL chunkLength, const PlankLL chunkDataPos);

PlankResult pl_AudioFileReader_W64_ParseFormat (PlankAudioFileReaderRef p, const PlankLL chunkLength, const PlankLL chunkDataPos);
PlankResult pl_AudioFileReader_W64_ParseMetaData (PlankAudioFileReaderRef p);
PlankResult pl_AudioFileReader_W64_ParseData (PlankAudioFileReaderRef p, const PlankLL chunkLength, const PlankLL chunkDataPos);

PlankResult pl_AudioFileReader_Iff_Open (PlankAudioFileReaderRef p);
PlankResult pl_AudioFileReader_Iff_ParseMain  (PlankAudioFileReaderRef p, const PlankIffID* mainID, const PlankIffID* formatID);
PlankResult pl_AudioFileReader_Iff_ReadFrames (PlankAudioFileReaderRef p, const int numFrames, void* data, int *framesRead);
PlankResult pl_AudioFileReader_Iff_SetFramePosition (PlankAudioFileReaderRef p, const PlankLL frameIndex);
PlankResult pl_AudioFileReader_Iff_GetFramePosition (PlankAudioFileReaderRef p, PlankLL *frameIndex);

PlankResult pl_AudioFileReader_OggVorbis_Open  (PlankAudioFileReaderRef p, const char* filepath);
PlankResult pl_AudioFileReader_OggVorbis_OpenWithFile  (PlankAudioFileReaderRef p, PlankFileRef file);
PlankResult pl_AudioFileReader_OggVorbis_Close (PlankAudioFileReaderRef p);
PlankResult pl_AudioFileReader_OggVorbis_ReadFrames (PlankAudioFileReaderRef p, const int numFrames, void* data, int *framesRead);
PlankResult pl_AudioFileReader_OggVorbis_SetFramePosition (PlankAudioFileReaderRef p, const PlankLL frameIndex);
PlankResult pl_AudioFileReader_OggVorbis_GetFramePosition (PlankAudioFileReaderRef p, PlankLL *frameIndex);

PlankResult pl_AudioFileReader_Opus_Open  (PlankAudioFileReaderRef p, const char* filepath);
PlankResult pl_AudioFileReader_Opus_OpenWithFile  (PlankAudioFileReaderRef p, PlankFileRef file);
PlankResult pl_AudioFileReader_Opus_Close (PlankAudioFileReaderRef p);
PlankResult pl_AudioFileReader_Opus_ReadFrames (PlankAudioFileReaderRef p, const int numFrames, void* data, int *framesRead);
PlankResult pl_AudioFileReader_Opus_SetFramePosition (PlankAudioFileReaderRef p, const PlankLL frameIndex);
PlankResult pl_AudioFileReader_Opus_GetFramePosition (PlankAudioFileReaderRef p, PlankLL *frameIndex);

PlankResult pl_AudioFileReader_Multi_OpenWithFile (PlankAudioFileReaderRef p, PlankFileRef file);
PlankResult pl_AudioFileReader_Multi_Close (PlankAudioFileReaderRef p);
PlankResult pl_AudioFileReader_Multi_ReadFrames (PlankAudioFileReaderRef p, const int numFrames, void* data, int *framesRead);
PlankResult pl_AudioFileReader_Multi_SetFramePosition (PlankAudioFileReaderRef p, const PlankLL frameIndex);
PlankResult pl_AudioFileReader_Multi_GetFramePosition (PlankAudioFileReaderRef p, PlankLL *frameIndex);
PlankResult pl_AudioFileReader_Multi_UpdateFormat (PlankAudioFileReaderRef p, const PlankAudioFileFormatInfo* newFormat, PlankB* frameFormatChanged);


#if PLANK_APPLE
#pragma mark Generic Functions
#endif

PlankAudioFileReaderRef pl_AudioFileReader_CreateAndInit()
{
    PlankAudioFileReaderRef p;
    p = pl_AudioFileReader_Create();
    
    if (p != PLANK_NULL)
    {
        if (pl_AudioFileReader_Init (p) != PlankResult_OK)
            pl_AudioFileReader_Destroy (p);
        else
            return p;
    }
    
    return PLANK_NULL;
}

PlankAudioFileReaderRef pl_AudioFileReader_Create()
{
    PlankMemoryRef m;
    PlankAudioFileReaderRef p;
    
    m = pl_MemoryGlobal();
    p = (PlankAudioFileReaderRef)pl_Memory_AllocateBytes (m, sizeof (PlankAudioFileReader));
    
    if (p != PLANK_NULL)
        pl_MemoryZero (p, sizeof (PlankAudioFileReader));        
        
    return p;
}

PlankResult pl_AudioFileReader_Init (PlankAudioFileReaderRef p)
{
    PlankResult result = PlankResult_OK;
                
    p->peer                        = PLANK_NULL;
    p->formatInfo.format           = PLANKAUDIOFILE_FORMAT_INVALID;
    p->formatInfo.encoding         = PLANKAUDIOFILE_ENCODING_INVALID;
    p->formatInfo.bitsPerSample    = 0;
    p->formatInfo.bytesPerFrame    = 0;
    p->formatInfo.minimumBitRate   = 0;
    p->formatInfo.maximumBitRate   = 0;
    p->formatInfo.nominalBitRate   = 0;
    p->formatInfo.numChannels      = 0;
    p->formatInfo.sampleRate       = 0.0;
    p->formatInfo.frameDuration    = 0.f;
    p->formatInfo.quality          = 0.f;
    
    pl_MemoryZero (p->formatInfo.channelMap, PLANKAUDIOFILE_CHANNELMAPLENGTH);
    
    p->numFrames                   = 0;
    p->dataPosition                = -1;
    
    p->metaData                    = PLANK_NULL;
    
    p->readFramesFunction          = PLANK_NULL;
    p->setFramePositionFunction    = PLANK_NULL;
    p->getFramePositionFunction    = PLANK_NULL;
    
    return result;
}

PlankResult pl_AudioFileReader_DeInit (PlankAudioFileReaderRef p)
{
    PlankResult result = PlankResult_OK;

    if (p == PLANK_NULL)
    {
        result = PlankResult_MemoryError;
        goto exit;
    }
    
    if ((result = pl_AudioFileReader_Close (p)) != PlankResult_OK) goto exit;
        
    pl_MemoryZero (p, sizeof (PlankAudioFileReader));

exit:
    return result;
}

PlankResult pl_AudioFileReader_Destroy (PlankAudioFileReaderRef p)
{
    PlankResult result = PlankResult_OK;
    PlankMemoryRef m = pl_MemoryGlobal();
    
    if (p == PLANK_NULL)
    {
        result = PlankResult_MemoryError;
        goto exit;
    }
    
    if ((result = pl_AudioFileReader_DeInit (p)) != PlankResult_OK)
        goto exit;
    
    result = pl_Memory_Free (m, p);    
    
exit:
    return result;
}

PlankFileRef pl_AudioFileReader_GetFile (PlankAudioFileReaderRef p)
{
    return p ? (PlankFileRef)p->peer : (PlankFileRef)PLANK_NULL;
}

PlankResult pl_AudioFileReader_Open (PlankAudioFileReaderRef p, const char* filepath)
{
    return pl_AudioFileReader_OpenInternal (p, filepath, PLANK_FALSE);
}

PlankResult pl_AudioFileReader_OpenWithMetaData (PlankAudioFileReaderRef p, const char* filepath)
{
    return pl_AudioFileReader_OpenInternal (p, filepath, PLANK_TRUE);
}

PlankResult pl_AudioFileReader_OpenInternalInternal (PlankAudioFileReaderRef p, const char* filepath, PlankFileRef file, const PlankB readMetaData)
{
    PlankResult result;
    PlankIffID mainID;
    PlankIffFileReaderRef iff;
    int fileStreamType;
        
    result = PlankResult_OK;
    iff = PLANK_NULL;
    
    if (((filepath) && (file)) || ((filepath == 0) && (file == 0)))
    {
        result = PlankResult_UnknownError;
        goto exit;
    }
    
    if ((result = pl_AudioFileReader_DeInit (p)) != PlankResult_OK) goto exit;
    if ((result = pl_AudioFileReader_Init (p)) != PlankResult_OK) goto exit;
        
    if (file)
    {
        if ((result = pl_File_GetStreamType (file, &fileStreamType)) != PlankResult_OK) goto exit;

        if (fileStreamType == PLANKFILE_STREAMTYPE_MULTI)
        {
            result = pl_AudioFileReader_Multi_OpenWithFile (p, file);
            goto exit; // we're a multi
        }
    }
    
    if (readMetaData)
        p->metaData = pl_AudioFileMetaData_CreateAndInit();
    
    if ((iff = pl_IffFileReader_CreateAndInit()) == PLANK_NULL)
    {
        result = PlankResult_MemoryError;
        goto exit;
    }
    
    // so the iff reader gets destroyed it we hit an error further down but before we're finished
    p->peer = iff;
    p->formatInfo.format = PLANKAUDIOFILE_FORMAT_UNKNOWNIFF;
    
    // open the file as an IFF
    if (filepath)
    {
        if ((result = pl_IffFileReader_Open (iff, filepath)) != PlankResult_OK) goto exit;
    }
    else if (file)
    {
        if ((result = pl_IffFileReader_OpenWithFile (iff, file)) != PlankResult_OK) goto exit;
    }
    
    // deterimine the file format, could be IFF or Ogg
    if ((result = pl_IffFileReader_GetMainID (iff, &mainID)) != PlankResult_OK) goto exit;

    if (iff->common.headerInfo.idType == PLANKIFFFILE_ID_FCC)
    {
        if ((mainID.fcc == pl_FourCharCode ("RIFF")) ||     // Riff
            (mainID.fcc == pl_FourCharCode ("FORM")) ||     // Iff
            (mainID.fcc == pl_FourCharCode ("caff")))       // CAF
        {
            if ((result = pl_AudioFileReader_Iff_Open (p)) != PlankResult_OK) goto exit;
        }
        else if (mainID.fcc == pl_FourCharCode ("OggS")) //Ogg this needs to handle any Ogg e.g., Vorbis or Opus
        {
            // close the Iff file and start again
            if ((result = pl_IffFileReader_Destroy (iff)) != PlankResult_OK) goto exit;
            
            p->peer = PLANK_NULL;
            p->formatInfo.format = PLANKAUDIOFILE_FORMAT_INVALID;
                    
#if PLANK_OGGVORBIS    
            if (p->peer == PLANK_NULL)
            {
                if (filepath)
                    result = pl_AudioFileReader_OggVorbis_Open (p, filepath);
                else if (file)
                    result = pl_AudioFileReader_OggVorbis_OpenWithFile (p, file);
                
                
                if (result != PlankResult_OK)
                {
                    pl_AudioFileReader_OggVorbis_Close (p);
                
                    p->peer = PLANK_NULL;
                    p->formatInfo.format = PLANKAUDIOFILE_FORMAT_INVALID;
                }
            }
#endif
#if PLANK_OPUS
            if (p->peer == PLANK_NULL)
            {
                if (filepath)
                    result = pl_AudioFileReader_Opus_Open (p, filepath);
                else if (file)
                    result = pl_AudioFileReader_Opus_OpenWithFile (p, file);
                
                if (result != PlankResult_OK)
                {
                    pl_AudioFileReader_Opus_Close (p);
                    
                    p->peer = PLANK_NULL;
                    p->formatInfo.format = PLANKAUDIOFILE_FORMAT_INVALID;
                }
            }
#endif
        }
    }
    else if (iff->common.headerInfo.idType == PLANKIFFFILE_ID_GUID)
    {
        if (pl_GUID_EqualWithString (&mainID.guid, PLANKAUDIOFILE_W64_RIFF_ID))
        {
            if ((result = pl_AudioFileReader_Iff_Open (p)) != PlankResult_OK) goto exit;
        }
    }
    else
    {
        if ((result = pl_IffFileReader_Destroy (iff)) != PlankResult_OK) goto exit;

        p->peer = PLANK_NULL;
        p->formatInfo.format = PLANKAUDIOFILE_FORMAT_INVALID;
        
        result = PlankResult_AudioFileInavlidType;
    }
    
exit:
    return result;
}

PlankResult pl_AudioFileReader_OpenInternal (PlankAudioFileReaderRef p, const char* filepath, const PlankB readMetaData)
{
    return pl_AudioFileReader_OpenInternalInternal (p, filepath, 0, readMetaData);
}

PlankResult pl_AudioFileReader_OpenWithFile (PlankAudioFileReaderRef p, PlankFileRef file, const PlankB readMetaData)
{
    return pl_AudioFileReader_OpenInternalInternal (p, 0, file, readMetaData);
}

PlankResult pl_AudioFileReader_Close (PlankAudioFileReaderRef p)
{
    PlankResult result = PlankResult_OK;

    if (p == PLANK_NULL)
        return PlankResult_FileCloseFailed;
    
    if (p->peer == PLANK_NULL)
        return PlankResult_OK;
    
    switch (p->formatInfo.format)
    {
        case PLANKAUDIOFILE_FORMAT_WAV:
        case PLANKAUDIOFILE_FORMAT_AIFF:
        case PLANKAUDIOFILE_FORMAT_AIFC:
        case PLANKAUDIOFILE_FORMAT_CAF:
        case PLANKAUDIOFILE_FORMAT_UNKNOWNIFF:
            result = pl_IffFileReader_Destroy ((PlankIffFileReader*)p->peer);
            break;
#if PLANK_OGGVORBIS
        case PLANKAUDIOFILE_FORMAT_OGGVORBIS:
            result = pl_AudioFileReader_OggVorbis_Close (p);
            break;
#endif
        default:
            if (p->peer != PLANK_NULL)
                return PlankResult_UnknownError;
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

const PlankAudioFileFormatInfo* pl_AudioFileReader_GetFormatInfoReadOnly (PlankAudioFileReaderRef p)
{
    return &p->formatInfo;
}

PlankResult pl_AudioFileReader_GetFormat (PlankAudioFileReaderRef p, int *format)
{
    *format = (int)p->formatInfo.format;
    return PlankResult_OK;
}

PlankResult pl_AudioFileReader_GetEncoding (PlankAudioFileReaderRef p, int *encoding)
{
    *encoding = (int)p->formatInfo.encoding;
    return PlankResult_OK;
}

PlankResult pl_AudioFileReader_GetBitsPerSample (PlankAudioFileReaderRef p, int *bitsPerSample)
{
    *bitsPerSample = p->formatInfo.bitsPerSample;
    return PlankResult_OK;
}

PlankResult pl_AudioFileReader_GetBytesPerFrame (PlankAudioFileReaderRef p, int *bytesPerFrame)
{
    *bytesPerFrame = p->formatInfo.bytesPerFrame;
    return PlankResult_OK;
}

PlankResult pl_AudioFileReader_GetNumChannels (PlankAudioFileReaderRef p, int *numChannels)
{
    *numChannels = p->formatInfo.numChannels;
    return PlankResult_OK;
}

PlankResult pl_AudioFileReader_GetSampleRate (PlankAudioFileReaderRef p, double *sampleRate)
{
    *sampleRate = p->formatInfo.sampleRate;
    return PlankResult_OK;
}

PlankResult pl_AudioFileReader_GetNumFrames (PlankAudioFileReaderRef p, PlankLL *numFrames)
{
    *numFrames = p->numFrames;
    return PlankResult_OK;
}

PlankResult pl_AudioFileReader_SetFramePosition (PlankAudioFileReaderRef p, const PlankLL frameIndex)
{
    PlankResult result = PlankResult_OK;
    
    if (p->peer == PLANK_NULL)
    {
        result = PlankResult_AudioFileNotReady;
        goto exit;
    }
        
    if (!p->setFramePositionFunction)
    {
        result = PlankResult_FunctionsInvalid;
        goto exit;
    }
    
    result = ((PlankAudioFileReaderSetFramePositionFunction)p->setFramePositionFunction)(p, frameIndex);
    
exit:
    return result;    
}

PlankResult pl_AudioFileReader_ResetFramePosition (PlankAudioFileReaderRef p)
{
    return pl_AudioFileReader_SetFramePosition (p, 0);
}

PlankResult pl_AudioFileReader_GetFramePosition (PlankAudioFileReaderRef p, PlankLL *frameIndex)
{
    PlankResult result = PlankResult_OK;
    
    if (p->peer == PLANK_NULL)
    {
        result = PlankResult_AudioFileNotReady;
        goto exit;
    }
    
    if (!p->getFramePositionFunction)
    {
        result = PlankResult_FunctionsInvalid;
        goto exit;
    }
    
    result = ((PlankAudioFileReaderGetFramePositionFunction)p->getFramePositionFunction)(p, frameIndex);
    
exit:
    return result;    
}

PlankResult pl_AudioFileReader_ReadFrames (PlankAudioFileReaderRef p, const int numFrames, void* data, int *framesRead)
{
    if (!p->readFramesFunction)
        return PlankResult_FunctionsInvalid;
    
    return ((PlankAudioFileReaderReadFramesFunction)p->readFramesFunction)(p, numFrames, data, framesRead);
}

// -- WAV Functions -- /////////////////////////////////////////////////////////
#if PLANK_APPLE
#pragma mark WAV Functions
#endif

static void pl_AudioFileWriter_WAVEXTChannelMaskToMap (PlankAudioFileFormatInfo* formatInfo, const PlankUI channelMask)
{
    PlankUI bit, mask;
    int channel;
    
    pl_MemoryZero (formatInfo->channelMap, PLANKAUDIOFILE_CHANNELMAPLENGTH);
    
    bit     = 0;
    channel = 0;
    
    while (bit < 32)
    {
        mask = ((PlankUI)1) << bit;
        
        if (mask & channelMask)
        {
            formatInfo->channelMap[channel] = bit;
            ++channel;
        }
        
        ++bit;
    }
}

PlankResult pl_AudioFileReader_WAV_ParseFormat (PlankAudioFileReaderRef p, const PlankLL chunkLength, const PlankLL chunkDataPos)
{
    PlankGUID ext;
    PlankResult result = PlankResult_OK;
    PlankUS compressionCode, numChannels;
    PlankUI sampleRate, byteRate, channelMask;
    PlankUS blockAlign, bitsPerSample;
    PlankGUID extpcm;
    PlankGUID extfloat;
    PlankGUID extambisonic;
    
    if ((result = pl_File_ReadUS ((PlankFileRef)p->peer, &compressionCode)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadUS ((PlankFileRef)p->peer, &numChannels)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadUI ((PlankFileRef)p->peer, &sampleRate)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadUI ((PlankFileRef)p->peer, &byteRate)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadUS ((PlankFileRef)p->peer, &blockAlign)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadUS ((PlankFileRef)p->peer, &bitsPerSample)) != PlankResult_OK) goto exit;
    
    channelMask = 0;
    p->numFrames = -1; // can't know yet
    
    if (compressionCode == PLANKAUDIOFILE_WAV_COMPRESSION_PCM)
    {
        p->formatInfo.encoding = PLANKAUDIOFILE_ENCODING_PCM_LITTLEENDIAN;
    }
    else if (compressionCode == PLANKAUDIOFILE_WAV_COMPRESSION_FLOAT)
    {
        p->formatInfo.encoding = PLANKAUDIOFILE_ENCODING_FLOAT_LITTLEENDIAN;
    }
    else if (compressionCode == PLANKAUDIOFILE_WAV_COMPRESSION_EXTENSIBLE)
    {
        if (chunkLength < PLANKAUDIOFILE_WAV_FMT_EXTENSIBLE_LENGTH) goto invalid;
        
        pl_MemoryZero (&ext, sizeof (PlankGUID));
        
        pl_GUID_InitHexString (&extpcm,        "00000001-0000-0010-8000-00aa00389b71");
        pl_GUID_InitHexString (&extfloat,      "00000003-0000-0010-8000-00aa00389b71");
        pl_GUID_InitHexString (&extambisonic,  "00000001-0721-11d3-8644-c8c1ca000000");

        if ((result = pl_File_SkipBytes ((PlankFileRef)p->peer, 4)) != PlankResult_OK) goto exit;
        if ((result = pl_File_ReadUI    ((PlankFileRef)p->peer, &channelMask)) != PlankResult_OK) goto exit;
        if ((result = pl_File_ReadGUID  ((PlankFileRef)p->peer, &ext)) != PlankResult_OK) goto exit;
        
        if (pl_GUID_Equal (&ext, &extpcm))
            p->formatInfo.encoding = PLANKAUDIOFILE_ENCODING_PCM_LITTLEENDIAN;
        else if (pl_GUID_Equal (&ext, &extfloat))
            p->formatInfo.encoding = PLANKAUDIOFILE_ENCODING_FLOAT_LITTLEENDIAN;
        else if (pl_GUID_Equal (&ext, &extambisonic))
            p->formatInfo.encoding = PLANKAUDIOFILE_ENCODING_PCM_LITTLEENDIAN;
        else
            goto invalid;        
    }
    else goto invalid;
    
    // set these last so that if the format is not recognised everything remains uninitialised
    
    if (bitsPerSample > 64)
    {
        p->formatInfo.bytesPerFrame = byteRate / (int)sampleRate;
        p->formatInfo.bitsPerSample = 8 * p->formatInfo.bytesPerFrame / numChannels;
    }
    else
    {
        // round up to whole bytes
        p->formatInfo.bytesPerFrame = (PlankI) (((bitsPerSample + (0x00000008 - 1)) & ~(0x00000008 - 1)) * numChannels / 8);
        p->formatInfo.bitsPerSample = (PlankI) bitsPerSample;
    }
    
    p->formatInfo.numChannels       = (PlankI) numChannels;
    p->formatInfo.sampleRate        = (PlankD) sampleRate;
    
    pl_AudioFileWriter_WAVEXTChannelMaskToMap (&p->formatInfo, channelMask);

    p->formatInfo.nominalBitRate    = (int)(p->formatInfo.bytesPerFrame * p->formatInfo.sampleRate * p->formatInfo.numChannels * PLANKAUDIOFILE_CHARBITS);
    p->formatInfo.minimumBitRate    = p->formatInfo.nominalBitRate;
    p->formatInfo.maximumBitRate    = p->formatInfo.nominalBitRate;
    p->formatInfo.frameDuration     = 1.0 / p->formatInfo.sampleRate;
    p->formatInfo.quality           = p->formatInfo.bitsPerSample / 64.f;
    
	(void)chunkDataPos;
	(void)chunkLength;

exit:
    return result;
    
invalid:
    result = PlankResult_AudioFileInavlidType;
    return result;
}

static PlankResult pl_AudioFileReader_WAV_ParseChunk_bext (PlankAudioFileReaderRef p, const PlankUI chunkLength, const PlankLL chunkEnd)
{
    PlankResult result = PlankResult_OK;
    char description [257];
    char originator [33];
    char originatorRef [33];
    char originationDate [11];
    char originationTime [9];
    PlankUI timeRefLow;
    PlankUI timeRefHigh;
    PlankUS version;
    PlankUC umid[64];
    PlankUC reserved[190];
    PlankUI fixedSize, lengthRemain;
    PlankDynamicArray tmp;
    PlankIffFileReaderRef iff;

    if ((result = pl_DynamicArray_Init (&tmp)) != PlankResult_OK) goto exit;
    
    pl_MemoryZero (description, 257);
    pl_MemoryZero (originator, 33);
    pl_MemoryZero (originatorRef, 33);
    pl_MemoryZero (originationDate, 11);
    pl_MemoryZero (originationTime, 9);

    iff = (PlankIffFileReaderRef)p->peer;

    fixedSize = 256 + 32 + 32 + 10 + 8 + 4 + 4 + 4 + 64 + 190;
    
    if ((result = pl_File_Read ((PlankFileRef)iff, description, 256, PLANK_NULL)) != PlankResult_OK) goto exit;
    if ((result = pl_File_Read ((PlankFileRef)iff, originator, 32, PLANK_NULL)) != PlankResult_OK) goto exit;
    if ((result = pl_File_Read ((PlankFileRef)iff, originatorRef, 32, PLANK_NULL)) != PlankResult_OK) goto exit;
    if ((result = pl_File_Read ((PlankFileRef)iff, originationDate, 10, PLANK_NULL)) != PlankResult_OK) goto exit;
    if ((result = pl_File_Read ((PlankFileRef)iff, originationTime, 8, PLANK_NULL)) != PlankResult_OK) goto exit;

    if ((result = pl_File_ReadUI ((PlankFileRef)iff, &timeRefLow)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadUI ((PlankFileRef)iff, &timeRefHigh)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadUS ((PlankFileRef)iff, &version)) != PlankResult_OK) goto exit;

    if ((result = pl_File_Read ((PlankFileRef)iff, umid, 64, PLANK_NULL)) != PlankResult_OK) goto exit;
    if ((result = pl_File_Read ((PlankFileRef)iff, reserved, 190, PLANK_NULL)) != PlankResult_OK) goto exit;
    
    if ((result = pl_AudioFileMetaData_ClearDescriptionComments (p->metaData)) != PlankResult_OK) goto exit;
    if ((result = pl_AudioFileMetaData_AddDescriptionComment (p->metaData, description)) != PlankResult_OK) goto exit;
    if ((result = pl_AudioFileMetaData_SetOriginatorArtist (p->metaData, originator)) != PlankResult_OK) goto exit;
    if ((result = pl_AudioFileMetaData_SetOriginatorRef (p->metaData, originatorRef)) != PlankResult_OK) goto exit;
    if ((result = pl_AudioFileMetaData_SetOriginationDate (p->metaData, originationDate)) != PlankResult_OK) goto exit;
    if ((result = pl_AudioFileMetaData_SetOriginationTime (p->metaData, originationTime)) != PlankResult_OK) goto exit;
    
    if ((result = pl_AudioFileMetaData_SetTimeRef (p->metaData, ((PlankLL)timeRefHigh << 32) | timeRefLow)) != PlankResult_OK) goto exit;
    if ((result = pl_AudioFileMetaData_SetUMID (p->metaData, umid)) != PlankResult_OK) goto exit;
    
    lengthRemain = chunkLength - fixedSize;
        
    if ((result = pl_DynamicArray_SetAsClearText (&tmp, lengthRemain)) != PlankResult_OK) goto exit;
    if ((result = pl_File_Read ((PlankFileRef)iff, pl_DynamicArray_GetArray (&tmp), lengthRemain, PLANK_NULL)) != PlankResult_OK) goto exit;
    if ((result = pl_AudioFileMetaData_AddCodingHistory (p->metaData, pl_DynamicArray_GetArray (&tmp))) != PlankResult_OK) goto exit;

exit:
    pl_DynamicArray_DeInit (&tmp);
    
    return result;
}

static PlankResult pl_AudioFileReader_WAV_ParseChunk_smpl_Loop (PlankAudioFileReaderRef p, const int index)
{
    PlankResult result = PlankResult_OK;
    PlankUI cueID, type, start, end, fraction, playCount;
    PlankAudioFileRegion loop;
    PlankAudioFileCuePointRef cuePointRef;
    PlankB success;
    PlankIffFileReaderRef iff;
    
    iff = (PlankIffFileReaderRef)p->peer;

    if ((result = pl_File_ReadUI ((PlankFileRef)iff, &cueID)) != PlankResult_OK) goto exit;
    cueID++; // offset WAV IDs by 1 using AIFF's standard that 0 is invalid

    if ((result = pl_File_ReadUI ((PlankFileRef)iff, &type)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadUI ((PlankFileRef)iff, &start)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadUI ((PlankFileRef)iff, &end)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadUI ((PlankFileRef)iff, &fraction)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadUI ((PlankFileRef)iff, &playCount)) != PlankResult_OK) goto exit;
    
    if ((result = pl_AudioFileRegion_Init (&loop)) != PlankResult_OK) goto exit;
    cuePointRef = pl_AudioFileRegion_GetAnchorCuePoint (&loop);
    
    if ((result = pl_AudioFileMetaData_RemoveCuePointWithID (p->metaData, cueID, cuePointRef, &success)) != PlankResult_OK) goto exit;
    
    if ((result = pl_AudioFileRegion_SetRegionWithAnchor (&loop, start, end, cuePointRef->position)) != PlankResult_OK) goto exit;
    if ((result = pl_AudioFileRegion_SetType (&loop, PLANKAUDIOFILE_REGIONTYPE_LOOP)) != PlankResult_OK) goto exit;
    if ((result = pl_AudioFileMetaData_AddLoopPoint (p->metaData, &loop)) != PlankResult_OK) goto exit;
    
exit:
    return result;
}

static PlankResult pl_AudioFileReader_WAV_ParseChunk_smpl (PlankAudioFileReaderRef p, const PlankUI chunkLength, const PlankLL chunkEnd)
{
    PlankResult result = PlankResult_OK;
    PlankUI manufacturer, product, samplePeriod, baseNote, detune, smpteFormat, smpteOffset, numSampleLoops, samplerData, i;
    PlankI gain, lowNote, highNote, lowVelocity, highVelocity;
    PlankIffFileReaderRef iff;
    
    iff = (PlankIffFileReaderRef)p->peer;

    if ((result = pl_File_ReadUI ((PlankFileRef)iff, &manufacturer)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadUI ((PlankFileRef)iff, &product)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadUI ((PlankFileRef)iff, &samplePeriod)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadUI ((PlankFileRef)iff, &baseNote)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadUI ((PlankFileRef)iff, &detune)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadUI ((PlankFileRef)iff, &smpteFormat)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadUI ((PlankFileRef)iff, &smpteOffset)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadUI ((PlankFileRef)iff, &numSampleLoops)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadUI ((PlankFileRef)iff, &samplerData)) != PlankResult_OK) goto exit;

    if ((result = pl_AudioFileMetaData_SetSamplerData (p->metaData, manufacturer, product, samplePeriod, smpteFormat, smpteOffset)) != PlankResult_OK) goto exit;
    if ((result = pl_AudioFileMetaData_GetInstrumentData (p->metaData, 0, 0, &gain, &lowNote, &highNote, &lowVelocity, &highVelocity)) != PlankResult_OK) goto exit;
    if ((result = pl_AudioFileMetaData_SetInstrumentData (p->metaData, baseNote, detune * 50 / 32768,
                                                          gain, lowNote, highNote, lowVelocity, highVelocity)) != PlankResult_OK) goto exit;

    for (i = 0; i < numSampleLoops; ++i)
    {
        if ((result = pl_AudioFileReader_WAV_ParseChunk_smpl_Loop (p, i)) != PlankResult_OK) goto exit;
    }
    
exit:
    return result;
}


static PlankResult pl_AudioFileReader_WAV_ParseChunk_inst (PlankAudioFileReaderRef p, const PlankUI chunkLength, const PlankLL chunkEnd)
{
    PlankResult result = PlankResult_OK;
    char baseNote, detune, gain, lowNote, highNote, lowVelocity, highVelocity;
    PlankIffFileReaderRef iff;
    
    iff = (PlankIffFileReaderRef)p->peer;

    if ((result = pl_File_ReadC ((PlankFileRef)iff, &baseNote)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadC ((PlankFileRef)iff, &detune)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadC ((PlankFileRef)iff, &gain)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadC ((PlankFileRef)iff, &lowNote)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadC ((PlankFileRef)iff, &highNote)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadC ((PlankFileRef)iff, &lowVelocity)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadC ((PlankFileRef)iff, &highVelocity)) != PlankResult_OK) goto exit;
        
    if ((result = pl_AudioFileMetaData_SetInstrumentData (p->metaData, baseNote, detune, gain,
                                                          lowNote, highNote, lowVelocity, highVelocity)) != PlankResult_OK) goto exit;

exit:
    return result;
}

static PlankResult pl_AudioFileReader_WAV_ParseChunk_cue (PlankAudioFileReaderRef p, const PlankUI chunkLength, const PlankLL chunkEnd)
{
    PlankResult result = PlankResult_OK;
    PlankUI numCuePoints, i;
    PlankIffFileReaderRef iff;
    PlankUI cueID, order, chunkID, chunkStart, blockStart, offset;
    PlankAudioFileCuePoint cuePoint;
    
    iff = (PlankIffFileReaderRef)p->peer;
    
    if ((result = pl_File_ReadUI ((PlankFileRef)iff, &numCuePoints)) != PlankResult_OK) goto exit;
    
    for (i = 0; i < numCuePoints; ++i)
    {
        if ((result = pl_File_ReadUI ((PlankFileRef)iff, &cueID)) != PlankResult_OK) goto exit;
        cueID++; // offset WAV IDs by 1 using AIFF's standard that 0 is invalid

        if ((result = pl_File_ReadUI ((PlankFileRef)iff, &order)) != PlankResult_OK) goto exit;
        if ((result = pl_File_ReadUI ((PlankFileRef)iff, &chunkID)) != PlankResult_OK) goto exit;
        if ((result = pl_File_ReadUI ((PlankFileRef)iff, &chunkStart)) != PlankResult_OK) goto exit;
        if ((result = pl_File_ReadUI ((PlankFileRef)iff, &blockStart)) != PlankResult_OK) goto exit;
        if ((result = pl_File_ReadUI ((PlankFileRef)iff, &offset)) != PlankResult_OK) goto exit;
                
        // assume there is no playlist, wavl or slnt chunks
        if ((chunkID == pl_FourCharCode ("data")) &&
            (chunkStart == 0) &&
            (blockStart == 0))
        {
            if ((result = pl_AudioFileCuePoint_Init (&cuePoint)) != PlankResult_OK) goto exit;
            if ((result = pl_AudioFileCuePoint_SetID (&cuePoint, cueID)) != PlankResult_OK) goto exit;
            if ((result = pl_AudioFileCuePoint_SetPosition (&cuePoint, offset)) != PlankResult_OK) goto exit;
            if ((result = pl_AudioFileCuePoint_SetType (&cuePoint, PLANKAUDIOFILE_CUEPOINTTYPE_CUEPOINT)) != PlankResult_OK) goto exit;

            if ((result = pl_AudioFileMetaData_AddCuePoint (p->metaData, &cuePoint)) != PlankResult_OK) goto exit;
        }
        else
        {
            result = PlankResult_AudioFileUnsupportedType;
            goto exit;
        }
    }
    
exit:
    return result;
}

static PlankResult pl_AudioFileReader_WAV_ParseChunk_LIST (PlankAudioFileReaderRef p, const PlankUI chunkLength, const PlankLL chunkEnd)
{
    PlankResult result = PlankResult_OK;
    PlankIffFileReaderRef iff;
    PlankFourCharCode typeID;
    PlankIffID adtlChunkID;
    PlankUI cueID, textLength, sampleLength, purpose;
    PlankLL pos, adtlChunkLength, adtlChunkEnd;
    PlankUS country, language, dialect, codePage;
    PlankAudioFileCuePointRef cuePointRef;
    PlankAudioFileRegion region;
    PlankB success;
    
    iff = (PlankIffFileReaderRef)p->peer;
    
    if ((result = pl_File_ReadFourCharCode ((PlankFileRef)iff, &typeID)) != PlankResult_OK) goto exit;

    if (typeID != pl_FourCharCode ("adtl"))
    {
        result = PlankResult_AudioFileInavlidType;
        goto exit;
    }
    
    if ((result = pl_File_GetPosition ((PlankFileRef)iff, &pos)) != PlankResult_OK) goto exit;
    
    while ((pos < chunkEnd) && (pl_File_IsEOF ((PlankFileRef)iff) == PLANK_FALSE))
    {
        if ((result = pl_IffFileReader_ParseChunkHeader (iff, 0, &adtlChunkID, &adtlChunkLength, &adtlChunkEnd, &pos)) != PlankResult_OK) goto exit;
        
        if (adtlChunkID.fcc == pl_FourCharCode ("labl"))
        {
            // label or title
            
            if ((result = pl_File_ReadUI ((PlankFileRef)iff, &cueID)) != PlankResult_OK) goto exit;            
            cueID++; // offset WAV IDs by 1 using AIFF's standard that 0 is invalid
            
            if ((result = pl_AudioFileMetaData_FindCuePointWithID (p->metaData, cueID, &cuePointRef, PLANK_NULL)) != PlankResult_OK) goto exit;

            if (cuePointRef != PLANK_NULL)
            {
                textLength = adtlChunkLength - 4;
                
                if ((result = pl_AudioFileCuePoint_SetLabelLengthClear (cuePointRef, textLength)) != PlankResult_OK) goto exit;
                if ((result = pl_File_Read ((PlankFileRef)iff, pl_AudioFileCuePoint_GetLabelWritable (cuePointRef), textLength, PLANK_NULL)) != PlankResult_OK) goto exit;
            }
        }
        if (adtlChunkID.fcc == pl_FourCharCode ("note"))
        {
            // comment
            
            if ((result = pl_File_ReadUI ((PlankFileRef)iff, &cueID)) != PlankResult_OK) goto exit;
            cueID++; // offset WAV IDs by 1 using AIFF's standard that 0 is invalid

            if ((result = pl_AudioFileMetaData_FindCuePointWithID (p->metaData, cueID, &cuePointRef, PLANK_NULL)) != PlankResult_OK) goto exit;
            
            if (cuePointRef != PLANK_NULL)
            {
                textLength = adtlChunkLength - 4;
                
                if ((result = pl_AudioFileCuePoint_SetCommentLengthClear (cuePointRef, textLength)) != PlankResult_OK) goto exit;
                if ((result = pl_File_Read ((PlankFileRef)iff, pl_AudioFileCuePoint_GetCommentWritable (cuePointRef), textLength, PLANK_NULL)) != PlankResult_OK) goto exit;
            }
        }
        else if (adtlChunkID.fcc == pl_FourCharCode ("ltxt"))
        {
            // labelled text chunk, add this as a region, removing the associated cue point from the cue array
            
            if ((result = pl_File_ReadUI ((PlankFileRef)iff, &cueID)) != PlankResult_OK) goto exit;
            cueID++; // offset WAV IDs by 1 using AIFF's standard that 0 is invalid
            
            if ((result = pl_AudioFileRegion_Init (&region)) != PlankResult_OK) goto exit;
            cuePointRef = pl_AudioFileRegion_GetAnchorCuePoint (&region);
            
            if ((result = pl_AudioFileMetaData_RemoveCuePointWithID (p->metaData, cueID, cuePointRef, &success)) != PlankResult_OK) goto exit;

            if (success)
            {
                if ((result = pl_File_ReadUI ((PlankFileRef)iff, &sampleLength)) != PlankResult_OK) goto exit;
                if ((result = pl_File_ReadUI ((PlankFileRef)iff, &purpose)) != PlankResult_OK) goto exit;
                if ((result = pl_File_ReadUS ((PlankFileRef)iff, &country)) != PlankResult_OK) goto exit;
                if ((result = pl_File_ReadUS ((PlankFileRef)iff, &language)) != PlankResult_OK) goto exit;
                if ((result = pl_File_ReadUS ((PlankFileRef)iff, &dialect)) != PlankResult_OK) goto exit;
                if ((result = pl_File_ReadUS ((PlankFileRef)iff, &codePage)) != PlankResult_OK) goto exit;
                
                textLength = adtlChunkLength - 20;

                if ((result = pl_AudioFileCuePoint_SetExtra (cuePointRef, purpose, country, language, dialect, codePage)) != PlankResult_OK) goto exit;
                if ((result = pl_AudioFileCuePoint_SetLabelLengthClear (cuePointRef, textLength)) != PlankResult_OK) goto exit;
                if ((result = pl_File_Read ((PlankFileRef)iff, pl_AudioFileCuePoint_GetLabelWritable (cuePointRef), textLength, PLANK_NULL)) != PlankResult_OK) goto exit;

                if ((result = pl_AudioFileRegion_SetRegion (&region, cuePointRef->position, cuePointRef->position + sampleLength)) != PlankResult_OK) goto exit;                
                if ((result = pl_AudioFileRegion_SetType (&region, PLANKAUDIOFILE_REGIONTYPE_REGION)) != PlankResult_OK) goto exit;

                if ((result = pl_AudioFileMetaData_AddRegion (p->metaData, &region)) != PlankResult_OK) goto exit;
            }
         }
        else
        {
            // notes:
            // - amadeus uses 'mcol' for marker colours
            // - 'file' chunk? for media file e.g. album art?
//            printf("%s - chunk in LIST\n", (pl_FourCharCode2String (adtlChunkID).string));
        }
        
    next:
        if ((result = pl_File_SetPosition ((PlankFileRef)iff, adtlChunkEnd)) != PlankResult_OK) goto exit;
        pos = adtlChunkEnd;
    }

    
//    printf("LIST - %d bytes\n", chunkLength);
    
exit:
    return result;
}

PlankResult pl_AudioFileReader_WAV_ParseMetaData (PlankAudioFileReaderRef p)
{
    PlankResult result = PlankResult_OK;
    PlankLL readChunkLength, readChunkEnd, mainEnd, pos;
    PlankIffID readChunkID;
    PlankIffFileReaderRef iff;
    PlankDynamicArrayRef block;
    PlankC* data;
    int bytesRead;

    iff = (PlankIffFileReaderRef)p->peer;
    pos = iff->common.headerInfo.mainHeaderEnd;
    
    if ((result = pl_AudioFileMetaData_SetSource (p->metaData, PLANKAUDIOFILE_FORMAT_WAV)) != PlankResult_OK) goto exit;
    
    if ((result = pl_IffFileReader_GetMainEnd (iff, &mainEnd)) != PlankResult_OK) goto exit;
    if ((result = pl_File_SetPosition ((PlankFileRef)iff, pos)) != PlankResult_OK) goto exit;
    
    while ((pos < mainEnd) && (pl_File_IsEOF ((PlankFileRef)iff) == PLANK_FALSE))
    {
        if ((result = pl_IffFileReader_ParseChunkHeader (iff, 0, &readChunkID, &readChunkLength, &readChunkEnd, &pos)) != PlankResult_OK) goto exit;
        
        if ((readChunkID.fcc == pl_FourCharCode ("fmt ")) ||
            (readChunkID.fcc == pl_FourCharCode ("data")))
        {
            // already done...
            goto next;
        }
        else if (readChunkID.fcc == pl_FourCharCode ("bext"))
        {
            if ((result = pl_AudioFileReader_WAV_ParseChunk_bext (p, readChunkLength, readChunkEnd)) != PlankResult_OK) goto exit;
        }
        else if (readChunkID.fcc == pl_FourCharCode ("levl"))
        {
            // might be large...
            goto next;
        }
        else if (readChunkID.fcc == pl_FourCharCode ("smpl"))
        {
            if ((result = pl_AudioFileReader_WAV_ParseChunk_smpl (p, readChunkLength, readChunkEnd)) != PlankResult_OK) goto exit;
        }
        else if ((readChunkID.fcc == pl_FourCharCode ("inst")) ||
                 (readChunkID.fcc == pl_FourCharCode ("INST")))
        {
            if ((result = pl_AudioFileReader_WAV_ParseChunk_inst (p, readChunkLength, readChunkEnd)) != PlankResult_OK) goto exit;
        }
        else if (readChunkID.fcc == pl_FourCharCode ("cue "))
        {
            if ((result = pl_AudioFileReader_WAV_ParseChunk_cue (p, readChunkLength, readChunkEnd)) != PlankResult_OK) goto exit;
        }
        else if ((readChunkID.fcc == pl_FourCharCode ("list")) ||
                 (readChunkID.fcc == pl_FourCharCode ("LIST")) )
        {
            if ((result = pl_AudioFileReader_WAV_ParseChunk_LIST (p, readChunkLength, readChunkEnd)) != PlankResult_OK) goto exit;
        }
        
        else if (readChunkID.fcc != iff->common.headerInfo.junkID.fcc)
        {
            // cache others that aren't junk
            
            block = pl_DynamicArray_Create();
            
            if (block != PLANK_NULL)
            {
                if ((result = pl_DynamicArray_InitWithItemSizeAndSize (block, 1, readChunkLength + 8, PLANK_FALSE)) != PlankResult_OK) goto exit;
                
                data = (PlankC*)pl_DynamicArray_GetArray (block);
                
                *(PlankFourCharCode*)data = readChunkID.fcc;
                data += 4;
                *(PlankUI*)data = readChunkLength;
                data += 4;
                
                if ((result = pl_File_Read ((PlankFileRef)iff, data, readChunkLength, &bytesRead)) != PlankResult_OK) goto exit;
                if ((result = pl_AudioFileMetaData_AddFormatSpecificBlock (p->metaData, block)) != PlankResult_OK) goto exit;
            }
        }
        
    next:
        if ((result = pl_File_SetPosition ((PlankFileRef)iff, readChunkEnd)) != PlankResult_OK) goto exit;
        pos = readChunkEnd;
    }

exit:
    return result;
}

PlankResult pl_AudioFileReader_WAV_ParseData (PlankAudioFileReaderRef p, const PlankLL chunkLength, const PlankLL chunkDataPos)
{
    PlankResult result = PlankResult_OK;
        
    p->dataPosition = chunkDataPos;
            
    if (p->formatInfo.bytesPerFrame > 0)
    {
        p->numFrames = chunkLength / p->formatInfo.bytesPerFrame;

        if ((chunkLength % p->formatInfo.bytesPerFrame) != 0)
            result = PlankResult_AudioFileDataChunkInvalid;
    }
    
    return result;
}

// -- AIFF Functions -- ////////////////////////////////////////////////////////
#if PLANK_APPLE
#pragma mark AIFF Functions
#endif

PlankResult pl_AudioFileReader_AIFF_ParseFormat (PlankAudioFileReaderRef p, const PlankLL chunkLength, const PlankLL chunkDataPos)
{
    PlankResult result = PlankResult_OK;
    PlankS numChannels;
    PlankUI numFrames;
    PlankS bitsPerSample;
    PlankF80 sampleRate;

    if ((result = pl_File_ReadS ((PlankFileRef)p->peer, &numChannels)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadUI ((PlankFileRef)p->peer, &numFrames)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadS ((PlankFileRef)p->peer, &bitsPerSample)) != PlankResult_OK) goto exit;
    if ((result = pl_File_Read ((PlankFileRef)p->peer, sampleRate.data, sizeof (sampleRate), PLANK_NULL)) != PlankResult_OK) goto exit;    
    
    p->formatInfo.bitsPerSample     = (PlankI) bitsPerSample;
    p->formatInfo.bytesPerFrame     = (PlankI) (((bitsPerSample + (0x00000008 - 1)) & ~(0x00000008 - 1)) * numChannels / 8); // round up to whole bytes
    p->formatInfo.numChannels       = (PlankI) numChannels;
    p->formatInfo.sampleRate        = (PlankD) pl_F802I (sampleRate);
    p->numFrames                    = (PlankLL) numFrames;
    p->formatInfo.nominalBitRate    = (int)(p->formatInfo.bytesPerFrame * p->formatInfo.sampleRate * p->formatInfo.numChannels * PLANKAUDIOFILE_CHARBITS);
    p->formatInfo.minimumBitRate    = p->formatInfo.nominalBitRate;
    p->formatInfo.maximumBitRate    = p->formatInfo.nominalBitRate;
    p->formatInfo.frameDuration     = 1.0 / p->formatInfo.sampleRate;
    p->formatInfo.quality           = p->formatInfo.bitsPerSample / 64.f;

	(void)chunkDataPos;
	(void)chunkLength;

exit:
    return PlankResult_OK;
}

PlankResult pl_AudioFileReader_AIFF_ParseMetaData (PlankAudioFileReaderRef p)
{
    (void)p;
    return PlankResult_OK;
}

PlankResult pl_AudioFileReader_AIFF_ParseData (PlankAudioFileReaderRef p, const PlankLL chunkLength, const PlankLL chunkDataPos)
{
    PlankResult result = PlankResult_OK;
    PlankUI offset, blockSize;
    PlankLL pos;
    
    if ((result = pl_File_ReadUI ((PlankFileRef)p->peer, &offset)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadUI ((PlankFileRef)p->peer, &blockSize)) != PlankResult_OK) goto exit;
    if ((result = pl_File_GetPosition ((PlankFileRef)p->peer, &pos)) != PlankResult_OK) goto exit;    
    
    p->dataPosition = pos;
        
	(void)chunkDataPos;
	(void)chunkLength;

exit:
    return result;
}

// -- AIFC Functions -- ////////////////////////////////////////////////////////
#if PLANK_APPLE
#pragma mark AIFC Functions
#endif

PlankResult pl_AudioFileReader_AIFC_ParseVersion (PlankAudioFileReaderRef p, const PlankLL chunkLength, const PlankLL chunkDataPos)
{
    PlankResult result = PlankResult_OK;
    PlankUI version;
    
    if ((result = pl_File_ReadUI ((PlankFileRef)p->peer, &version)) != PlankResult_OK) goto exit;
    if (version == 0) goto exit;    
    if (version == PLANKAUDIOFILE_AIFC_VERSION) goto exit;
    
    result = PlankResult_AudioFileInavlidType;
    
	(void)chunkDataPos;
	(void)chunkLength;

exit:
    return result;
}

PlankResult pl_AudioFileReader_AIFC_ParseMetaData (PlankAudioFileReaderRef p)
{
    (void)p;
    return PlankResult_OK;
}

PlankResult pl_AudioFileReader_AIFC_ParseFormat (PlankAudioFileReaderRef p, const PlankLL chunkLength, const PlankLL chunkDataPos)
{
    PlankResult result = PlankResult_OK;
    PlankFourCharCode compressionID;
    PlankPascalString255 compressionName;
    
    if ((result = pl_AudioFileReader_AIFF_ParseFormat (p, chunkLength, chunkDataPos)) != PlankResult_OK) goto exit;

    if ((result = pl_File_ReadFourCharCode ((PlankFileRef)p->peer, &compressionID)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadPascalString255 ((PlankFileRef)p->peer, &compressionName)) != PlankResult_OK) goto exit;
    
    if (compressionID == pl_FourCharCode ("NONE"))
    {
        p->formatInfo.encoding = PLANKAUDIOFILE_ENCODING_PCM_BIGENDIAN;
    }
    else if (compressionID == pl_FourCharCode ("twos"))
    {
        p->formatInfo.encoding = PLANKAUDIOFILE_ENCODING_PCM_BIGENDIAN;
    }
    else if (compressionID == pl_FourCharCode ("sowt"))
    {
        p->formatInfo.encoding = PLANKAUDIOFILE_ENCODING_PCM_LITTLEENDIAN;
    }
    else if (compressionID == pl_FourCharCode ("fl32"))
    {
        if (p->formatInfo.bitsPerSample != 32)
        {
            result = PlankResult_AudioFileInavlidType;
            goto exit;
        }
        
        p->formatInfo.encoding = PLANKAUDIOFILE_ENCODING_FLOAT_BIGENDIAN;
    }
    else if (compressionID == pl_FourCharCode ("fl64"))
    {
        if (p->formatInfo.bitsPerSample != 64)
        {
            result = PlankResult_AudioFileInavlidType;
            goto exit;
        }
        
        p->formatInfo.encoding = PLANKAUDIOFILE_ENCODING_FLOAT_BIGENDIAN;
    }
    
exit:
    return result;
}

PlankResult pl_AudioFileReader_AIFC_ParseData (PlankAudioFileReaderRef p, const PlankLL chunkLength, const PlankLL chunkDataPos)
{
    return pl_AudioFileReader_AIFF_ParseData (p, chunkLength, chunkDataPos);
}

// -- CAF Functions -- ////////////////////////////////////////////////////////
#if PLANK_APPLE
#pragma mark CAF Functions
#endif

PlankResult pl_AudioFileReader_CAF_ParseFormat (PlankAudioFileReaderRef p, const PlankLL chunkLength, const PlankLL chunkDataPos)
{
    PlankResult result = PlankResult_OK;
    PlankFourCharCode formatID;
    PlankUI formatFlags, bytesPerPacket, framesPerPacket, numChannels, bitsPerChannel;
    PlankD sampleRate;
    
    if ((result = pl_File_ReadD ((PlankFileRef)p->peer, &sampleRate)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadFourCharCode ((PlankFileRef)p->peer, &formatID)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadUI ((PlankFileRef)p->peer, &formatFlags)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadUI ((PlankFileRef)p->peer, &bytesPerPacket)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadUI ((PlankFileRef)p->peer, &framesPerPacket)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadUI ((PlankFileRef)p->peer, &numChannels)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadUI ((PlankFileRef)p->peer, &bitsPerChannel)) != PlankResult_OK) goto exit;

    if (formatID != pl_FourCharCode ("lpcm"))
    {
        result = PlankResult_AudioFileUnsupportedType;
        goto exit;
    }
    
    if (framesPerPacket != 1)
    {
        // should be 1 for PCM
        result = PlankResult_AudioFileUnsupportedType;
        goto exit;
    }
    
    p->formatInfo.encoding          = 0;
    
    if (!(formatFlags & PLANKAUDIOFILE_CAF_LITTLEENDIAN_FLAG))
        p->formatInfo.encoding     |= PLANKAUDIOFILE_ENCODING_BIGENDIAN_FLAG;
    
    if (formatFlags & PLANKAUDIOFILE_CAF_FLOAT_FLAG)
    {
        if ((bitsPerChannel != 32) && (bitsPerChannel != 64))
        {
            result = PlankResult_AudioFileInavlidType;
            goto exit;
        }
        
        p->formatInfo.encoding     |= PLANKAUDIOFILE_ENCODING_FLOAT_FLAG;
    }
    else
    {
        p->formatInfo.encoding     |= PLANKAUDIOFILE_ENCODING_PCM_FLAG;
    }
    
    p->numFrames                    = -1;
    p->formatInfo.sampleRate        = sampleRate;
    p->formatInfo.numChannels       = (PlankI)numChannels;
    p->formatInfo.bytesPerFrame     = (PlankI)bytesPerPacket;
    p->formatInfo.bitsPerSample     = (PlankI)bitsPerChannel;
    p->formatInfo.nominalBitRate    = (int)(p->formatInfo.bytesPerFrame * p->formatInfo.sampleRate * p->formatInfo.numChannels * PLANKAUDIOFILE_CHARBITS);
    p->formatInfo.minimumBitRate    = p->formatInfo.nominalBitRate;
    p->formatInfo.maximumBitRate    = p->formatInfo.nominalBitRate;
    p->formatInfo.frameDuration     = 1.0 / p->formatInfo.sampleRate;
    p->formatInfo.quality           = p->formatInfo.bitsPerSample / 64.f;
    
	(void)chunkDataPos;
	(void)chunkLength;
    
exit:
    return PlankResult_OK;
}

PlankResult pl_AudioFileReader_CAF_ParseMetaData (PlankAudioFileReaderRef p)
{
    (void)p;
    return PlankResult_OK;
}

PlankResult pl_AudioFileReader_CAF_ParseData (PlankAudioFileReaderRef p, const PlankLL chunkLength, const PlankLL chunkDataPos)
{
    PlankResult result;
    PlankUI numEdits;
    PlankLL audioDataLength;
    
    result = PlankResult_OK;
    
    if ((result = pl_File_ReadUI ((PlankFileRef)p->peer, &numEdits)) != PlankResult_OK) goto exit;
    
    p->dataPosition = chunkDataPos + 4;                             // plus the numEdits field
    audioDataLength = (chunkLength == -1) ? -1 : chunkLength  - 4;  // less the numEdits field
    
    if (p->formatInfo.bytesPerFrame > 0)
    {
        if (audioDataLength == -1)
        {
            p->numFrames = -1;
        }
        else
        {
            p->numFrames = audioDataLength / p->formatInfo.bytesPerFrame;
        
            if ((chunkLength % p->formatInfo.bytesPerFrame) != 0)
                result = PlankResult_AudioFileDataChunkInvalid;
        }
    }
                        
exit:
    return result;
}

// -- W64 Functions -- ////////////////////////////////////////////////////////
#if PLANK_APPLE
#pragma mark W64 Functions
#endif

PlankResult pl_AudioFileReader_W64_ParseFormat (PlankAudioFileReaderRef p, const PlankLL chunkLength, const PlankLL chunkDataPos)
{
    return pl_AudioFileReader_WAV_ParseFormat (p, chunkLength, chunkDataPos);
}

PlankResult pl_AudioFileReader_W64_ParseMetaData (PlankAudioFileReaderRef p)
{
    (void)p;
    return PlankResult_OK;
}

PlankResult pl_AudioFileReader_W64_ParseData (PlankAudioFileReaderRef p, const PlankLL chunkLength, const PlankLL chunkDataPos)
{
    PlankResult result = PlankResult_OK;
    
    p->dataPosition = chunkDataPos;
    
    if (p->formatInfo.bytesPerFrame > 0)
    {
        p->numFrames = chunkLength / p->formatInfo.bytesPerFrame;
        
        if ((chunkLength % p->formatInfo.bytesPerFrame) != 0)
            result = PlankResult_AudioFileDataChunkInvalid;
    }
    
    return result;
}

// -- Generic Iff Functions -- /////////////////////////////////////////////////
#if PLANK_APPLE
#pragma mark Generic Iff Functions
#endif

PlankResult pl_AudioFileReader_Iff_Open (PlankAudioFileReaderRef p)
{
    PlankResult result;
    PlankLL chunkLength, chunkDataPos;
    PlankIffFileReaderRef iff;
    PlankIffID mainID, formatID;
    
    iff = (PlankIffFileReaderRef)p->peer;
    
    // deterimine the IFF file format, could be IFF or RIFF
    if ((result = pl_IffFileReader_GetMainID (iff, &mainID)) != PlankResult_OK) goto exit;
    if ((result = pl_IffFileReader_GetFormatID (iff, &formatID)) != PlankResult_OK) goto exit;
    if ((result = pl_AudioFileReader_Iff_ParseMain (p, &mainID, &formatID)) != PlankResult_OK) goto exit;
    
    // parse based on the format
    if (p->formatInfo.format == PLANKAUDIOFILE_FORMAT_WAV)
    {
        if ((result = pl_IffFileReader_SeekChunk (iff, 0, "fmt ", &chunkLength, &chunkDataPos)) != PlankResult_OK) goto exit;
        if ((result = pl_AudioFileReader_WAV_ParseFormat (p, chunkLength, chunkDataPos)) != PlankResult_OK) goto exit;
        
        if (p->metaData)
        {
            result = pl_AudioFileReader_WAV_ParseMetaData (p);
            
            if ((result != PlankResult_OK) && result != PlankResult_FileEOF)
                goto exit;
        }
        
        if ((result = pl_IffFileReader_SeekChunk (iff, 0, "data", &chunkLength, &chunkDataPos)) != PlankResult_OK) goto exit;
        if ((result = pl_AudioFileReader_WAV_ParseData (p, chunkLength, chunkDataPos)) != PlankResult_OK) goto exit;
    }
    else if (p->formatInfo.format == PLANKAUDIOFILE_FORMAT_AIFF)
    {
        if ((result = pl_IffFileReader_SeekChunk (iff, 0, "COMM", &chunkLength, &chunkDataPos)) != PlankResult_OK) goto exit;
        if ((result = pl_AudioFileReader_AIFF_ParseFormat (p, chunkLength, chunkDataPos)) != PlankResult_OK) goto exit;
        
        p->formatInfo.encoding = PLANKAUDIOFILE_ENCODING_PCM_BIGENDIAN;
        
        if (p->metaData)
        {
            if ((result = pl_AudioFileReader_AIFF_ParseMetaData (p)) != PlankResult_OK) goto exit;
        }
        
        if ((result = pl_IffFileReader_SeekChunk (iff, 0, "SSND", &chunkLength, &chunkDataPos)) != PlankResult_OK) goto exit;
        if ((result = pl_AudioFileReader_AIFF_ParseData (p, chunkLength, chunkDataPos)) != PlankResult_OK) goto exit;
    }
    else if (p->formatInfo.format == PLANKAUDIOFILE_FORMAT_AIFC)
    {
        if ((result = pl_IffFileReader_SeekChunk (iff, 0, "FVER", &chunkLength, &chunkDataPos)) != PlankResult_OK) goto exit;
        if ((result = pl_AudioFileReader_AIFC_ParseVersion (p, chunkLength, chunkDataPos)) != PlankResult_OK) goto exit;
        
        if ((result = pl_IffFileReader_SeekChunk (iff, 0, "COMM", &chunkLength, &chunkDataPos)) != PlankResult_OK) goto exit;
        if ((result = pl_AudioFileReader_AIFC_ParseFormat (p, chunkLength, chunkDataPos)) != PlankResult_OK) goto exit;
        
        if (p->metaData)
        {
            if ((result = pl_AudioFileReader_AIFC_ParseMetaData (p)) != PlankResult_OK) goto exit;
        }
        
        if ((result = pl_IffFileReader_SeekChunk (iff, 0, "SSND", &chunkLength, &chunkDataPos)) != PlankResult_OK) goto exit;
        if ((result = pl_AudioFileReader_AIFC_ParseData (p, chunkLength, chunkDataPos)) != PlankResult_OK) goto exit;        
    }
    else if (p->formatInfo.format == PLANKAUDIOFILE_FORMAT_CAF)
    {
        if ((result = pl_IffFileReader_SeekChunk (iff, 0, "desc", &chunkLength, &chunkDataPos)) != PlankResult_OK) goto exit;
        if ((result = pl_AudioFileReader_CAF_ParseFormat (p, chunkLength, chunkDataPos)) != PlankResult_OK) goto exit;
        
        if (p->metaData)
        {
            result = pl_AudioFileReader_CAF_ParseMetaData (p);
            
            if ((result != PlankResult_OK) && result != PlankResult_FileEOF)
                goto exit;
        }
        
        if ((result = pl_IffFileReader_SeekChunk (iff, 0, "data", &chunkLength, &chunkDataPos)) != PlankResult_OK) goto exit;
        if ((result = pl_AudioFileReader_CAF_ParseData (p, chunkLength, chunkDataPos)) != PlankResult_OK) goto exit;
    }
    else if (p->formatInfo.format == PLANKAUDIOFILE_FORMAT_W64)
    {
        if ((result = pl_IffFileReader_SeekChunk (iff, 0, PLANKAUDIOFILE_W64_FMT_ID, &chunkLength, &chunkDataPos)) != PlankResult_OK) goto exit;
        if ((result = pl_AudioFileReader_W64_ParseFormat (p, chunkLength, chunkDataPos)) != PlankResult_OK) goto exit;
        
        if (p->metaData)
        {
            result = pl_AudioFileReader_W64_ParseMetaData (p);
            
            if ((result != PlankResult_OK) && result != PlankResult_FileEOF)
                goto exit;
        }
        
        if ((result = pl_IffFileReader_SeekChunk (iff, 0, PLANKAUDIOFILE_W64_DATA_ID, &chunkLength, &chunkDataPos)) != PlankResult_OK) goto exit;
        if ((result = pl_AudioFileReader_W64_ParseData (p, chunkLength, chunkDataPos)) != PlankResult_OK) goto exit;
    }
    else
    {
        result = PlankResult_AudioFileInavlidType;
        goto exit;
    }    
    
    p->readFramesFunction       = pl_AudioFileReader_Iff_ReadFrames;
    p->setFramePositionFunction = pl_AudioFileReader_Iff_SetFramePosition;
    p->getFramePositionFunction = pl_AudioFileReader_Iff_GetFramePosition;

    if ((result = pl_AudioFileReader_ResetFramePosition (p)) != PlankResult_OK) goto exit;     
    
exit:
    return result;
}

PlankResult pl_AudioFileReader_Iff_ParseMain  (PlankAudioFileReaderRef p, 
                                               const PlankIffID* mainID,
                                               const PlankIffID* formatID)
{        
    PlankIffFileReader* iff = PLANK_NULL;
    PlankB isBigEndian = PLANK_FALSE;
    
    iff = (PlankIffFileReader*)p->peer;
    
    if (iff->common.headerInfo.idType == PLANKIFFFILE_ID_FCC)
    {
        if (mainID->fcc == pl_FourCharCode ("RIFF"))
        {
            isBigEndian = PLANK_FALSE;
            
            if (formatID->fcc == pl_FourCharCode ("WAVE"))
            {
                p->formatInfo.format = PLANKAUDIOFILE_FORMAT_WAV;
            }
            else goto exit;
        }
        else if (mainID->fcc == pl_FourCharCode ("FORM"))
        {
            isBigEndian = PLANK_TRUE;
            
            if (formatID->fcc == pl_FourCharCode ("AIFF"))
            {
                p->formatInfo.format = PLANKAUDIOFILE_FORMAT_AIFF;
            }
            else if (formatID->fcc == pl_FourCharCode ("AIFC"))
            {
                p->formatInfo.format = PLANKAUDIOFILE_FORMAT_AIFC;
            }
            else goto exit;
        }
        else if (mainID->fcc == pl_FourCharCode ("caff"))
        {
            isBigEndian = PLANK_TRUE;
            p->formatInfo.format = PLANKAUDIOFILE_FORMAT_CAF;
        }
    }
    else if (iff->common.headerInfo.idType == PLANKIFFFILE_ID_GUID)
    {
        if (pl_GUID_EqualWithString (&mainID->guid, PLANKAUDIOFILE_W64_RIFF_ID))
        {
            isBigEndian = PLANK_FALSE;
            
            if (pl_GUID_EqualWithString (&formatID->guid, PLANKAUDIOFILE_W64_WAVE_ID))
            {
                p->formatInfo.format = PLANKAUDIOFILE_FORMAT_W64;
            }
            else goto exit;
        }
        
    }
    else goto exit;
    
    pl_IffFileReader_SetEndian (iff, isBigEndian);
    
    return PlankResult_OK;
    
exit:
    return PlankResult_AudioFileInavlidType;
}

PlankResult pl_AudioFileReader_Iff_ReadFrames (PlankAudioFileReaderRef p, const int numFrames, void* data, int *framesRead)
{
    PlankResult result = PlankResult_OK;
    PlankLL startFrame, endFrame;
    int framesToRead, bytesToRead, bytesRead;
    
    if (p->peer == PLANK_NULL)
    {
        result = PlankResult_AudioFileNotReady;
        goto exit;
    }
    
    if ((p->dataPosition < 0) || (p->formatInfo.bytesPerFrame <= 0))
    {
        result = PlankResult_AudioFileNotReady;
        goto exit;
    }
    
    if ((result = pl_AudioFileReader_GetFramePosition (p, &startFrame)) != PlankResult_OK) goto exit;
    
    if (startFrame < 0)
    {
        result = PlankResult_AudioFileInvalidFilePosition;
        goto exit;
    }
    
    endFrame = startFrame + numFrames;
    
    framesToRead = ((p->numFrames == -1) || (endFrame <= p->numFrames)) ? (numFrames) : (int)(p->numFrames - startFrame);
    bytesToRead = framesToRead * p->formatInfo.bytesPerFrame;
    
    if (bytesToRead > 0)
    {
        result = pl_File_Read ((PlankFileRef)p->peer, data, bytesToRead, &bytesRead);
    }
    else
    {
        bytesRead = 0;
        result = PlankResult_FileEOF;
    }
    
    if (framesRead != PLANK_NULL)
        *framesRead = bytesRead / p->formatInfo.bytesPerFrame;
    
    // should zero if framesToRead < numFrames
    
exit:
    return result;
}

PlankResult pl_AudioFileReader_Iff_SetFramePosition (PlankAudioFileReaderRef p, const PlankLL frameIndex)
{
    PlankResult result;
    PlankLL pos;
    
    if ((p->dataPosition < 0) || (p->formatInfo.bytesPerFrame <= 0))
    {
        result = PlankResult_AudioFileNotReady;
        goto exit;
    }
    
    pos = p->dataPosition + frameIndex * p->formatInfo.bytesPerFrame;
    result = pl_File_SetPosition ((PlankFileRef)p->peer, pos);
    
exit:
    return result;
}

PlankResult pl_AudioFileReader_Iff_GetFramePosition (PlankAudioFileReaderRef p, PlankLL *frameIndex)
{
    PlankResult result = PlankResult_OK;
    PlankLL pos;
    
    if ((p->dataPosition < 0) || (p->formatInfo.bytesPerFrame <= 0))
    {
        result = PlankResult_AudioFileNotReady;
        goto exit;
    }
    
    if ((result = pl_File_GetPosition ((PlankFileRef)p->peer, &pos)) != PlankResult_OK) goto exit;
    
    *frameIndex = (pos - p->dataPosition) / p->formatInfo.bytesPerFrame;
    
exit:
    return result;
}

// -- Useful Ogg Functions -- //////////////////////////////////////////////////

#if PLANK_OGGVORBIS || PLANK_OPUS
#if PLANK_APPLE
#pragma mark Useful Ogg Functions
#endif

static PlankResult pl_OggFile_FindNextPageOffset (PlankFileRef p, const PlankLL total, PlankLL left, PlankLL right, PlankLL* offset)
{
    PlankResult result;
    PlankLL original;
    char sync[4]; // could initialise here if not for C89!
    int syncIndex;
    char byte;
        
    sync[0] = 'O';
    sync[1] = 'g';
    sync[2] = 'g';
    sync[3] = 'S';
    syncIndex = 0;
    
    result = PlankResult_OK;
    
    if ((result = pl_File_GetPosition (p, &original)) != PlankResult_OK) goto exit;
    
    left = pl_ClipLL (left, 0, total);
    right = pl_ClipLL (right, 0, total);
    
    if ((result = pl_File_SetPosition (p, left)) != PlankResult_OK) goto exit;
    
    // now find 'OggS'
    while ((syncIndex < 4) && (left < right))
    {
        if ((result = pl_File_ReadC (p, &byte)) != PlankResult_OK)
            goto exit; // will hit EOF
        
        syncIndex = (byte == sync[syncIndex]) ? syncIndex + 1 : 0;
        left++;
    }
    
    *offset = (syncIndex == 4) ? left - 4 : -1; 

    if ((result = pl_File_SetPosition (p, original)) != PlankResult_OK) goto exit;

exit:
    return result;
}

static PlankResult pl_OggFile_FindPrevPageOffset (PlankFileRef p, const PlankLL total, PlankLL left, PlankLL right, PlankLL* offset)
{
    PlankResult result;
    PlankLL original;
    char sync[4]; // could initialise here if not for C89!
    int syncIndex;
    char byte;
    
    sync[0] = 'S';
    sync[1] = 'g';
    sync[2] = 'g';
    sync[3] = 'O';
    syncIndex = 0;
    
    result = PlankResult_OK;
    
    if ((result = pl_File_GetPosition (p, &original)) != PlankResult_OK) goto exit;
    
    left = pl_ClipLL (left, 0, total);
    right = pl_ClipLL (right, 0, total);
    
    
    // now find 'OggS'
    while ((syncIndex < 4) && (left < right))
    {
        if ((result = pl_File_SetPosition (p, right)) != PlankResult_OK) goto exit;

        result = pl_File_ReadC (p, &byte);
        
        if ((result != PlankResult_OK) && (result != PlankResult_FileEOF))
            goto exit;
        
        syncIndex = (byte == sync[syncIndex]) ? syncIndex + 1 : 0;
        right--;
    }
    
    *offset = (syncIndex == 4) ? right + 1 : -1; 
    
    if ((result = pl_File_SetPosition (p, original)) != PlankResult_OK) goto exit;
    
exit:
    return result;
}


#endif

// -- Ogg Vorbis Functions -- //////////////////////////////////////////////////

#if PLANK_OGGVORBIS
#if PLANK_APPLE
#pragma mark Ogg Vorbis Functions
#endif

#include "../../containers/plank_DynamicArray.h"
typedef struct PlankOggVorbisFileReader
{
    PlankFile file;
    OggVorbis_File oggVorbisFile;
    ov_callbacks callbacks;
    PlankDynamicArray buffer;
    int bufferPosition;
    int bufferFrames;
    PlankLL totalFramesRead;
    int bitStream;
} PlankOggVorbisFileReader;

typedef PlankOggVorbisFileReader* PlankOggVorbisFileReaderRef;

size_t pl_OggVorbisFileReader_ReadCallback (PlankP ptr, size_t size, size_t size2, PlankP ref);
int pl_OggVorbisFileReader_SeekCallback (PlankP ref, PlankLL offset, int code);
int pl_OggVorbisFileReader_CloseCallback (PlankP ref);
long pl_OggVorbisFileReader_TellCallback (PlankP ref);


PlankResult pl_AudioFileReader_OggVorbis_Open  (PlankAudioFileReaderRef p, const char* filepath)
{
    PlankResult result;
    PlankOggVorbisFileReaderRef ogg;
    PlankMemoryRef m;
    PlankLL numFrames;
    PlankL bufferSize;
    PlankI bytesPerSample;
    
    int err;
    vorbis_info* info;
    vorbis_comment* comment;
    
    m = pl_MemoryGlobal();
    
    // open as ogg
    ogg = (PlankOggVorbisFileReaderRef)pl_Memory_AllocateBytes (m, sizeof (PlankOggVorbisFileReader));
    
    if (ogg == PLANK_NULL)
    {
        result = PlankResult_MemoryError;
        goto exit;
    }
    
    pl_MemoryZero (ogg, sizeof (PlankOggVorbisFileReader));
    
    p->peer = ogg;
    bytesPerSample = sizeof (float);
    p->formatInfo.format = PLANKAUDIOFILE_FORMAT_OGGVORBIS;
    p->formatInfo.encoding = PLANKAUDIOFILE_ENCODING_FLOAT_LITTLEENDIAN;
    p->formatInfo.bitsPerSample = PLANKAUDIOFILE_CHARBITS * bytesPerSample;
        
    ogg->bufferPosition  = 0;
    ogg->bufferFrames    = 0;
    ogg->totalFramesRead = 0;
    ogg->bitStream       = -1;

    if ((result = pl_File_Init (&ogg->file)) != PlankResult_OK) goto exit;
    
    // open as binary, not writable, litte endian
    if ((result = pl_File_OpenBinaryRead (&ogg->file, filepath, PLANK_FALSE, PLANK_FALSE)) != PlankResult_OK) goto exit;
    
    ogg->callbacks.read_func  = &pl_OggVorbisFileReader_ReadCallback;
    ogg->callbacks.seek_func  = &pl_OggVorbisFileReader_SeekCallback;
    ogg->callbacks.close_func = &pl_OggVorbisFileReader_CloseCallback;
    ogg->callbacks.tell_func  = &pl_OggVorbisFileReader_TellCallback;
    
    err = ov_open_callbacks (p, &ogg->oggVorbisFile, 0, 0, ogg->callbacks); // docs suggest this should be on the other thread if threaded...
    
    if (err != 0)
    {
        result = PlankResult_AudioFileInavlidType;
        goto exit;
    }
    
    info = ov_info (&ogg->oggVorbisFile, -1);
    comment = ov_comment (&ogg->oggVorbisFile, -1);
    
    p->formatInfo.numChannels       = info->channels;
    p->formatInfo.sampleRate        = info->rate;
    p->formatInfo.bytesPerFrame     = info->channels * bytesPerSample;
    p->formatInfo.nominalBitRate    = info->bitrate_nominal;
    p->formatInfo.minimumBitRate    = info->bitrate_lower;
    p->formatInfo.maximumBitRate    = info->bitrate_upper;
    p->formatInfo.frameDuration     = 0.0;
    p->formatInfo.quality           = 0.f;
    
    numFrames = ov_pcm_total (&ogg->oggVorbisFile, -1);
    
    bufferSize = numFrames > 0 ? pl_MinL (numFrames * p->formatInfo.bytesPerFrame, (PlankL)4096) : (PlankL)4096;
    
    if ((result = pl_DynamicArray_InitWithItemSizeAndSize (&ogg->buffer, 1, bufferSize, PLANK_FALSE)) != PlankResult_OK) goto exit;
    
    if (numFrames < 0) // could allow this for continuous streams?
    {
        result = PlankResult_UnknownError;
        goto exit;
    }
        
    p->numFrames = numFrames;
    p->readFramesFunction       = pl_AudioFileReader_OggVorbis_ReadFrames;
    p->setFramePositionFunction = pl_AudioFileReader_OggVorbis_SetFramePosition;
    p->getFramePositionFunction = pl_AudioFileReader_OggVorbis_GetFramePosition;
    
exit:
    return result;
}

PlankResult pl_AudioFileReader_OggVorbis_OpenWithFile  (PlankAudioFileReaderRef p, PlankFileRef file)
{
    PlankResult result;
    PlankOggVorbisFileReaderRef ogg;
    PlankMemoryRef m;
    PlankLL numFrames;
    PlankL bufferSize;
    PlankI bytesPerSample;
    
    int err, mode;
    vorbis_info* info;
    vorbis_comment* comment;
    
    m = pl_MemoryGlobal();
    
    // open as ogg
    ogg = (PlankOggVorbisFileReaderRef)pl_Memory_AllocateBytes (m, sizeof (PlankOggVorbisFileReader));
    
    if (ogg == PLANK_NULL)
    {
        result = PlankResult_MemoryError;
        goto exit;
    }
    
    pl_MemoryZero (ogg, sizeof (PlankOggVorbisFileReader));

    p->peer = ogg;
    bytesPerSample = sizeof (float);
    p->formatInfo.format = PLANKAUDIOFILE_FORMAT_OGGVORBIS;
    p->formatInfo.encoding = PLANKAUDIOFILE_ENCODING_FLOAT_LITTLEENDIAN;
    p->formatInfo.bitsPerSample = PLANKAUDIOFILE_CHARBITS * bytesPerSample;
        
    ogg->bufferPosition  = 0;
    ogg->bufferFrames    = 0;
    ogg->totalFramesRead = 0;
    ogg->bitStream       = -1;
    
    if ((result = pl_File_GetMode (file, &mode)) != PlankResult_OK) goto exit;
    
    if (!(mode & PLANKFILE_BINARY))
    {
        result = PlankResult_AudioFileInavlidType;
        goto exit;
    }

    if (!(mode & PLANKFILE_READ))
    {
        result = PlankResult_AudioFileInavlidType;
        goto exit;
    }
    
    if (mode & PLANKFILE_BIGENDIAN)
    {
        result = PlankResult_AudioFileInavlidType;
        goto exit;
    }
    
    pl_MemoryCopy (&ogg->file, file, sizeof (PlankFile));
    pl_MemoryZero (file, sizeof (PlankFile));
    
    ogg->callbacks.read_func  = &pl_OggVorbisFileReader_ReadCallback;
    ogg->callbacks.seek_func  = &pl_OggVorbisFileReader_SeekCallback;
    ogg->callbacks.close_func = &pl_OggVorbisFileReader_CloseCallback;
    ogg->callbacks.tell_func  = &pl_OggVorbisFileReader_TellCallback;
    
    err = ov_open_callbacks (p, &ogg->oggVorbisFile, 0, 0, ogg->callbacks); // docs suggest this should be on the other thread if threaded...
    
    if (err != 0)
    {
        result = PlankResult_AudioFileInavlidType;
        goto exit;
    }
    
    info = ov_info (&ogg->oggVorbisFile, -1);
    comment = ov_comment (&ogg->oggVorbisFile, -1);
    
    p->formatInfo.numChannels       = info->channels;
    p->formatInfo.sampleRate        = info->rate;
    p->formatInfo.bytesPerFrame     = info->channels * bytesPerSample;
    p->formatInfo.nominalBitRate    = info->bitrate_nominal;
    p->formatInfo.minimumBitRate    = info->bitrate_lower;
    p->formatInfo.maximumBitRate    = info->bitrate_upper;
    p->formatInfo.frameDuration     = 0.0;
    p->formatInfo.quality           = 0.f;

    numFrames = ov_pcm_total (&ogg->oggVorbisFile, -1);
    
    bufferSize = numFrames > 0 ? pl_MinL (numFrames * p->formatInfo.bytesPerFrame, (PlankL)4096) : (PlankL)4096;
    
    if ((result = pl_DynamicArray_InitWithItemSizeAndSize (&ogg->buffer, 1, bufferSize, PLANK_FALSE)) != PlankResult_OK) goto exit;
    
    if (numFrames < 0) // could allow this for continuous streams?
    {
        result = PlankResult_UnknownError;
        goto exit;
    }
    
    p->numFrames = numFrames;
    p->readFramesFunction       = pl_AudioFileReader_OggVorbis_ReadFrames;
    p->setFramePositionFunction = pl_AudioFileReader_OggVorbis_SetFramePosition;
    p->getFramePositionFunction = pl_AudioFileReader_OggVorbis_GetFramePosition;
    
exit:
    return result;
}

PlankResult pl_AudioFileReader_OggVorbis_Close (PlankAudioFileReaderRef p)
{
    PlankOggVorbisFileReaderRef ogg;
    PlankResult result = PlankResult_OK;
    int err;
    PlankMemoryRef m = pl_MemoryGlobal();
    
    if (p == PLANK_NULL)
    {
        result = PlankResult_MemoryError;
        goto exit;
    }
    
    ogg = (PlankOggVorbisFileReaderRef)p->peer;
    
    err = ov_clear (&ogg->oggVorbisFile); // closes our PlankFile in the close callback
    
    if (err != 0)
    {
        result = PlankResult_FileCloseFailed;
        goto exit;
    }
    
    if ((result = pl_DynamicArray_DeInit (&ogg->buffer)) != PlankResult_OK) goto exit;

    pl_Memory_Free (m, ogg);

exit:
    return result;
}

PlankResult pl_AudioFileReader_OggVorbis_ReadFrames (PlankAudioFileReaderRef p, const int numFrames, void* data, int *framesReadOut)
{    
    PlankResult result;
    PlankOggVorbisFileReaderRef ogg;
    int numFramesRemaining, bufferFramesRemaining, bufferFramePosition;
    int bufferSizeInBytes, bytesPerFrame, bufferFrameEnd, bitStream;
    int framesThisTime, numChannels, framesRead, streamFrameEnd, i, j;
    float* buffer;
    float* dst;
    float** pcm;
    float* bufferTemp;
    float* pcmTemp;
    OggVorbis_File* file;
    vorbis_info* info;
    
    result = PlankResult_OK;
    ogg = (PlankOggVorbisFileReaderRef)p->peer;
    file = &ogg->oggVorbisFile;

    numFramesRemaining      = numFrames;
    bufferFramesRemaining   = ogg->bufferFrames;         // starts at 0
    bufferFramePosition     = ogg->bufferPosition;       // starts at 0
    bufferSizeInBytes       = pl_DynamicArray_GetSize (&ogg->buffer);
    bytesPerFrame           = p->formatInfo.bytesPerFrame;
    numChannels             = p->formatInfo.numChannels;
    bufferFrameEnd          = bufferSizeInBytes / bytesPerFrame;
    buffer                  = (float*)pl_DynamicArray_GetArray (&ogg->buffer);
    dst                     = (float*)data;
    streamFrameEnd          = p->numFrames;
    bitStream               = ogg->bitStream;
    
    framesRead = 0;
    
    while (numFramesRemaining > 0)
    {
        if (bufferFramesRemaining > 0)
        {
            framesThisTime = pl_MinI (bufferFramesRemaining, numFramesRemaining);
                        
            pl_MemoryCopy (dst, buffer + bufferFramePosition * numChannels, framesThisTime * bytesPerFrame);
            
            bufferFramePosition += framesThisTime;
            bufferFramesRemaining -= framesThisTime;
            numFramesRemaining -= framesThisTime;
            framesRead += framesThisTime;
            
            dst += framesThisTime * numChannels;
        }
        
        if (bufferFramesRemaining == 0)
        {
            bufferTemp = 0;
            pcmTemp = 0;
            pcm = 0;
            
            framesThisTime = (int)ov_read_float (file, &pcm, bufferFrameEnd, &bitStream);
            
            if (bitStream != ogg->bitStream)
            {
                ogg->bitStream = bitStream;
                info = ov_info (file, -1);
                
                if (info->channels != numChannels)
                {
                    result = PlankResult_UnknownError;
                    goto exit;
                }
                
                if (info->rate != (int)p->formatInfo.sampleRate)
                {
                    result = PlankResult_UnknownError;
                    goto exit;
                }
            }
            
            if (framesThisTime == 0)
            {
                result = PlankResult_FileEOF;                
                goto exit;
            }
            else if (framesThisTime < 0)
            {
                // OV_HOLE or OV_EINVAL
                result = PlankResult_FileReadError;
                goto exit;
            }
                        
            bufferFramePosition = 0;
            bufferFramesRemaining = framesThisTime;
            ogg->totalFramesRead += framesThisTime;
                        
            // interleave to buffer...
            
            for (i = 0; i < numChannels; ++i)
            {
                bufferTemp = buffer + i;
                pcmTemp = pcm[i];
                
                for (j = 0; j < framesThisTime; ++j, bufferTemp += numChannels)
                    *bufferTemp = pl_ClipF (pcmTemp[j], -1.f, 1.f);
            }
        }
    }
    
exit:
    if (numFramesRemaining > 0)
        pl_MemoryZero (dst, numFramesRemaining * bytesPerFrame);
    
    ogg->bufferFrames   = bufferFramesRemaining;
    ogg->bufferPosition = bufferFramePosition;

    *framesReadOut = framesRead;
    
    return result;
}


PlankResult pl_AudioFileReader_OggVorbis_SetFramePosition (PlankAudioFileReaderRef p, const PlankLL frameIndex)
{
    PlankOggVorbisFileReaderRef ogg;
    int err;
    
    ogg = (PlankOggVorbisFileReaderRef)p->peer;
    err = ov_pcm_seek_lap (&ogg->oggVorbisFile, frameIndex); // should probably eventually do my own lapping in readframes?
    
    if (err != 0)
        return PlankResult_FileSeekFailed;
    
    ogg->bufferFrames   = 0;
    ogg->bufferPosition = 0;
    
    return PlankResult_OK;
}

PlankResult pl_AudioFileReader_OggVorbis_GetFramePosition (PlankAudioFileReaderRef p, PlankLL *frameIndex)
{
    PlankOggVorbisFileReaderRef ogg;
    PlankLL pos;
    
    ogg = (PlankOggVorbisFileReaderRef)p->peer;
    pos = ov_pcm_tell (&ogg->oggVorbisFile);
    
    if (pos < 0)
        return PlankResult_FileSeekFailed;
    
    *frameIndex = pos;
    
    return PlankResult_OK;
}

#if PLANK_APPLE
#pragma mark Ogg Vorbis Callbacks
#endif

size_t pl_OggVorbisFileReader_ReadCallback (PlankP ptr, size_t size, size_t nmemb, PlankP datasource)
{
    size_t ret;
    PlankResult result;
    PlankAudioFileReaderRef p;
    PlankOggVorbisFileReaderRef ogg;
    int bytesRead;
    
    p = (PlankAudioFileReaderRef)datasource;
    ogg = (PlankOggVorbisFileReaderRef)p->peer;
    
    result = pl_File_Read ((PlankFileRef)ogg, ptr, (int)(size * nmemb) / size, &bytesRead);
    ret = bytesRead > 0 ? bytesRead : 0;
    
    if ((result != PlankResult_OK) && (result != PlankResult_FileEOF))
        errno = -1;
    
    return ret;
}

int pl_OggVorbisFileReader_SeekCallback (PlankP datasource, PlankLL offset, int code)
{    
    PlankResult result;
    PlankAudioFileReaderRef p;
    PlankOggVorbisFileReaderRef ogg;
    PlankFileRef file;
    
    p = (PlankAudioFileReaderRef)datasource;
    ogg = (PlankOggVorbisFileReaderRef)p->peer;
    file = (PlankFileRef)ogg;
    
    // API says return -1 (OV_FALSE) if the file is not seekable    
    // call inner callback directly
    result = (file->setPositionFunction) (file, offset, code);
        
    return result == PlankResult_OK ? 0 : OV_FALSE;
}

int pl_OggVorbisFileReader_CloseCallback (PlankP datasource)
{
    PlankResult result;
    PlankAudioFileReaderRef p;
    PlankOggVorbisFileReaderRef ogg;
    PlankFileRef file;
    
    p = (PlankAudioFileReaderRef)datasource;
    ogg = (PlankOggVorbisFileReaderRef)p->peer;
    file = (PlankFileRef)ogg;
    
    result = pl_File_DeInit (file);
    
    return result == PlankResult_OK ? 0 : OV_FALSE;
}

// check this should be long or guarantee it's 64 bits?
long pl_OggVorbisFileReader_TellCallback (PlankP datasource)
{
    PlankResult result;
    PlankAudioFileReaderRef p;
    PlankOggVorbisFileReaderRef ogg;
    PlankFileRef file;
    PlankLL position;
    
    p = (PlankAudioFileReaderRef)datasource;
    ogg = (PlankOggVorbisFileReaderRef)p->peer;
    file = (PlankFileRef)ogg;
    
    result = pl_File_GetPosition (file, &position);

    return result == PlankResult_OK ? (long)position : (long)OV_FALSE;
}
#endif // PLANK_OGGVORBIS

// -- Opus Functions -- ////////////////////////////////////////////////////////



#if PLANK_OPUS
#if PLANK_APPLE
#pragma mark Opus Functions
#endif

#include "../../containers/plank_DynamicArray.h"
typedef struct PlankOpusFileReader
{
    PlankFile file;
    OggOpusFile* oggOpusFile;
    OpusFileCallbacks callbacks;
    PlankDynamicArray buffer;
    int bufferPosition;
    int bufferFrames;    
    PlankLL totalFramesRead;
    int link;
} PlankOpusFileReader;

typedef PlankOpusFileReader* PlankOpusFileReaderRef;

size_t pl_OpusFileReader_ReadCallback (PlankP ptr, size_t size, size_t size2, PlankP ref);
int pl_OpusFileReader_SeekCallback (PlankP ref, PlankLL offset, int code);
int pl_OpusFileReader_CloseCallback (PlankP ref);
PlankLL pl_OpusFileReader_TellCallback (PlankP ref);

PlankResult pl_AudioFileReader_Opus_Open  (PlankAudioFileReaderRef p, const char* filepath)
{
    PlankResult result;
    PlankOpusFileReaderRef opus;
    PlankMemoryRef m;
    PlankLL numFrames;
    PlankL bufferSize;
    PlankI bytesPerSample;
    const OpusTags* tags;
    
    int err;
    
    m = pl_MemoryGlobal();
    
    // open as ogg
    opus = (PlankOpusFileReaderRef)pl_Memory_AllocateBytes (m, sizeof (PlankOpusFileReader));
    
    if (opus == PLANK_NULL)
    {
        result = PlankResult_MemoryError;
        goto exit;
    }
    
    pl_MemoryZero (opus, sizeof (PlankOpusFileReader));
    
    p->peer = opus;
    bytesPerSample = sizeof (float);
    p->formatInfo.format = PLANKAUDIOFILE_FORMAT_OPUS;
    p->formatInfo.encoding = PLANKAUDIOFILE_ENCODING_FLOAT_LITTLEENDIAN;
    p->formatInfo.bitsPerSample = PLANKAUDIOFILE_CHARBITS * bytesPerSample;
        
    opus->bufferPosition  = 0;
    opus->bufferFrames    = 0;
    opus->totalFramesRead = 0;
    opus->link            = -1;
    
    if ((result = pl_File_Init (&opus->file)) != PlankResult_OK) goto exit;
    
    // open as binary, not writable, litte endian
    if ((result = pl_File_OpenBinaryRead (&opus->file, filepath, PLANK_FALSE, PLANK_FALSE)) != PlankResult_OK) goto exit;
    
    opus->callbacks.read  = &pl_OpusFileReader_ReadCallback;
    opus->callbacks.seek  = &pl_OpusFileReader_SeekCallback;
    opus->callbacks.close = &pl_OpusFileReader_CloseCallback;
    opus->callbacks.tell  = &pl_OpusFileReader_TellCallback;
        
    opus->oggOpusFile = op_open_callbacks (p, &opus->callbacks, NULL, 0, &err);
    
    if (err != 0)
    {
        result = PlankResult_AudioFileInavlidType;
        goto exit;
    }
    
    tags = op_tags (opus->oggOpusFile, -1);
    
    p->formatInfo.numChannels       = op_channel_count (opus->oggOpusFile, -1);
    p->formatInfo.sampleRate        = PLANKAUDIOFILE_OPUS_DEFAULTSAMPLERATE;
    p->formatInfo.bytesPerFrame     = p->formatInfo.numChannels * bytesPerSample;
    p->formatInfo.nominalBitRate    = op_bitrate (opus->oggOpusFile, -1);
    p->formatInfo.minimumBitRate    = p->formatInfo.nominalBitRate;
    p->formatInfo.maximumBitRate    = p->formatInfo.nominalBitRate;
    p->formatInfo.frameDuration     = 0.0;
    p->formatInfo.quality           = 0.f;

    numFrames = op_pcm_total (opus->oggOpusFile, -1);
    bufferSize = PLANKAUDIOFILE_OPUS_MAXFRAMESIZE * p->formatInfo.bytesPerFrame;
    
    if ((result = pl_DynamicArray_InitWithItemSizeAndSize (&opus->buffer, 1, bufferSize, PLANK_FALSE)) != PlankResult_OK) goto exit;
    
    if (numFrames < 0) // should really allow this for continuous streams or nonseekable..
    {
        result = PlankResult_UnknownError;
        goto exit;
    }
    
    p->numFrames = numFrames;
    p->readFramesFunction       = pl_AudioFileReader_Opus_ReadFrames;
    p->setFramePositionFunction = pl_AudioFileReader_Opus_SetFramePosition;
    p->getFramePositionFunction = pl_AudioFileReader_Opus_GetFramePosition;
    
exit:
    return result;
}


PlankResult pl_AudioFileReader_Opus_OpenWithFile  (PlankAudioFileReaderRef p, PlankFileRef file)
{
    PlankResult result;
    PlankOpusFileReaderRef opus;
    PlankMemoryRef m;
    PlankLL numFrames;
    PlankL bufferSize;
    PlankI bytesPerSample;
    const OpusTags* tags;
    
    int err, mode;
    
    m = pl_MemoryGlobal();
    
    // open as ogg
    opus = (PlankOpusFileReaderRef)pl_Memory_AllocateBytes (m, sizeof (PlankOpusFileReader));
    
    if (opus == PLANK_NULL)
    {
        result = PlankResult_MemoryError;
        goto exit;
    }
    
    pl_MemoryZero (opus, sizeof (PlankOpusFileReader));
    
    p->peer = opus;
    bytesPerSample = sizeof (float);
    p->formatInfo.format = PLANKAUDIOFILE_FORMAT_OPUS;
    p->formatInfo.encoding = PLANKAUDIOFILE_ENCODING_FLOAT_LITTLEENDIAN;
    p->formatInfo.bitsPerSample = PLANKAUDIOFILE_CHARBITS * bytesPerSample;
    
    opus->bufferPosition  = 0;
    opus->bufferFrames    = 0;
    opus->totalFramesRead = 0;
    opus->link            = -1;
    
    if ((result = pl_File_GetMode (file, &mode)) != PlankResult_OK) goto exit;
    
    if (!(mode & PLANKFILE_BINARY))
    {
        result = PlankResult_AudioFileInavlidType;
        goto exit;
    }
    
    if (!(mode & PLANKFILE_READ))
    {
        result = PlankResult_AudioFileInavlidType;
        goto exit;
    }
    
    if (mode & PLANKFILE_BIGENDIAN)
    {
        result = PlankResult_AudioFileInavlidType;
        goto exit;
    }
    
    pl_MemoryCopy (&opus->file, file, sizeof (PlankFile));
    pl_MemoryZero (file, sizeof (PlankFile));
    
    opus->callbacks.read  = &pl_OpusFileReader_ReadCallback;
    opus->callbacks.seek  = &pl_OpusFileReader_SeekCallback;
    opus->callbacks.close = &pl_OpusFileReader_CloseCallback;
    opus->callbacks.tell  = &pl_OpusFileReader_TellCallback;
    
    opus->oggOpusFile = op_open_callbacks (p, &opus->callbacks, NULL, 0, &err);
    
    if (err != 0)
    {
        result = PlankResult_AudioFileInavlidType;
        goto exit;
    }
    
    tags = op_tags (opus->oggOpusFile, -1);
    
    p->formatInfo.numChannels       = op_channel_count (opus->oggOpusFile, -1);
    p->formatInfo.sampleRate        = PLANKAUDIOFILE_OPUS_DEFAULTSAMPLERATE;
    p->formatInfo.bytesPerFrame     = p->formatInfo.numChannels * bytesPerSample;
    p->formatInfo.nominalBitRate    = op_bitrate (opus->oggOpusFile, -1);
    p->formatInfo.minimumBitRate    = p->formatInfo.nominalBitRate;
    p->formatInfo.maximumBitRate    = p->formatInfo.nominalBitRate;
    p->formatInfo.frameDuration     = 0.0;
    p->formatInfo.quality           = 0.f;

    numFrames = op_pcm_total (opus->oggOpusFile, -1);
    bufferSize = PLANKAUDIOFILE_OPUS_MAXFRAMESIZE * p->formatInfo.bytesPerFrame;
    
    if ((result = pl_DynamicArray_InitWithItemSizeAndSize (&opus->buffer, 1, bufferSize, PLANK_FALSE)) != PlankResult_OK) goto exit;
    
    if (numFrames < 0) // should really allow this for continuous streams or nonseekable..
    {
        result = PlankResult_UnknownError;
        goto exit;
    }
    
    p->numFrames = numFrames;
    p->readFramesFunction       = pl_AudioFileReader_Opus_ReadFrames;
    p->setFramePositionFunction = pl_AudioFileReader_Opus_SetFramePosition;
    p->getFramePositionFunction = pl_AudioFileReader_Opus_GetFramePosition;
    
exit:
    return result;
}

PlankResult pl_AudioFileReader_Opus_Close (PlankAudioFileReaderRef p)
{
    PlankOpusFileReaderRef opus;
    PlankResult result = PlankResult_OK;
    PlankMemoryRef m = pl_MemoryGlobal();
    
    if (p == PLANK_NULL)
    {
        result = PlankResult_MemoryError;
        goto exit;
    }
    
    opus = (PlankOpusFileReaderRef)p->peer;
    op_free (opus->oggOpusFile);
    opus->oggOpusFile = PLANK_NULL;
    
    if ((result = pl_DynamicArray_DeInit (&opus->buffer)) != PlankResult_OK) goto exit;
    
    pl_Memory_Free (m, opus);
    
exit:
    return result;
}

PlankResult pl_AudioFileReader_Opus_ReadFrames (PlankAudioFileReaderRef p, const int numFrames, void* data, int *framesReadOut)
{
    PlankResult result;
    PlankOpusFileReaderRef opus;
    int numFramesRemaining, bufferFramesRemaining, bufferFramePosition;
    int bufferSizeInBytes, bytesPerFrame, bufferFrameEnd, link;
    int framesThisTime, numChannels, framesRead, streamFrameEnd;
    float* buffer;
    float* dst;
    OggOpusFile* file;
    
    result = PlankResult_OK;
    opus = (PlankOpusFileReaderRef)p->peer;
    file = opus->oggOpusFile;
    
    numFramesRemaining      = numFrames;
    bufferFramesRemaining   = opus->bufferFrames;         // starts at 0
    bufferFramePosition     = opus->bufferPosition;       // starts at 0
    bufferSizeInBytes       = pl_DynamicArray_GetSize (&opus->buffer);
    bytesPerFrame           = p->formatInfo.bytesPerFrame;
    numChannels             = p->formatInfo.numChannels;
    bufferFrameEnd          = bufferSizeInBytes / bytesPerFrame;
    buffer                  = (float*)pl_DynamicArray_GetArray (&opus->buffer);
    dst                     = (float*)data;
    streamFrameEnd          = p->numFrames;
    link                    = opus->link;
    
    framesRead = 0;
    
    while (numFramesRemaining > 0)
    {
        if (bufferFramesRemaining > 0)
        {
            framesThisTime = pl_MinI (bufferFramesRemaining, numFramesRemaining);
            
            pl_MemoryCopy (dst, buffer + bufferFramePosition * numChannels, framesThisTime * bytesPerFrame);
            
            bufferFramePosition += framesThisTime;
            bufferFramesRemaining -= framesThisTime;
            numFramesRemaining -= framesThisTime;
            framesRead += framesThisTime;
            
            dst += framesThisTime * numChannels;
        }
        
        if (bufferFramesRemaining == 0)
        {            
            framesThisTime = op_read_float (file, buffer, bufferFrameEnd, &link);
            
            if (link != opus->link)
            {
                opus->link = link;
                
                if (op_channel_count (file, -1) != numChannels)
                {
                    result = PlankResult_UnknownError;
                    goto exit;
                }                
            }
            
            if (framesThisTime == 0)
            {
                result = PlankResult_FileEOF;
                goto exit;
            }
            else if (framesThisTime < 0)
            {
                // other error
                result = PlankResult_FileReadError;
                goto exit;
            }
            
            bufferFramePosition = 0;
            bufferFramesRemaining = framesThisTime;
            opus->totalFramesRead += framesThisTime;            
        }
    }
    
exit:
    if (numFramesRemaining > 0)
        pl_MemoryZero (dst, numFramesRemaining * bytesPerFrame);
    
    opus->bufferFrames   = bufferFramesRemaining;
    opus->bufferPosition = bufferFramePosition;
    
    *framesReadOut = framesRead;
    
    return result;
}

PlankResult pl_AudioFileReader_Opus_SetFramePosition (PlankAudioFileReaderRef p, const PlankLL frameIndex)
{
    PlankOpusFileReaderRef opus;
    int err;
    
    opus = (PlankOpusFileReaderRef)p->peer;
    err = op_pcm_seek (opus->oggOpusFile, frameIndex);
    
    if (err != 0)
        return PlankResult_FileSeekFailed;
    
    opus->bufferFrames   = 0;
    opus->bufferPosition = 0;
    
    return PlankResult_OK;
}

PlankResult pl_AudioFileReader_Opus_GetFramePosition (PlankAudioFileReaderRef p, PlankLL *frameIndex)
{
    PlankOpusFileReaderRef opus;
    PlankLL pos;
    
    opus = (PlankOpusFileReaderRef)p->peer;
    pos = op_pcm_tell (opus->oggOpusFile);
    
    if (pos < 0)
        return PlankResult_FileSeekFailed;
    
    *frameIndex = pos;
    
    return PlankResult_OK;
}

#if PLANK_APPLE
#pragma mark Opus Callbacks
#endif

size_t pl_OpusFileReader_ReadCallback (PlankP ptr, size_t size, size_t nmemb, PlankP datasource)
{
    size_t ret;
    PlankResult result;
    PlankAudioFileReaderRef p;
    PlankOpusFileReaderRef opus;
    int bytesRead;
    
    p = (PlankAudioFileReaderRef)datasource;
    opus = (PlankOpusFileReaderRef)p->peer;
    
    result = pl_File_Read ((PlankFileRef)opus, ptr, (int)(size * nmemb) / size, &bytesRead);
    ret = bytesRead > 0 ? bytesRead : 0;
    
    if ((result != PlankResult_OK) && (result != PlankResult_FileEOF))
        errno = -1;
    
    return ret;
}

int pl_OpusFileReader_SeekCallback (PlankP datasource, PlankLL offset, int code)
{
    PlankResult result;
    PlankAudioFileReaderRef p;
    PlankOpusFileReaderRef opus;
    PlankFileRef file;
    
    p = (PlankAudioFileReaderRef)datasource;
    opus = (PlankOpusFileReaderRef)p->peer;
    file = (PlankFileRef)opus;
    
    result = (file->setPositionFunction) (file, offset, code);
    
    return result == PlankResult_OK ? 0 : -1;
}

int pl_OpusFileReader_CloseCallback (PlankP datasource)
{
    PlankResult result;
    PlankAudioFileReaderRef p;
    PlankOpusFileReaderRef opus;
    PlankFileRef file;
    
    p = (PlankAudioFileReaderRef)datasource;
    opus = (PlankOpusFileReaderRef)p->peer;
    file = (PlankFileRef)opus;
    
    result = pl_File_DeInit (file);
    
    return result == PlankResult_OK ? 0 : OP_FALSE;
}

PlankLL pl_OpusFileReader_TellCallback (PlankP datasource)
{
    PlankResult result;
    PlankAudioFileReaderRef p;
    PlankOpusFileReaderRef opus;
    PlankFileRef file;
    PlankLL position;
    
    p = (PlankAudioFileReaderRef)datasource;
    opus = (PlankOpusFileReaderRef)p->peer;
    file = (PlankFileRef)opus;
    
    result = pl_File_GetPosition (file, &position);
    
    return result == PlankResult_OK ? position : (PlankLL)OP_FALSE;
}
#endif // PLANK_OPUS

// -- MultiFile Functions -- //////////////////////////////////////////////////

#if PLANK_APPLE
#pragma mark Multi
#endif

typedef struct PlankMultiAudioFileReader* PlankMultiAudioFileReaderRef;
typedef PlankResult (*PlankMultiAudioFileNextFunction)(PlankMultiAudioFileReaderRef);

typedef struct PlankMultiAudioFileReader
{
    PlankFile originalMulti;
    void* source;
    PlankAudioFileReaderRef currentAudioFile;
    PlankMultiAudioFileNextFunction nextFuntion;
    int mode;
    int index;
} PlankMultiAudioFileReader;


PlankResult pl_MultiAudioFileReaderArrayNextFunction (PlankMultiAudioFileReaderRef p);
PlankResult pl_MultiAudioFileReaderQueueNextFunction (PlankMultiAudioFileReaderRef p);

//..

PlankResult pl_AudioFileReader_Multi_OpenWithFile (PlankAudioFileReaderRef p, PlankFileRef file)
{
    PlankResult result = PlankResult_OK;
    PlankMultiAudioFileReaderRef amul;
    PlankMemoryRef m;
    PlankMulitFileReaderRef multi;
    PlankDynamicArrayRef fileArray, audioFileArray;
    PlankAudioFileReaderRef audioFile;
    PlankFileRef originalFile;
    int arraySize, i;
    
    m = pl_MemoryGlobal();

    amul = (PlankMultiAudioFileReaderRef)pl_Memory_AllocateBytes(m, sizeof (PlankMultiAudioFileReader));
    
    if (amul == PLANK_NULL)
    {
        result = PlankResult_MemoryError;
        goto exit;
    }

    pl_MemoryZero (amul, sizeof(PlankMultiAudioFileReader));
    p->peer = amul;
    
    pl_MemoryCopy (&amul->originalMulti, file, sizeof (PlankFile));
    pl_MemoryZero (file, sizeof (PlankFile));
    file = (PlankFileRef)PLANK_NULL;
    
    // we now own the file
    // also, assume we have checked that the file is definitely a multi

    multi = (PlankMulitFileReaderRef)amul->originalMulti.stream;
    pl_MultiFileReader_GetMode (multi, &amul->mode);
    pl_MultiFileReader_GetIndex (multi, &amul->index);
    
    switch (amul->mode)
    {
        case PLANKMULITFILE_MODE_ARRAYSEQUENCEONCE:
        case PLANKMULITFILE_MODE_ARRAYSEQUENCELOOP:
        case PLANKMULITFILE_MODE_ARRAYRANDOM:
        case PLANKMULITFILE_MODE_ARRAYRANDOMNOREPEAT:
        case PLANKMULITFILE_MODE_ARRAYCALLBACK:
            amul->nextFuntion = pl_MultiAudioFileReaderArrayNextFunction;
            amul->currentAudioFile = (PlankAudioFileReaderRef)PLANK_NULL;
            pl_MultiFileReader_GetArray (multi, &fileArray);
            arraySize = pl_DynamicArray_GetSize (fileArray);
            audioFileArray = pl_DynamicArray_Create();
            
            if (!audioFileArray)
            {
                result = PlankResult_MemoryError;
                goto exit;
            }
            
            pl_DynamicArray_InitWithItemSizeAndSize (audioFileArray, sizeof (PlankAudioFileReader), arraySize, PLANK_TRUE);
            amul->source = audioFileArray;
            
            // open all the files..
            for (i = 0; i < arraySize; ++i)
            {
                audioFile = ((PlankAudioFileReader*)pl_DynamicArray_GetArray (audioFileArray)) + i;
                originalFile = ((PlankFile*)pl_DynamicArray_GetArray (fileArray)) + i;
                pl_AudioFileReader_OpenWithFile (audioFile, originalFile, PLANK_FALSE);
                
                if (amul->index == i)
                    amul->currentAudioFile = audioFile;
            }
            
            break;
        case PLANKMULITFILE_MODE_QUEUE:
            amul->nextFuntion = pl_MultiAudioFileReaderQueueNextFunction;
            amul->currentAudioFile = pl_AudioFileReader_CreateAndInit();
            amul->source = multi->source;
            
            // get the first file (if it exists)
            result = pl_MultiAudioFileReaderQueueNextFunction (amul);
            break;
        default:
            result = PlankResult_UnknownError;
            goto exit;
    }
    
    if (pl_AudioFileReader_GetFile (amul->currentAudioFile) != PLANK_NULL)
        pl_MemoryCopy (&p->formatInfo, &amul->currentAudioFile->formatInfo, sizeof (PlankAudioFileFormatInfo));
    
    p->readFramesFunction       = pl_AudioFileReader_Multi_ReadFrames;
    p->setFramePositionFunction = pl_AudioFileReader_Multi_SetFramePosition;
    p->getFramePositionFunction = pl_AudioFileReader_Multi_GetFramePosition;
    
exit:
    return result;
}

PlankResult pl_AudioFileReader_Multi_Close (PlankAudioFileReaderRef p)
{
    PlankResult result = PlankResult_OK;
    PlankMultiAudioFileReaderRef amul;
    PlankDynamicArrayRef audioFileArray;
    PlankAudioFileReaderRef audioFile;
    int arraySize, i;

    amul = (PlankMultiAudioFileReaderRef)p->peer;
    
    switch (amul->mode)
    {
        case PLANKMULITFILE_MODE_ARRAYSEQUENCEONCE:
        case PLANKMULITFILE_MODE_ARRAYSEQUENCELOOP:
        case PLANKMULITFILE_MODE_ARRAYRANDOM:
        case PLANKMULITFILE_MODE_ARRAYRANDOMNOREPEAT:
        case PLANKMULITFILE_MODE_ARRAYCALLBACK:
            // here currentAudioFile just points to one of the files in the array
            amul->currentAudioFile = (PlankAudioFileReaderRef)PLANK_NULL;
            audioFileArray = (PlankDynamicArrayRef)amul->source;
            arraySize = pl_DynamicArray_GetSize (audioFileArray);

            for (i = 0; i < arraySize; ++i)
            {
                audioFile = ((PlankAudioFileReader*)pl_DynamicArray_GetArray (audioFileArray)) + i;
                pl_AudioFileReader_DeInit (audioFile);
            }
            
            pl_DynamicArray_Destroy (audioFileArray);
            break;
        case PLANKMULITFILE_MODE_QUEUE:
            // here currentAudioFile was allocated by us
            pl_AudioFileReader_Destroy (amul->currentAudioFile);
            amul->currentAudioFile = (PlankAudioFileReaderRef)PLANK_NULL;
            break;
        default:
            result = PlankResult_UnknownError;
            goto exit;
    }
    
    result = pl_File_DeInit (&amul->originalMulti);
    pl_MemoryZero (amul, sizeof (PlankMultiAudioFileReader));
    
exit:
    return result;
}

PlankResult pl_AudioFileReader_Multi_ReadFrames (PlankAudioFileReaderRef p, const int numFrames, void* data, int *framesRead)
{
    PlankResult result = PlankResult_OK;
    PlankMultiAudioFileReaderRef amul;
    int framesThisTime, framesReadLocal, maximumFrames, bytesThisTime;
    PlankP ptr;
    PlankB frameFormatChanged;
    
    amul = (PlankMultiAudioFileReaderRef)p->peer;

    framesReadLocal = 0;
    frameFormatChanged = PLANK_FALSE;
    
    // try to get a new file if one isnt open
    
    if (!pl_AudioFileReader_GetFile (amul->currentAudioFile))
    {
        result = (amul->nextFuntion) (amul);
        
        if (result != PlankResult_OK)
            goto exit;
        
        if (pl_AudioFileReader_GetFile (amul->currentAudioFile) != PLANK_NULL)
            pl_AudioFileReader_Multi_UpdateFormat (p, &amul->currentAudioFile->formatInfo, PLANK_NULL);
    }
    
    maximumFrames = numFrames;
    ptr = data;
    
    while (maximumFrames > 0)
    {
        if (frameFormatChanged)
        {
            result = PlankResult_AudioFileFrameFormatChanged;
            goto exit;
        }
        else if (!pl_AudioFileReader_GetFile (amul->currentAudioFile))
        {
            result = PlankResult_FileEOF;
            goto exit;
        }
        else
        {
            result = pl_AudioFileReader_ReadFrames (amul->currentAudioFile, maximumFrames, ptr, &framesThisTime);
            
            if ((result != PlankResult_OK) && (result != PlankResult_FileEOF))
                goto exit;
            
            bytesThisTime = framesThisTime * amul->currentAudioFile->formatInfo.bytesPerFrame;

            if (framesThisTime < maximumFrames)
            {
                result = (amul->nextFuntion) (amul);
                                
                if (result != PlankResult_OK)
                    goto exit;
                
                if (pl_AudioFileReader_GetFile (amul->currentAudioFile) != PLANK_NULL)
                    pl_AudioFileReader_Multi_UpdateFormat (p, &amul->currentAudioFile->formatInfo, &frameFormatChanged);
            }
        }
        
        framesReadLocal += framesThisTime;
        maximumFrames -= framesThisTime;
        ptr += bytesThisTime;
    }
    
exit:
    if (framesRead != PLANK_NULL)
        *framesRead = framesReadLocal;
    
    return result;    
}

PlankResult pl_AudioFileReader_Multi_SetFramePosition (PlankAudioFileReaderRef p, const PlankLL frameIndex)
{
    (void)p;
    (void)frameIndex;
    return PlankResult_FileSeekFailed;
}

PlankResult pl_AudioFileReader_Multi_GetFramePosition (PlankAudioFileReaderRef p, PlankLL *frameIndex)
{
    (void)p;
    *frameIndex = 0;
    return PlankResult_FileSeekFailed;
}

PlankResult pl_AudioFileReader_Multi_UpdateFormat (PlankAudioFileReaderRef p, const PlankAudioFileFormatInfo* newFormat, PlankB* frameFormatChanged)
{
    PlankResult result = PlankResult_OK;
    PlankB changed;
    
    changed = PLANK_FALSE;
    
    if (p->formatInfo.encoding != newFormat->encoding)
    {
        changed = PLANK_TRUE;
        goto exit;
    }
    
    if (p->formatInfo.numChannels != newFormat->numChannels)
    {
        changed = PLANK_TRUE;
        goto exit;
    }
    
    if (p->formatInfo.bytesPerFrame != newFormat->bytesPerFrame)
    {
        changed = PLANK_TRUE;
        goto exit;
    }
    
    if (p->formatInfo.sampleRate != newFormat->sampleRate)
    {
        changed = PLANK_TRUE;
        goto exit;
    }
    
exit:
    pl_MemoryCopy (&p->formatInfo, newFormat, sizeof (PlankAudioFileFormatInfo));
    
    if (frameFormatChanged != PLANK_NULL)
        *frameFormatChanged = changed;
    
    return result;
}

// multi callbacks

PlankResult pl_MultiAudioFileReaderArrayNextFunction (PlankMultiAudioFileReaderRef p)
{
    PlankResult result = PlankResult_OK;
    PlankMulitFileReaderRef multi;
    PlankDynamicArrayRef array;
    PlankAudioFileReaderRef audioFile;
    PlankAudioFileReader* audioFileArray;
    
    int arraySize, index;

    audioFile = (PlankAudioFileReaderRef)PLANK_NULL;
    multi = (PlankMulitFileReaderRef)p->originalMulti.stream;
    
    result = (multi->nextFuntion) (multi);
    
    if (result == PlankResult_OK)
    {
        array     = (PlankDynamicArrayRef)p->source;
        arraySize = pl_DynamicArray_GetSize (array);
        
        pl_MultiFileReader_GetIndex (multi, &index);
        
        if ((index >= 0) && (index < arraySize))
        {
            audioFileArray = (PlankAudioFileReader*)pl_DynamicArray_GetArray (array);
            audioFile = audioFileArray + index;
            p->index = index;
        }
        
        p->currentAudioFile = audioFile;
    }
    
exit:
    return result;
}

PlankResult pl_MultiAudioFileReaderQueueNextFunction (PlankMultiAudioFileReaderRef p)
{
    PlankResult result = PlankResult_OK;
    PlankMulitFileReaderRef multi;
    PlankFileRef file;
    PlankLockFreeQueueRef q;
    PlankLockFreeQueueElementRef e;
    
    file = (PlankFileRef)PLANK_NULL;
    q = (PlankLockFreeQueueRef)p->source;
    e = (PlankLockFreeQueueElementRef)PLANK_NULL;
    multi = (PlankMulitFileReaderRef)p->originalMulti.stream;
    
    if ((result = pl_LockFreeQueue_Pop (q, &e)) != PlankResult_OK)
        goto exit;
    
    if (e)
    {
        file = (PlankFileRef)pl_LockFreeQueueElement_GetData (e);
        pl_LockFreeQueueElement_Destroy (e);
    }
    
    pl_AudioFileReader_DeInit (p->currentAudioFile);
    
    if (file)
    {
        pl_AudioFileReader_OpenWithFile (p->currentAudioFile, file, PLANK_FALSE);
        
        if (q->freeFunction)
            result = (q->freeFunction) (file);
    }
    
exit:    
    return result;
}



