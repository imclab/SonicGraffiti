#pragma once

#include <AudioToolbox/ExtendedAudioFile.h>
#include <string>

using namespace std;

class pkmEXTAudioFileWriter
{
public:
	pkmEXTAudioFileWriter			() {}
	~pkmEXTAudioFileWriter			() {}
	
	bool	open					(string path, int frameSize = 512);
	void	write					(float *target, long start, long count);
	void	close					();
	
	AudioStreamBasicDescription		mFormat;
	ExtAudioFileRef					mSoundID;
	
	unsigned int					mFrameRate,
									mNumChannels,
									mNumFrames,
									mBytesPerSample;	
	
	short							*short_frame;
	float							*float_frame;
	bool							bReady;
};