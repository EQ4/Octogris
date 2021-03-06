/*
 ==============================================================================
 Octogris2: multichannel sound spatialization plug-in.
 
 Copyright (C) 2015  GRIS-UdeM
 
 Trajectories.cpp
 Created: 3 Aug 2014 11:42:38am
 
 Developers: Antoine Missout, Vincent Berthiaume
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ==============================================================================
 */


#include "Trajectories.h"
#include "PluginProcessor.h"
#include "SourceMover.h"

// ==============================================================================
void Trajectory::start() {
    if (s_bTrajMover){
        mMover->begin(mFilter->getSrcSelected(), kTrajectory);
    } else if (s_bUseOneSource){
        int j = 0;
        mFilter->beginParameterChangeGesture(mFilter->getParamForSourceX(j));
        mFilter->beginParameterChangeGesture(mFilter->getParamForSourceY(j));
    } else {
        if (mSource < 0) {
            //no source selected, so automation on all sources
            for (int j = 0; j < mFilter->getNumberOfSources(); j++) {
                mFilter->beginParameterChangeGesture(mFilter->getParamForSourceX(j));
                mFilter->beginParameterChangeGesture(mFilter->getParamForSourceY(j));
            }
        } else {
            mFilter->beginParameterChangeGesture(mFilter->getParamForSourceX(mSource));
            mFilter->beginParameterChangeGesture(mFilter->getParamForSourceY(mSource));
        }
    }
    
    for (int i = 0; i < mFilter->getNumberOfSources(); i++){
        mSourcesInitRT.add(mFilter->getSourceRT(i));
    }
    
	mStarted = true;
}

bool Trajectory::process(float seconds, float beats) {
	if (mStopped) return true;
    if (!mStarted) {
            start();
    }
	if (mDone == mTotalDuration) {
		spProcess(0, 0);
        stop();
        
		return true;
	}

	float duration = mBeats ? beats : seconds;
	spProcess(duration, seconds);
	
	mDone += duration;
	if (mDone > mTotalDuration)
		mDone = mTotalDuration;
	
	return false;
}

float Trajectory::progress()
{
	return mDone / mTotalDuration;
}

void Trajectory::stop()
{
	if (!mStarted || mStopped) return;

    if (s_bTrajMover){
        mMover->end(kTrajectory);
    } else if (s_bUseOneSource){
        int j = 0;
        mFilter->endParameterChangeGesture(mFilter->getParamForSourceX(j));
        mFilter->endParameterChangeGesture(mFilter->getParamForSourceY(j));
    } else {
        if (mSource < 0){
            //no source selected, so automation on all sources
            for (int j = 0; j < mFilter->getNumberOfSources(); j++) {
                mFilter->endParameterChangeGesture(mFilter->getParamForSourceX(j));
                mFilter->endParameterChangeGesture(mFilter->getParamForSourceY(j));
            }
        } else {
            mFilter->endParameterChangeGesture(mFilter->getParamForSourceX(mSource));
            mFilter->endParameterChangeGesture(mFilter->getParamForSourceY(mSource));
        }
    }
	mStopped = true;
}

Trajectory::Trajectory(OctogrisAudioProcessor *filter, SourceMover *p_pMover, float duration, bool beats, float times, int source)
    :mFilter(filter)
    ,mMover(p_pMover)
	,mStarted(false)
	,mStopped(false)
	,mDone(0)
	,mDurationSingleTraj(duration)
	,mBeats(beats)
	,mSource(source)
{
	if (mDurationSingleTraj < 0.0001) mDurationSingleTraj = 0.0001;
	if (times < 0.0001) times = 0.0001;
	mTotalDuration = mDurationSingleTraj * times;
}

std::unique_ptr<vector<String>> Trajectory::getTrajectoryPossibleDirections(int p_iTrajectory){
    unique_ptr<vector<String>> vDirections (new vector<String>);
    
    switch(p_iTrajectory) {
        case Circle:
        case EllipseTr:
            vDirections->push_back("Clockwise");
            vDirections->push_back("Counter Clockwise");
            break;
        case Spiral:
            vDirections->push_back("In, Clockwise");
            vDirections->push_back("In, Counter Clockwise");
            vDirections->push_back("Out, Clockwise");
            vDirections->push_back("Out, Counter Clockwise");
            break;
        case Pendulum:
            vDirections->push_back("In");
            vDirections->push_back("Out");
            vDirections->push_back("Crossover");
            break;
        case AllTrajectoryTypes::Random:
            vDirections->push_back("Slow");
            vDirections->push_back("Mid");
            vDirections->push_back("Fast");
            break;
        case RandomTarget:
        case SymXTarget:
        case SymYTarget:
        case ClosestSpeakerTarget:
            return nullptr;
            
        default:
            jassert(0);
    }
    
    return vDirections;
}

unique_ptr<AllTrajectoryDirections> Trajectory::getTrajectoryDirection(int p_iSelectedTrajectory, int p_iSelectedDirection){
    
    unique_ptr<AllTrajectoryDirections> pDirection (new AllTrajectoryDirections);
    
    switch (p_iSelectedTrajectory) {
            
        case Circle:
        case EllipseTr:
            *pDirection = static_cast<AllTrajectoryDirections>(p_iSelectedDirection);
            break;
        case Spiral:
            *pDirection = static_cast<AllTrajectoryDirections>(p_iSelectedDirection+5);
            break;
        case Pendulum:
            *pDirection = static_cast<AllTrajectoryDirections>(p_iSelectedDirection+2);
            break;
        case AllTrajectoryTypes::Random:
            *pDirection = static_cast<AllTrajectoryDirections>(p_iSelectedDirection+9);
            break;
        case RandomTarget:
        case SymXTarget:
        case SymYTarget:
        case ClosestSpeakerTarget:
            *pDirection = None;
            break;
        default:
            jassert(0);
    }
    
    return pDirection;
}

std::unique_ptr<vector<String>> Trajectory::getTrajectoryPossibleReturns(int p_iTrajectory){
    
    unique_ptr<vector<String>> vReturns (new vector<String>);
    
    switch(p_iTrajectory) {
        case Circle:
        case EllipseTr:
        case AllTrajectoryTypes::Random:
            return nullptr;
        case Spiral:
        case Pendulum:
            vReturns->push_back("One Way");
            vReturns->push_back("Return");
            break;
        case RandomTarget:
        case SymXTarget:
        case SymYTarget:
        case ClosestSpeakerTarget:
            return nullptr;
        default:
            jassert(0);
    }
    
    return vReturns;
}



// ==============================================================================
class CircleTrajectory : public Trajectory
{
public:
    CircleTrajectory(OctogrisAudioProcessor *filter, SourceMover *p_pMover, float duration, bool beats, float times, int source, bool ccw)
    : Trajectory(filter, p_pMover, duration, beats, times, source), mCCW(ccw) {}
    
protected:
    void spProcess(float duration, float seconds) {
        float da = mDone / mDurationSingleTraj * (2 * M_PI);
        if (!mCCW) da = -da;
        if (s_bTrajMover){
            FPoint p = mSourcesInitRT.getUnchecked(mFilter->getSrcSelected());
            mMover->move(mFilter->convertRt2Xy01(p.x, p.y+da), kTrajectory);
        } else {
            for (int i = 0; i < mFilter->getNumberOfSources(); i++)
                if (mSource < 0 || mSource == i){
                    bool bWriteAutomation = (s_bUseOneSource && i == 0) ? true : false;
                    FPoint p = mSourcesInitRT.getUnchecked(i);
                    mFilter->setSourceRT(i, FPoint(p.x, p.y + da), bWriteAutomation);
                }
        }
    }
	
private:
	bool mCCW;
};

// ==============================================================================
class SpiralTrajectory : public Trajectory
{
public:
	SpiralTrajectory(OctogrisAudioProcessor *filter, SourceMover *p_pMover, float duration, bool beats, float times, int source, bool ccw, bool in, bool rt)
	: Trajectory(filter, p_pMover, duration, beats, times, source), mCCW(ccw), mIn(in), mRT(rt) {}
	
protected:
    void spProcess(float duration, float seconds)
    {
        float da;
        if (mRT) {
            da = mDone / mDurationSingleTraj * (2 * M_PI);
        } else {
            if (mDone < mTotalDuration) da = fmodf(mDone / mDurationSingleTraj * M_PI, M_PI);
            else da = M_PI;
        }
        if (!mCCW) da = -da;
        
        if (s_bTrajMover){
            FPoint p = mSourcesInitRT.getUnchecked(mFilter->getSrcSelected());
            float l = (cos(da)+1)*0.5;
            float r = mIn ? (p.x * l) : (p.x + (2 - p.x) * (1 - l));
            float t = p.y + 2*da;
            mMover->move(mFilter->convertRt2Xy01(r, t), kTrajectory);
        } else {
            for (int i = 0; i < mFilter->getNumberOfSources(); i++)
                if (mSource < 0 || mSource == i) {
                    FPoint p = mSourcesInitRT.getUnchecked(i);
                    float l = (cos(da)+1)*0.5;
                    float r = mIn ? (p.x * l) : (p.x + (2 - p.x) * (1 - l));
                    float t = p.y + 2*da;
                    bool bWriteAutomation = (s_bUseOneSource && i == 0) ? true : false;
                    mFilter->setSourceRT(i, FPoint(r, t), bWriteAutomation);
                }
        }
    }
	
private:
	bool mCCW, mIn, mRT;
};

// ==============================================================================
class PendulumTrajectory : public Trajectory
{
public:
	PendulumTrajectory(OctogrisAudioProcessor *filter, SourceMover *p_pMover, float duration, bool beats, float times, int source, bool in, bool rt, bool cross)
	: Trajectory(filter, p_pMover, duration, beats, times, source), mIn(in), mRT(rt), mCross(cross) {}
	
protected:
	void spProcess(float duration, float seconds){
        float da;
        float fCurDampening = .5 * mDone / mTotalDuration;
        if (mRT){
            da = mDone / mDurationSingleTraj * (2 * M_PI);
        } else {
            if (mDone < mTotalDuration){
                da = fmodf(mDone / mDurationSingleTraj * M_PI, M_PI);
            } else {
                da = M_PI;
            }
        }
        
        if (s_bTrajMover){
            FPoint p = mSourcesInitRT.getUnchecked(mFilter->getSrcSelected());
            float l = mCross ? cos(da) : (cos(da)+1)*0.5;
            float r = (mCross || mIn) ? (p.x * l) : (p.x + (2 - p.x) * (1 - l));
            
            FPoint newposition = mFilter->convertRt2Xy01(r, p.y);
            JUCE_COMPILER_WARNING("this doesn't work because 0,0 is in bottom corner here...")
            newposition.x = newposition.x - fCurDampening * newposition.x;
            newposition.y = newposition.y - fCurDampening * newposition.y;
            mMover->move(newposition, kTrajectory);
            
            
            
        } else {
            for (int i = 0; i < mFilter->getNumberOfSources(); i++){
                if (mSource < 0 || mSource == i) {
                    FPoint p = mSourcesInitRT.getUnchecked(i);
                    float l = mCross ? cos(da) : (cos(da)+1)*0.5;
                    float r = (mCross || mIn) ? (p.x * l) : (p.x + (2 - p.x) * (1 - l));
                    bool bWriteAutomation = (s_bUseOneSource && i == 0) ? true : false;
                    mFilter->setSourceRT(i, FPoint(r, p.y), bWriteAutomation);
                }
            }
        }
	}
	
private:
	bool mIn, mRT, mCross;
};

// ==============================================================================
class EllipseTrajectory : public Trajectory
{
public:
	EllipseTrajectory(OctogrisAudioProcessor *filter, SourceMover *p_pMover, float duration, bool beats, float times, int source, bool ccw)
	: Trajectory(filter, p_pMover, duration, beats, times, source), mCCW(ccw) {}
	
protected:
	void spProcess(float duration, float seconds)
	{
		float da = mDone / mDurationSingleTraj * (2 * M_PI);
        if (!mCCW) da = -da;
        if (s_bTrajMover){
            // http://www.edmath.org/MATtours/ellipses/ellipses1.07.3.html
            FPoint p = mSourcesInitRT.getUnchecked(mFilter->getSrcSelected());
            float a = 1;
            float b = 0.5;
            float cosDa = cos(da);
            float a2 = a*a;
            float b2 = b*b;
            float cosDa2 = cosDa*cosDa;
            float r2 = (a2*b2)/((b2-a2)*cosDa2+a2);
            float r = sqrt(r2);
            mMover->move(mFilter->convertRt2Xy01(p.x * r, p.y + da), kTrajectory);
        } else {
            for (int i = 0; i < mFilter->getNumberOfSources(); i++){
                if (mSource < 0 || mSource == i)
                {
                    FPoint p = mSourcesInitRT.getUnchecked(i);
                    
                    // http://www.edmath.org/MATtours/ellipses/ellipses1.07.3.html
                    float a = 1;
                    float b = 0.5;
                    float cosDa = cos(da);
                    float a2 = a*a;
                    float b2 = b*b;
                    float cosDa2 = cosDa*cosDa;
                    float r2 = (a2*b2)/((b2-a2)*cosDa2+a2);
                    float r = sqrt(r2);
                    
                    bool bWriteAutomation = (s_bUseOneSource && i == 0) ? true : false;
                    mFilter->setSourceRT(i, FPoint(p.x * r, p.y + da), bWriteAutomation);
                }
            }
        }
    }
	
private:
	bool mCCW;
};

// ==============================================================================
// Mersenne Twister random number generator, this is now included in c++11, see here: http://en.cppreference.com/w/cpp/numeric/random
class MTRand_int32
{
public:
	MTRand_int32()
	{
		seed(rand());
	}
	uint32_t rand_uint32()
	{
		if (p == n) gen_state();
		unsigned long x = state[p++];
		x ^= (x >> 11);
		x ^= (x << 7) & 0x9D2C5680;
		x ^= (x << 15) & 0xEFC60000;
		return x ^ (x >> 18);
	}
	void seed(uint32_t s)
	{
		state[0] = s;
		for (int i = 1; i < n; ++i)
			state[i] = 1812433253 * (state[i - 1] ^ (state[i - 1] >> 30)) + i;

		p = n; // force gen_state() to be called for next random number
	}

private:
	static const int n = 624, m = 397;
	int p;
	unsigned long state[n];
	unsigned long twiddle(uint32_t u, uint32_t v)
	{
		return (((u & 0x80000000) | (v & 0x7FFFFFFF)) >> 1) ^ ((v & 1) * 0x9908B0DF);
	}
	void gen_state()
	{
		for (int i = 0; i < (n - m); ++i)
			state[i] = state[i + m] ^ twiddle(state[i], state[i + 1]);
		for (int i = n - m; i < (n - 1); ++i)
			state[i] = state[i + m - n] ^ twiddle(state[i], state[i + 1]);
		state[n - 1] = state[m - 1] ^ twiddle(state[n - 1], state[0]);
		
		p = 0; // reset position
	}
	// make copy constructor and assignment operator unavailable, they don't make sense
	MTRand_int32(const MTRand_int32&);
	void operator=(const MTRand_int32&);
};


class RandomTrajectory : public Trajectory
{
public:
	RandomTrajectory(OctogrisAudioProcessor *filter, SourceMover *p_pMover, float duration, bool beats, float times, int source, float speed, bool bUniqueTarget)
	: Trajectory(filter, p_pMover, duration, beats, times, source), mClock(0), mSpeed(speed), mUniqueTarget(bUniqueTarget) {}
	
protected:
	void spProcess(float duration, float seconds)
	{
        if (fmodf(mDone, mDurationSingleTraj) < 0.01){
            mFilter->restoreCurrentLocations(mFilter->getSrcSelected());
        }
        
		mClock += seconds;
		while(mClock > 0.01)
		{
			mClock -= 0.01;
            
            float r1 = mRNG.rand_uint32() / (float)0xFFFFFFFF;
            float r2 = mRNG.rand_uint32() / (float)0xFFFFFFFF;
            
            if (s_bTrajMover){
                FPoint p = mFilter->getSourceXY(mFilter->getSrcSelected());
                if (!mUniqueTarget){
                    r1 = mRNG.rand_uint32() / (float)0xFFFFFFFF;
                    r2 = mRNG.rand_uint32() / (float)0xFFFFFFFF;
                }
                p.x += (r1 - 0.5) * mSpeed;
                p.y += (r2 - 0.5) * mSpeed;
                mMover->move(mFilter->convertRt2Xy01(p.x, p.y), kTrajectory);
            } else {
                for (int i = 0; i < mFilter->getNumberOfSources(); i++)
                    if (mSource < 0 || mSource == i)
                    {
                        FPoint p = mFilter->getSourceXY(i);
                        if (!mUniqueTarget){
                            r1 = mRNG.rand_uint32() / (float)0xFFFFFFFF;
                            r2 = mRNG.rand_uint32() / (float)0xFFFFFFFF;
                        }
                        p.x += (r1 - 0.5) * mSpeed;
                        p.y += (r2 - 0.5) * mSpeed;
                        bool bWriteAutomation = (s_bUseOneSource && i == 0) ? true : false;
                        mFilter->setSourceXY(i, p, bWriteAutomation);
                    }
            }
        }
    }
	
private:
	MTRand_int32 mRNG;
	float mClock;
	float mSpeed;
    bool mUniqueTarget;
};

// ==============================================================================
class TargetTrajectory : public Trajectory
{
public:
	TargetTrajectory(OctogrisAudioProcessor *filter, SourceMover *p_pMover, float duration, bool beats, float times, int source)
	: Trajectory(filter, p_pMover, duration, beats, times, source), mCycle(-1) {}
	
protected:
	virtual FPoint destinationForSource(int s, FPoint o) = 0;
    virtual void resetIfRandomTarget(){};

	void spProcess(float duration, float seconds)
	{
		float p = mDone / mDurationSingleTraj;
		
		int cycle = (int)p;
		if (mCycle != cycle)
		{
            
            resetIfRandomTarget();
			mCycle = cycle;
			mSourcesOrigins.clearQuick();
			mSourcesDestinations.clearQuick();
            
			for (int i = 0; i < mFilter->getNumberOfSources(); i++)
			if (mSource < 0 || mSource == i)
			{
                FPoint o = mFilter->getSourceXY(i);
				mSourcesOrigins.add(o);
                mSourcesDestinations.add(destinationForSource(i, o));
                //mSourcesDestinations.add(uniqueDestination);
			}
		}

		float d = fmodf(p, 1);
		for (int i = 0; i < mFilter->getNumberOfSources(); i++)
		if (mSource < 0 || mSource == i)
		{
			FPoint a = mSourcesOrigins.getUnchecked(i);
			FPoint b = mSourcesDestinations.getUnchecked(i);
			FPoint p = a + (b - a) * d;
            bool bWriteAutomation = (s_bUseOneSource && i == 0) ? true : false;
			mFilter->setSourceXY(i, p, bWriteAutomation);
		}
	}
	
private:
	Array<FPoint> mSourcesOrigins;
	Array<FPoint> mSourcesDestinations;
	int mCycle;
};


// ==============================================================================
class RandomTargetTrajectory : public TargetTrajectory
{
public:
	RandomTargetTrajectory(OctogrisAudioProcessor *filter, SourceMover *p_pMover, float duration, bool beats, float times, int source)
	: TargetTrajectory(filter, p_pMover, duration, beats, times, source) {}
	
protected:
	FPoint destinationForSource(int s, FPoint o)
	{
		float r1 = mRNG.rand_uint32() / (float)0xFFFFFFFF;
		float r2 = mRNG.rand_uint32() / (float)0xFFFFFFFF;
		float x = r1 * (kRadiusMax*2) - kRadiusMax;
		float y = r2 * (kRadiusMax*2) - kRadiusMax;
		float r = hypotf(x, y);
		if (r > kRadiusMax)
		{
			float c = kRadiusMax/r;
			x *= c;
			y *= c;
		}
		return FPoint(x,y);
	}
    
    void resetIfRandomTarget(){
        mFilter->restoreCurrentLocations(mFilter->getSrcSelected());
    }
	
private:
	MTRand_int32 mRNG;
};

// ==============================================================================
class SymXTargetTrajectory : public TargetTrajectory
{
public:
	SymXTargetTrajectory(OctogrisAudioProcessor *filter, SourceMover *p_pMover, float duration, bool beats, float times, int source)
	: TargetTrajectory(filter, p_pMover, duration, beats, times, source) {}
	
protected:
	FPoint destinationForSource(int s, FPoint o)
	{
		return FPoint(o.x,-o.y);
	}
};

// ==============================================================================
class SymYTargetTrajectory : public TargetTrajectory
{
public:
	SymYTargetTrajectory(OctogrisAudioProcessor *filter, SourceMover *p_pMover, float duration, bool beats, float times, int source)
	: TargetTrajectory(filter, p_pMover, duration, beats, times, source) {}
	
protected:
	FPoint destinationForSource(int s, FPoint o)
	{
		return FPoint(-o.x,o.y);
	}
};

// ==============================================================================
class ClosestSpeakerTargetTrajectory : public TargetTrajectory
{
public:
	ClosestSpeakerTargetTrajectory(OctogrisAudioProcessor *filter, SourceMover *p_pMover, float duration, bool beats, float times, int source)
	: TargetTrajectory(filter, p_pMover, duration, beats, times, source) {}
	
protected:
	FPoint destinationForSource(int s, FPoint o)
	{
		int bestSpeaker = 0;
		float bestDistance = o.getDistanceFrom(mFilter->getSpeakerXY(0));
		
		for (int i = 1; i < mFilter->getNumberOfSpeakers(); i++)
		{
			float d = o.getDistanceFrom(mFilter->getSpeakerXY(i));
			if (d < bestDistance)
			{
				bestSpeaker = i;
				bestDistance = d;
			}
		}
		
		return mFilter->getSpeakerXY(bestSpeaker);
	}
};


int Trajectory::NumberOfTrajectories() { return TotalNumberTrajectories-1; }

String Trajectory::GetTrajectoryName(int i)
{
    switch(i)
    {
        case Circle: return "Circle";
        case EllipseTr: return "Ellipse";
        case Spiral: return "Spiral";
        case Pendulum: return "Pendulum";
        case AllTrajectoryTypes::Random: return "Random";
        case RandomTarget: return "Random Target";
        case SymXTarget: return "Sym X Target";
        case SymYTarget: return "Sym Y Target";
        case ClosestSpeakerTarget: return "Closest Speaker Target";
    }
    jassert(0);
    return "";
}

Trajectory::Ptr Trajectory::CreateTrajectory(int type, OctogrisAudioProcessor *filter, SourceMover *p_pMover, float duration, bool beats, AllTrajectoryDirections direction, bool bReturn, float times, int source, bool bUniqueTarget)
{
    
    bool ccw, in, cross;
    float speed;
    
    if (direction != None)
    switch (direction) {
        case CW:
            ccw = false;
            break;
        case CCW:
            ccw = true;
            break;
        case In:
            in = true;
            cross = false;
            break;
        case Out:
            in = false;
            cross = false;
            break;
        case Crossover:
            in = true;
            cross = true;
            break;
        case InCW:
            in = true;
            ccw = false;
            break;
        case InCCW:
            in = true;
            ccw = true;
            break;
        case OutCW:
            in = false;
            ccw = false;
            break;
        case OutCCW:
            in = false;
            ccw = true;
            break;
        case Slow:
            speed = .02;
            break;
        case Mid:
            speed = .06;
            break;
        case Fast:
            speed = .1;
            break;
        default:
            break;
    }
    
    
    switch(type)
    {
        case Circle:                     return new CircleTrajectory(filter, p_pMover, duration, beats, times, source, ccw);
        case EllipseTr:                  return new EllipseTrajectory(filter, p_pMover, duration, beats, times, source, ccw);
        case Spiral:                     return new SpiralTrajectory(filter, p_pMover, duration, beats, times, source, ccw, in, bReturn);
        case Pendulum:                   return new PendulumTrajectory(filter, p_pMover, duration, beats, times, source, in, bReturn, cross);
        case AllTrajectoryTypes::Random: return new RandomTrajectory(filter, p_pMover, duration, beats, times, source, speed, bUniqueTarget);
        case RandomTarget:               return new RandomTargetTrajectory(filter, p_pMover, duration, beats, times, source);
        case SymXTarget:                 return new SymXTargetTrajectory(filter, p_pMover, duration, beats, times, source);
        case SymYTarget:                 return new SymYTargetTrajectory(filter, p_pMover, duration, beats, times, source);
        case ClosestSpeakerTarget:       return new ClosestSpeakerTargetTrajectory(filter, p_pMover, duration, beats, times, source);
    }
    jassert(0);
    return NULL;
}