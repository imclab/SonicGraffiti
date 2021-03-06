#include "pkmEXTAudioFileWriter.h"
#include <libgen.h>
#include <Accelerate/Accelerate.h>

bool pkmEXTAudioFileWriter::open(string mPath, int frameSize)
{
	bReady = false;
	OSStatus err = noErr;
	CFURLRef mURL = CFURLCreateFromFileSystemRepresentation(NULL,  
															(const UInt8 *) mPath.c_str(), 
															mPath.size(), 
															false);

	mFormat.mSampleRate         = 44100.00;
	mFormat.mFormatID           = kAudioFormatLinearPCM;
	mFormat.mFormatFlags        = kAudioFormatFlagsNativeFloatPacked;//kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;	// kLinearPCMFormatFlagIsFloat
	mFormat.mFramesPerPacket    = 1;
	mFormat.mChannelsPerFrame   = 1;
	mFormat.mBitsPerChannel     = sizeof(float) * 8;//16;
	mFormat.mBytesPerPacket     = 4;//2;
	mFormat.mBytesPerFrame      = 4;//2;
	mFormat.mReserved           = 0;

	err = ExtAudioFileCreateWithURL(mURL, 
									kAudioFileWAVEType,
									&mFormat,
									NULL,
									kAudioFileFlags_EraseFile, 
									&mSoundID);
	
	CFRelease(mURL); mURL = NULL;
	
	if(err != noErr){
		printf("[pkmEXTAudioFileWriter]: Error opening target file %s for writing: %d\n", mPath.c_str(), err);
		close();
		return false;
	}
	
	
	err = ExtAudioFileSetProperty(mSoundID, kExtAudioFileProperty_ClientDataFormat, sizeof(mFormat), &mFormat);
	if (err != noErr)
	{
		printf("[pkmEXTAudioFileWriter]: ExtAudioFileSetProperty failed: %d\n", err);
		close();
		return false;
	}
	
	// pull out basic properties
	mFrameRate		= (unsigned) mFormat.mSampleRate;
	mNumChannels	= (unsigned) mFormat.mChannelsPerFrame;
	mBytesPerSample = (unsigned) mFormat.mBitsPerChannel / 8;
	mNumFrames		= 0;
	//printf("[pkmEXTAudioFileWriter]: opened %s (%d hz, %d ch, %d samples, %d bps)\n", mPath.c_str(), mFrameRate, mNumChannels, mNumFrames, mBytesPerSample);
	
	//short_frame = (short *)malloc(sizeof(short) * frameSize);
	//float_frame = (float *)malloc(sizeof(float) * frameSize);
	
	bReady = true;
    
    return true;
}

void pkmEXTAudioFileWriter::close()
{
	bReady = false;
	ExtAudioFileDispose(mSoundID);	
	//free(short_frame);
	//free(float_frame);
}	

void pkmEXTAudioFileWriter::write(float *frame, long start, long count)
{
    
	if (!bReady) {
		return;
	}
	OSStatus err = noErr;
	UInt32 size = count;
	int offset = 0;
	
	while (offset < count)
	{
		// number of frames
		size = count - offset;	
		
		// convert to 16-bit integer [-32767.0, 32767.0]
		//float factor = 32767.0;
		//vDSP_vsmul(frame + offset, 1, &factor, float_frame, 1, size);
		//vDSP_vfixr16(float_frame, 1, short_frame, 1, size);
		
		AudioBufferList buf;
		buf.mNumberBuffers = 1;
		buf.mBuffers[0].mNumberChannels = 1;
		buf.mBuffers[0].mDataByteSize = (count - offset) * mNumChannels * mBytesPerSample;
		buf.mBuffers[0].mData = frame + offset;//short_frame;
		err = ExtAudioFileWrite(mSoundID, size, &buf);
		
		mNumFrames += size;
		
		if (err != noErr)
		{
			printf("[pkmEXTAudioFileWriter]: ExtAudioFileWriteAsync failed: %d\n", err);
			return;
		}	
		else
		{
			offset += size;
			
			if (size == 0)
				break;
		}
	}

}