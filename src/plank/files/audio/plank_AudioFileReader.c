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

#include "../../core/plank_StandardHeader.h"
#include "../plank_File.h"
#include "../plank_IffFileReader.h"
#include "plank_AudioFileReader.h"
#include "../../maths/plank_Maths.h"

// private structures

// private functions and data
typedef PlankResult (*PlankAudioFileReaderReadFramesFunction)(PlankAudioFileReaderRef, const int, void*, int *);
typedef PlankResult (*PlankAudioFileReaderSetFramePositionFunction)(PlankAudioFileReaderRef, const int);
typedef PlankResult (*PlankAudioFileReaderGetFramePositionFunction)(PlankAudioFileReaderRef, int *);

#if PLANK_APPLE
#pragma mark Private Function Declarations
#endif

PlankResult pl_AudioFileReader_WAV_ParseFormat (PlankAudioFileReaderRef p, const PlankUI chunkLength, const PlankLL chunkDataPos);
PlankResult pl_AudioFileReader_WAV_ParseData (PlankAudioFileReaderRef p, const PlankUI chunkLength, const PlankLL chunkDataPos);

PlankResult pl_AudioFileReader_AIFF_ParseFormat (PlankAudioFileReaderRef p, const PlankUI chunkLength, const PlankLL chunkDataPos);
PlankResult pl_AudioFileReader_AIFF_ParseData(PlankAudioFileReaderRef p, const PlankUI chunkLength, const PlankLL chunkDataPos);

PlankResult pl_AudioFileReader_AIFC_ParseVersion (PlankAudioFileReaderRef p, const PlankUI chunkLength, const PlankLL chunkDataPos);
PlankResult pl_AudioFileReader_AIFC_ParseFormat (PlankAudioFileReaderRef p, const PlankUI chunkLength, const PlankLL chunkDataPos);
PlankResult pl_AudioFileReader_AIFC_ParseData (PlankAudioFileReaderRef p, const PlankUI chunkLength, const PlankLL chunkDataPos);

PlankResult pl_AudioFileReader_Iff_Open (PlankAudioFileReaderRef p, const char* filepath);
PlankResult pl_AudioFileReader_Iff_ParseMain  (PlankAudioFileReaderRef p, const PlankFourCharCode mainID, const PlankFourCharCode formatID);
PlankResult pl_AudioFileReader_Iff_ReadFrames (PlankAudioFileReaderRef p, const int numFrames, void* data, int *framesRead);
PlankResult pl_AudioFileReader_Iff_SetFramePosition (PlankAudioFileReaderRef p, const int frameIndex);
PlankResult pl_AudioFileReader_Iff_GetFramePosition (PlankAudioFileReaderRef p, int *frameIndex);

PlankResult pl_AudioFileReader_OggVorbis_Open  (PlankAudioFileReaderRef p, const char* filepath);
PlankResult pl_AudioFileReader_OggVorbis_Close (PlankAudioFileReaderRef p);
PlankResult pl_AudioFileReader_OggVorbis_ReadFrames (PlankAudioFileReaderRef p, const int numFrames, void* data, int *framesRead);
PlankResult pl_AudioFileReader_OggVorbis_SetFramePosition (PlankAudioFileReaderRef p, const int frameIndex);
PlankResult pl_AudioFileReader_OggVorbis_GetFramePosition (PlankAudioFileReaderRef p, int *frameIndex);

PlankResult pl_AudioFileReader_Opus_Open  (PlankAudioFileReaderRef p, const char* filepath);
PlankResult pl_AudioFileReader_Opus_Close (PlankAudioFileReaderRef p);
PlankResult pl_AudioFileReader_Opus_ReadFrames (PlankAudioFileReaderRef p, const int numFrames, void* data, int *framesRead);
PlankResult pl_AudioFileReader_Opus_SetFramePosition (PlankAudioFileReaderRef p, const int frameIndex);
PlankResult pl_AudioFileReader_Opus_GetFramePosition (PlankAudioFileReaderRef p, int *frameIndex);


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
    p->formatInfo.numChannels      = 0;
    p->formatInfo.sampleRate       = 0.0;
    p->dataLength                  = 0;
    p->numFrames                   = 0;
    p->dataPosition                = -1;
    
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
    
    switch (p->formatInfo.format)
    {
        case PLANKAUDIOFILE_FORMAT_WAV:
        case PLANKAUDIOFILE_FORMAT_AIFF:
        case PLANKAUDIOFILE_FORMAT_AIFC:
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
                result = PlankResult_UnknownError;
    }

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
    return (PlankFileRef)p->peer;
}

PlankResult pl_AudioFileReader_Open (PlankAudioFileReaderRef p, const char* filepath)
{
    PlankResult result;
    PlankFourCharCode mainID;
    PlankIffFileReaderRef iff;
    
    result = PlankResult_OK;
    iff = PLANK_NULL;
    
    if ((iff = pl_IffFileReader_CreateAndInit()) == PLANK_NULL)
    {
        result = PlankResult_MemoryError;
        goto exit;
    }
    
    // so the iff reader gets destroyed it we hit an error further down but before we're finished
    p->peer = iff;
    p->formatInfo.format = PLANKAUDIOFILE_FORMAT_UNKNOWNIFF;
    
    // open the file as an IFF
    if ((result = pl_IffFileReader_Open (iff, filepath)) != PlankResult_OK) goto exit;

    // deterimine the file format, could be IFF or Ogg
    if ((result = pl_IffFileReader_GetMainID (iff, &mainID)) != PlankResult_OK) goto exit;

    if ((mainID == pl_FourCharCode ("RIFF")) || // Riff
        (mainID == pl_FourCharCode ("FORM")))   // Iff
    {
        result = pl_AudioFileReader_Iff_Open (p, filepath);
    }
#if PLANK_OGGVORBIS || PLANK_OPUS
    else if (mainID == pl_FourCharCode ("OggS")) //Ogg this needs to handle any Ogg e.g., Vorbis or Opus
    {
        // close the Iff file and start again
        if ((result = pl_IffFileReader_Destroy (iff)) != PlankResult_OK) goto exit;
        
        p->peer = PLANK_NULL;
        p->formatInfo.format = PLANKAUDIOFILE_FORMAT_INVALID;
                
        // something to decide what type of ogg it is...
#if PLANK_OGGVORBIS    
        if (p->peer == PLANK_NULL)
        {
            result = pl_AudioFileReader_OggVorbis_Open (p, filepath);
            
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
            result = pl_AudioFileReader_Opus_Open (p, filepath);
            
            if (result != PlankResult_OK)
            {
                pl_AudioFileReader_Opus_Close (p);
                
                p->peer = PLANK_NULL;
                p->formatInfo.format = PLANKAUDIOFILE_FORMAT_INVALID;
            }
        }
#endif
    }
#endif
    else 
    {
        if ((result = pl_IffFileReader_Destroy (iff)) != PlankResult_OK) goto exit;

        p->peer = PLANK_NULL;
        p->formatInfo.format = PLANKAUDIOFILE_FORMAT_INVALID;
        
        result = PlankResult_AudioFileReaderInavlidType;
    }
    
exit:
    return result;
}

PlankResult pl_AudioFileReader_Close (PlankAudioFileReaderRef p)
{
    if (p == PLANK_NULL || p->peer == PLANK_NULL)
        return PlankResult_FileCloseFailed;
    
    return pl_File_Close ((PlankFileRef)p->peer); 
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

PlankResult pl_AudioFileReader_GetNumFrames (PlankAudioFileReaderRef p, int *numFrames)
{
    *numFrames = (int)p->numFrames;
    return PlankResult_OK;
}

PlankResult pl_AudioFileReader_SetFramePosition (PlankAudioFileReaderRef p, const int frameIndex)
{
    PlankResult result = PlankResult_OK;
    
    if (p->peer == PLANK_NULL)
    {
        result = PlankResult_AudioFileReaderNotReady;
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

PlankResult pl_AudioFileReader_GetFramePosition (PlankAudioFileReaderRef p, int *frameIndex)
{
    PlankResult result = PlankResult_OK;
    
    if (p->peer == PLANK_NULL)
    {
        result = PlankResult_AudioFileReaderNotReady;
        goto exit;
    }
    
    result = ((PlankAudioFileReaderGetFramePositionFunction)p->getFramePositionFunction)(p, frameIndex);
    
exit:
    return result;    
}

PlankResult pl_AudioFileReader_ReadFrames (PlankAudioFileReaderRef p, const int numFrames, void* data, int *framesRead)
{
    return ((PlankAudioFileReaderReadFramesFunction)p->readFramesFunction)(p, numFrames, data, framesRead);
}

// -- WAV Functions -- /////////////////////////////////////////////////////////
#if PLANK_APPLE
#pragma mark WAV Functions
#endif

PlankResult pl_AudioFileReader_WAV_ParseFormat (PlankAudioFileReaderRef p, const PlankUI chunkLength, const PlankLL chunkDataPos)
{
    PlankResult result = PlankResult_OK;
    PlankUS compressionCode, numChannels;
    PlankUI sampleRate, byteRate;
    PlankUS blockAlign, bitsPerSample;

    if ((result = pl_File_ReadUS ((PlankFileRef)p->peer, &compressionCode)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadUS ((PlankFileRef)p->peer, &numChannels)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadUI ((PlankFileRef)p->peer, &sampleRate)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadUI ((PlankFileRef)p->peer, &byteRate)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadUS ((PlankFileRef)p->peer, &blockAlign)) != PlankResult_OK) goto exit;
    if ((result = pl_File_ReadUS ((PlankFileRef)p->peer, &bitsPerSample)) != PlankResult_OK) goto exit;
    
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
        // must implememnt
        result = PlankResult_AudioFileReaderInavlidType;
        goto exit;
    }
    else 
    {
        result = PlankResult_AudioFileReaderInavlidType;
        goto exit;
    }
    
    // set these last so that if the format is not recognised everything remains uninitialised
    p->formatInfo.bitsPerSample = (PlankI) bitsPerSample;
    p->formatInfo.bytesPerFrame = (PlankI) (((bitsPerSample + (0x00000008 - 1)) & ~(0x00000008 - 1)) * numChannels / 8); // round up to whole bytes
    p->formatInfo.numChannels   = (PlankI) numChannels;
    p->formatInfo.sampleRate    = (PlankD) sampleRate;

	(void)chunkDataPos;
	(void)chunkLength;

exit:
    return result;
}

PlankResult pl_AudioFileReader_WAV_ParseData (PlankAudioFileReaderRef p, const PlankUI chunkLength, const PlankLL chunkDataPos)
{
    PlankResult result = PlankResult_OK;
        
    p->dataPosition = chunkDataPos;
            
    if (p->formatInfo.bytesPerFrame > 0)
        p->numFrames = chunkLength / p->formatInfo.bytesPerFrame;

    if ((chunkLength % p->formatInfo.bytesPerFrame) != 0)
        result = PlankResult_AudioFileReaderDataChunkInvalid;
  
    return result;
}

// -- AIFF Functions -- ////////////////////////////////////////////////////////
#if PLANK_APPLE
#pragma mark AIFF Functions
#endif

PlankResult pl_AudioFileReader_AIFF_ParseFormat (PlankAudioFileReaderRef p, const PlankUI chunkLength, const PlankLL chunkDataPos)
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
    
    p->formatInfo.bitsPerSample = (PlankI) bitsPerSample;
    p->formatInfo.bytesPerFrame = (PlankI) (((bitsPerSample + (0x00000008 - 1)) & ~(0x00000008 - 1)) * numChannels / 8); // round up to whole bytes
    p->formatInfo.numChannels   = (PlankI) numChannels;
    p->formatInfo.sampleRate    = (PlankD) pl_F802I (sampleRate);
    p->numFrames                = (PlankLL) numFrames;

	(void)chunkDataPos;
	(void)chunkLength;

exit:
    return PlankResult_OK;
}

PlankResult pl_AudioFileReader_AIFF_ParseData (PlankAudioFileReaderRef p, const PlankUI chunkLength, const PlankLL chunkDataPos)
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

PlankResult pl_AudioFileReader_AIFC_ParseVersion (PlankAudioFileReaderRef p, const PlankUI chunkLength, const PlankLL chunkDataPos)
{
    PlankResult result = PlankResult_OK;
    PlankUI version;
    
    if ((result = pl_File_ReadUI ((PlankFileRef)p->peer, &version)) != PlankResult_OK) goto exit;
    if (version == 0) goto exit;    
    if (version == PLANKAUDIOFILE_AIFC_VERSION) goto exit;
    
    result = PlankResult_AudioFileReaderInavlidType;
    
	(void)chunkDataPos;
	(void)chunkLength;

exit:
    return result;
}

PlankResult pl_AudioFileReader_AIFC_ParseFormat (PlankAudioFileReaderRef p, const PlankUI chunkLength, const PlankLL chunkDataPos)
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
            result = PlankResult_AudioFileReaderInavlidType;
            goto exit;
        }
        
        p->formatInfo.encoding = PLANKAUDIOFILE_ENCODING_FLOAT_BIGENDIAN;
    }
    else if (compressionID == pl_FourCharCode ("fl64"))
    {
        if (p->formatInfo.bitsPerSample != 64)
        {
            result = PlankResult_AudioFileReaderInavlidType;
            goto exit;
        }
        
        p->formatInfo.encoding = PLANKAUDIOFILE_ENCODING_FLOAT_BIGENDIAN;
    }
    
exit:
    return result;
}

PlankResult pl_AudioFileReader_AIFC_ParseData (PlankAudioFileReaderRef p, const PlankUI chunkLength, const PlankLL chunkDataPos)
{
    return pl_AudioFileReader_AIFF_ParseData (p, chunkLength, chunkDataPos);
}

// -- Generic Iff Functions -- /////////////////////////////////////////////////
#if PLANK_APPLE
#pragma mark Generic Iff Functions
#endif

PlankResult pl_AudioFileReader_Iff_Open (PlankAudioFileReaderRef p, const char* filepath)
{
    PlankResult result;
    PlankUI chunkLength;
    PlankLL chunkDataPos;
    PlankIffFileReaderRef iff;
    PlankFourCharCode mainID, formatID;
    (void)filepath;
    
    iff = (PlankIffFileReaderRef)p->peer;
    
    // deterimine the IFF file format, could be IFF or RIFF
    if ((result = pl_IffFileReader_GetMainID (iff, &mainID)) != PlankResult_OK) goto exit;
    if ((result = pl_IffFileReader_GetFormatID (iff, &formatID)) != PlankResult_OK) goto exit;
    if ((result = pl_AudioFileReader_Iff_ParseMain (p, mainID, formatID)) != PlankResult_OK) goto exit;
    
    // parse based on the format
    if (p->formatInfo.format == PLANKAUDIOFILE_FORMAT_WAV)
    {
        if ((result = pl_IffFileReader_SeekChunk (iff, pl_FourCharCode ("fmt "), &chunkLength, &chunkDataPos)) != PlankResult_OK) goto exit;
        if ((result = pl_AudioFileReader_WAV_ParseFormat (p, chunkLength, chunkDataPos)) != PlankResult_OK) goto exit;
        
        if ((result = pl_IffFileReader_SeekChunk (iff, pl_FourCharCode ("data"), &chunkLength, &chunkDataPos)) != PlankResult_OK) goto exit;
        if ((result = pl_AudioFileReader_WAV_ParseData (p, chunkLength, chunkDataPos)) != PlankResult_OK) goto exit;
    }
    else if (p->formatInfo.format == PLANKAUDIOFILE_FORMAT_AIFF)
    {
        if ((result = pl_IffFileReader_SeekChunk (iff, pl_FourCharCode ("COMM"), &chunkLength, &chunkDataPos)) != PlankResult_OK) goto exit;
        if ((result = pl_AudioFileReader_AIFF_ParseFormat (p, chunkLength, chunkDataPos)) != PlankResult_OK) goto exit;
        
        p->formatInfo.encoding = PLANKAUDIOFILE_ENCODING_PCM_BIGENDIAN;
        
        if ((result = pl_IffFileReader_SeekChunk (iff, pl_FourCharCode ("SSND"), &chunkLength, &chunkDataPos)) != PlankResult_OK) goto exit;
        if ((result = pl_AudioFileReader_AIFF_ParseData (p, chunkLength, chunkDataPos)) != PlankResult_OK) goto exit;        
    }
    else if (p->formatInfo.format == PLANKAUDIOFILE_FORMAT_AIFC)
    {
        if ((result = pl_IffFileReader_SeekChunk (iff, pl_FourCharCode ("FVER"), &chunkLength, &chunkDataPos)) != PlankResult_OK) goto exit;
        if ((result = pl_AudioFileReader_AIFC_ParseVersion (p, chunkLength, chunkDataPos)) != PlankResult_OK) goto exit;
        
        if ((result = pl_IffFileReader_SeekChunk (iff, pl_FourCharCode ("COMM"), &chunkLength, &chunkDataPos)) != PlankResult_OK) goto exit;
        if ((result = pl_AudioFileReader_AIFC_ParseFormat (p, chunkLength, chunkDataPos)) != PlankResult_OK) goto exit;
        
        if ((result = pl_IffFileReader_SeekChunk (iff, pl_FourCharCode ("SSND"), &chunkLength, &chunkDataPos)) != PlankResult_OK) goto exit;
        if ((result = pl_AudioFileReader_AIFC_ParseData (p, chunkLength, chunkDataPos)) != PlankResult_OK) goto exit;        
    }
    else
    {
        result = PlankResult_AudioFileReaderInavlidType;
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
                                               const PlankFourCharCode mainID, 
                                               const PlankFourCharCode formatID)
{        
    PlankIffFileReader* iff = PLANK_NULL;
    PlankB isBigEndian = PLANK_FALSE;
    
    iff = (PlankIffFileReader*)p->peer;
    
    if (mainID == pl_FourCharCode ("RIFF"))
    {
        isBigEndian = PLANK_FALSE;
        
        if (formatID == pl_FourCharCode ("WAVE"))
        {
            p->formatInfo.format = PLANKAUDIOFILE_FORMAT_WAV;
        }
        else goto exit;
    }
    else if (mainID == pl_FourCharCode ("FORM"))
    {
        isBigEndian = PLANK_TRUE;
        
        if (formatID == pl_FourCharCode ("AIFF"))
        {
            p->formatInfo.format = PLANKAUDIOFILE_FORMAT_AIFF;
        }
        else if (formatID == pl_FourCharCode ("AIFC"))
        {
            p->formatInfo.format = PLANKAUDIOFILE_FORMAT_AIFC;
        }
        else goto exit;
    }
    else goto exit;
    
    pl_IffFileReader_SetEndian (iff, isBigEndian);
    
    return PlankResult_OK;
    
exit:
    return PlankResult_AudioFileReaderInavlidType;
}

PlankResult pl_AudioFileReader_Iff_ReadFrames (PlankAudioFileReaderRef p, const int numFrames, void* data, int *framesRead)
{
    PlankResult result = PlankResult_OK;
    int startFrame, endFrame, framesToRead, bytesToRead, bytesRead;
    
    if (p->peer == PLANK_NULL)
    {
        result = PlankResult_AudioFileReaderNotReady;
        goto exit;
    }
    
    if ((p->dataPosition < 0) || (p->formatInfo.bytesPerFrame <= 0))
    {
        result = PlankResult_AudioFileReaderNotReady;
        goto exit;
    }
    
    if ((result = pl_AudioFileReader_GetFramePosition (p, &startFrame)) != PlankResult_OK) goto exit;
    
    if (startFrame < 0)
    {
        result = PlankResult_AudioFileReaderInvalidFilePosition;
        goto exit;
    }
    
    endFrame = startFrame + numFrames;
    
    framesToRead = (endFrame > p->numFrames) ? ((int)p->numFrames - startFrame) : (numFrames);
    bytesToRead = framesToRead * p->formatInfo.bytesPerFrame;
    
    result = pl_File_Read ((PlankFileRef)p->peer, data, bytesToRead, &bytesRead);
    
    if (framesRead != PLANK_NULL)
        *framesRead = bytesRead / p->formatInfo.bytesPerFrame;
    
    // should zero if framesToRead < numFrames
    
exit:
    return result;
}

PlankResult pl_AudioFileReader_Iff_SetFramePosition (PlankAudioFileReaderRef p, const int frameIndex)
{
    PlankResult result;
    PlankLL pos;
    
    if ((p->dataPosition < 0) || (p->formatInfo.bytesPerFrame <= 0))
    {
        result = PlankResult_AudioFileReaderNotReady;
        goto exit;
    }
    
    pos = p->dataPosition + frameIndex * p->formatInfo.bytesPerFrame;
    result = pl_File_SetPosition ((PlankFileRef)p->peer, pos);
    
exit:
    return result;
}

PlankResult pl_AudioFileReader_Iff_GetFramePosition (PlankAudioFileReaderRef p, int *frameIndex)
{
    PlankResult result = PlankResult_OK;
    PlankLL pos;
    
    if ((p->dataPosition < 0) || (p->formatInfo.bytesPerFrame <= 0))
    {
        result = PlankResult_AudioFileReaderNotReady;
        goto exit;
    }
    
    if ((result = pl_File_GetPosition ((PlankFileRef)p->peer, &pos)) != PlankResult_OK) goto exit;
    
    *frameIndex = (int)(pos - p->dataPosition) / p->formatInfo.bytesPerFrame;
    
exit:
    return result;
}

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
    PlankI bytesPerSample;
    
    int err, bufferSize;
    vorbis_info* info;
    
    m = pl_MemoryGlobal();
    
    // open as ogg
    ogg = (PlankOggVorbisFileReaderRef)pl_Memory_AllocateBytes (m, sizeof (PlankOggVorbisFileReader));
    
    if (ogg == PLANK_NULL)
    {
        result = PlankResult_MemoryError;
        goto exit;
    }
    
    p->peer = ogg;
    bytesPerSample = sizeof (float);
    p->formatInfo.format = PLANKAUDIOFILE_FORMAT_OGGVORBIS;
    p->formatInfo.encoding = PLANKAUDIOFILE_ENCODING_FLOAT_LITTLEENDIAN;
    p->formatInfo.bitsPerSample = PLANKAUDIOFILE_CHARBITS * bytesPerSample;
    
    pl_MemoryZero (ogg, sizeof (PlankOggVorbisFileReader));
    
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
        result = PlankResult_AudioFileReaderInavlidType;
        goto exit;
    }
    
    info = ov_info (&ogg->oggVorbisFile, -1);
    
    p->formatInfo.numChannels   = info->channels;
    p->formatInfo.sampleRate    = info->rate;
    p->formatInfo.bytesPerFrame = info->channels * bytesPerSample;
    
    numFrames = ov_pcm_total (&ogg->oggVorbisFile, -1);
    
    bufferSize = pl_MinLL (numFrames * p->formatInfo.bytesPerFrame, (PlankLL)4096);
    
    if ((result = pl_DynamicArray_InitWithItemSizeAndSize (&ogg->buffer, 1, bufferSize, PLANK_FALSE)) != PlankResult_OK) goto exit;
    
    if (numFrames < 0)
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

    static int calls = 0;
    int numCalls;
    calls++;

    numCalls = calls;
    
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


PlankResult pl_AudioFileReader_OggVorbis_SetFramePosition (PlankAudioFileReaderRef p, const int frameIndex)
{
    PlankOggVorbisFileReaderRef ogg;
    int err;
    
    ogg = (PlankOggVorbisFileReaderRef)p->peer;
    err = ov_pcm_seek_lap (&ogg->oggVorbisFile, frameIndex); // should probably eventually do my own lapping in readframes?
    
    if (err != 0)
        return PlankResult_FileSeekFailed;
    
    return PlankResult_OK;
}

PlankResult pl_AudioFileReader_OggVorbis_GetFramePosition (PlankAudioFileReaderRef p, int *frameIndex)
{
    PlankOggVorbisFileReaderRef ogg;
    PlankLL pos;
    
    ogg = (PlankOggVorbisFileReaderRef)p->peer;
    pos = ov_pcm_tell (&ogg->oggVorbisFile);
    
    if (pos < 0)
        return PlankResult_FileSeekFailed;
    
    *frameIndex = (int)pos;
    
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

typedef struct PlankOpusFileReader
{
    PlankFile file;
    
    ogg_sync_state      oy;
    ogg_page            og;
    ogg_packet          op;
    ogg_stream_state    os;
    
    PlankLL page_granule;
    PlankLL link_out;
    PlankLL packet_count;
    
    OpusMSDecoder *st;
    
//    int maxFrameSize;
    int stream_init;
    int eos;
    int has_opus_stream;
    int opus_serialno;
    int total_links;
    int mapping_family;
    int preskip;
    int streams;
    int gran_offset;
    
    int rate;
    int channels;
    
    float gain;
    float manual_gain;
//    float *output;
} PlankOpusFileReader;

typedef PlankOpusFileReader* PlankOpusFileReaderRef;

static PlankResult pl_AudioFileReader_Opus_ProcessHeader (PlankAudioFileReaderRef p, 
                                                          ogg_packet *op, 
                                                          float manual_gain, 
                                                          OpusMSDecoder** decoder)
{
    PlankResult result;
    PlankOpusFileReaderRef opus;
    int err;
    OpusMSDecoder *st;
    OpusHeader header;
    
    result = PlankResult_OK;
    opus = (PlankOpusFileReaderRef)p->peer;
    st = PLANK_NULL;
    
    if (opus_header_parse (op->packet, (int)op->bytes, &header) == 0)
        goto exit;
    
    opus->mapping_family = header.channel_mapping;
    opus->channels = header.channels;
    
    
    if (!opus->rate)
        opus->rate = header.input_sample_rate;
    
    /*If the rate is unspecified we decode to 48000*/
    if (opus->rate == 0)
        opus->rate = PLANKAUDIOFILE_OPUS_DEFAULTSAMPLERATE;
    
    if (opus->rate < PLANKAUDIOFILE_OPUS_MINSAMPLERATE || opus->rate > PLANKAUDIOFILE_OPUS_MAXSAMPLERATE)
        opus->rate = PLANKAUDIOFILE_OPUS_DEFAULTSAMPLERATE;
    
    opus->preskip = header.preskip;
    
    st = opus_multistream_decoder_create (PLANKAUDIOFILE_OPUS_DEFAULTSAMPLERATE, header.channels, header.nb_streams, header.nb_coupled, header.stream_map, &err);
    
    if (err != OPUS_OK)
        goto exit;
    
    if (!st)
        goto exit;
    
    opus->streams = header.nb_streams;
    
    if (header.gain != 0 || manual_gain != 0.f)
    {
        /*Gain API added in a newer libopus version, if we don't have it
         we apply the gain ourselves. We also add in a user provided
         manual gain at the same time.*/
        int gainadj = (int)(manual_gain * 256.f) + header.gain;
        
        err = opus_multistream_decoder_ctl (st, OPUS_SET_GAIN (gainadj));
        
        if (err == OPUS_UNIMPLEMENTED)
            opus->gain = pl_PowD (10.0, gainadj / 5120.0);
        else if (err != OPUS_OK)
            goto exit;
    }
    
exit:
    *decoder = st;  // could st leak from one of the error checks?
    return result;    
}

PlankResult pl_AudioFileReader_Opus_Open  (PlankAudioFileReaderRef p, const char* filepath)
{
    PlankResult result;
    PlankOpusFileReaderRef opus;
    PlankMemoryRef m;
    //PlankLL numFrames;  ?? how ??
    PlankI bytesPerSample;

    int err;
    
    m = pl_MemoryGlobal();
    
    // open as opus
    opus = (PlankOpusFileReaderRef)pl_Memory_AllocateBytes (m, sizeof (PlankOpusFileReader));
    
    if (opus == PLANK_NULL)
    {
        result = PlankResult_MemoryError;
        goto exit;
    }
    
    p->peer = opus;
    bytesPerSample = sizeof (float);
    p->formatInfo.format = PLANKAUDIOFILE_FORMAT_OPUS;
    p->formatInfo.encoding = PLANKAUDIOFILE_ENCODING_FLOAT_LITTLEENDIAN;
    p->formatInfo.bitsPerSample = PLANKAUDIOFILE_CHARBITS * bytesPerSample;
    
    pl_MemoryZero (opus, sizeof (PlankOpusFileReader));
    opus->channels = -1;
    opus->gain = 1.f;
    
    result = pl_File_Init (&opus->file);
    result = pl_File_OpenBinaryRead (&opus->file, filepath, PLANK_FALSE, PLANK_FALSE);
    err = ogg_sync_init (&opus->oy);
    
    // need to fill these before exit so need to read a few packets/pages
    /*
    p->formatInfo.numChannels   = info->channels;
    p->formatInfo.sampleRate    = info->rate;
    p->formatInfo.bytesPerFrame = info->channels * bytesPerSample;
    */
    
exit:
    return result;
}

PlankResult pl_AudioFileReader_Opus_Close (PlankAudioFileReaderRef p)
{
    PlankResult result;
    PlankOpusFileReaderRef opus;
    int err;
    
    opus = (PlankOpusFileReaderRef)p->peer;

    if (opus->stream_init)
        err = ogg_stream_clear (&opus->os);
        
    err = ogg_sync_clear (&opus->oy);
        
    result = pl_File_DeInit (&opus->file);
    
    return result;
}

PlankResult pl_AudioFileReader_Opus_ReadFrames (PlankAudioFileReaderRef p, const int numFrames, void* frameData, int *framesReadOut)
{
    PlankResult fileResult, errResult;
    PlankOpusFileReaderRef opus;
    int err;
    char *data;
    int i, nb_read;
    int ret;
    int frame_size;
    PlankLL maxout;
    PlankLL outsamp;
    PlankB frameDone;
    float* output;
    
    if (numFrames > PLANKAUDIOFILE_OPUS_MAXFRAMESIZE)
        return PlankResult_FileReadError; // could handle better than this...
    
    opus = (PlankOpusFileReaderRef)p->peer;

    frameDone = PLANK_FALSE;
    output = (float*)frameData;
    
    while (!frameDone)
    {
        /*Get the ogg buffer for writing*/
        data = ogg_sync_buffer (&opus->oy, PLANKAUDIOFILE_OPUS_READBYTES);
        
        /*Read bitstream from input file*/
        fileResult = pl_File_Read (&opus->file, data, PLANKAUDIOFILE_OPUS_READBYTES, &nb_read);
        
        err = ogg_sync_wrote (&opus->oy, nb_read);
        
        /*Loop for all complete pages we got (most likely only one)*/
        while ((err = ogg_sync_pageout (&opus->oy, &opus->og)) == 1) 
        {
            if (opus->stream_init == 0) 
            {
                err = ogg_stream_init (&opus->os, ogg_page_serialno (&opus->og));
                opus->stream_init = 1;
            }
            
            if (ogg_page_serialno (&opus->og) != opus->os.serialno) 
            {
                /* so all streams are read. */
                err = ogg_stream_reset_serialno (&opus->os, ogg_page_serialno (&opus->og));
            }
            
            /*Add page to the bitstream*/
            err = ogg_stream_pagein (&opus->os, &opus->og);
            opus->page_granule = ogg_page_granulepos (&opus->og);
            
            /*Extract all available packets*/
            while ((err = ogg_stream_packetout (&opus->os, &opus->op)) == 1)
            {
                /*OggOpus streams are identified by a magic string in the initial stream header.*/
                if (opus->op.b_o_s && opus->op.bytes >= 8 && !memcmp (opus->op.packet, PLANKAUDIOFILE_OPUS_HEAD, 8)) 
                {
                    if (!opus->has_opus_stream)
                    {
                        opus->opus_serialno     = opus->os.serialno;
                        opus->has_opus_stream   = 1;
                        opus->link_out          = 0;
                        opus->packet_count      = 0;
                        opus->eos               = 0;
                        opus->total_links++;
                    }
                }
                
                if (!opus->has_opus_stream || opus->os.serialno != opus->opus_serialno)
                    break;
                
                /*If first packet in a logical stream, process the Opus header*/
                if (opus->packet_count == 0)
                {
                    errResult = pl_AudioFileReader_Opus_ProcessHeader (p, &opus->op, opus->manual_gain, &opus->st);
                    
                    if (!opus->st)
                        goto exit;
                    
                    /*Remember how many samples at the front we were told to skip
                     so that we can adjust the timestamp counting.*/
                    opus->gran_offset = opus->preskip;
                }
                else if (opus->packet_count == 1)
                {
                    // comments
                }
                else 
                {
                    // other packets
                    
                    /*End of stream condition*/
                    if (opus->op.e_o_s && opus->os.serialno == opus->opus_serialno)
                        opus->eos = 1; /* don't care for anything except opus eos */
                    
                    ret = opus_multistream_decode_float (opus->st, 
                                                         opus->op.packet, 
                                                         opus->op.bytes, 
                                                         output, 
                                                         numFrames, 0);
                    
                    if (ret < 0)
                        break;
                    
                    frame_size = ret;
                    
                    if (opus->gain != 0)
                    {
                        for (i = 0; i < frame_size * opus->channels; i++)
                            output[i] *= opus->gain;
                    }
                    
                    maxout = ((opus->page_granule - opus->gran_offset) * opus->rate / PLANKAUDIOFILE_OPUS_DEFAULTSAMPLERATE) - opus->link_out;
                    outsamp = maxout; // should be actual samples output..
                    opus->link_out += outsamp;
                    
                    frameDone = PLANK_TRUE;
                    *framesReadOut = (int)outsamp; // or just frame_size ??
                }
                
                opus->packet_count++;
            }
            
            if (opus->eos)
            {
                opus->has_opus_stream = 0;
                
                if (opus->st)
                    opus_multistream_decoder_destroy (opus->st); // possibly don't do this???, not sure how to try and loop
                
                opus->st = NULL;
            }
            
        }
        
        if (fileResult == PlankResult_FileEOF) // or pl_File_IsEOF (&file) ??
            frameDone = PLANK_TRUE;
    }
    
exit:
    return fileResult;
}

PlankResult pl_AudioFileReader_Opus_SetFramePosition (PlankAudioFileReaderRef p, const int frameIndex)
{
    (void)p;
    (void)frameIndex;
    return PlankResult_UnknownError;
}

PlankResult pl_AudioFileReader_Opus_GetFramePosition (PlankAudioFileReaderRef p, int *frameIndex)
{
    (void)p;
    (void)frameIndex;
    return PlankResult_UnknownError;
}

#endif