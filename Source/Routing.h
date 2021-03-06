/*
  ==============================================================================

    Routing.h
    Created: 31 Mar 2015 6:06:27am
    Author:  makira

  ==============================================================================
*/

#ifndef ROUTING_H_INCLUDED
#define ROUTING_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

#define kChannels (16)
#define kMaxSize (1024)

class Router
{
private:
	Router();
	~Router();

	AudioSampleBuffer mOutputs;
	SpinLock mLock;

public:
	static Router & instance();
	
	void accumulate(int channels, int frames, const AudioSampleBuffer &buffer);
	void reset() { mOutputs.clear(); }
	void clear(int channel) { mOutputs.clear(channel, 0, mOutputs.getNumSamples()); }
	
	float ** outputBuffers(int frames);
};

#endif  // ROUTING_H_INCLUDED
