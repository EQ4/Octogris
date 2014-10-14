/*
  ==============================================================================

    This file was auto-generated by the Introjucer!

    It contains the basic startup code for a Juce application.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "FieldComponent.h"
#include "Trajectories.h"
#include "OctoLeap.h"
#include "OscComponent.h"

//==============================================================================
static const int kMargin = 10;
static const int kDefaultLabelHeight = 18;
static const int kCenterColumnWidth = 180;
static const int kRightColumnWidth = 340;
static const int kParamBoxHeight = 165;
static const int kTimerDelay = 1000 / 20; // 20 fps

//==============================================================================
class MiniProgressBar : public Component
{
public:
	MiniProgressBar() : mValue(0) {}
	void paint(Graphics &g)
	{
		Rectangle<int> box = getLocalBounds();
		
		g.setColour(Colours::black);
		g.fillRect(box);
		
		g.setColour(Colour::fromRGB(0,255,0));
		box.setWidth(box.getWidth() * mValue);
		g.fillRect(box);
	}
	void setValue(float v) { mValue = v; repaint(); }
private:
	float mValue;
};

//==============================================================================
class OctTabbedComponent : public TabbedComponent
{
public:
	OctTabbedComponent(TabbedButtonBar::Orientation orientation, OctogrisAudioProcessor *filter)
	:
		TabbedComponent(orientation),
		mFilter(filter),
		mInited(false)
	{}
	
	void currentTabChanged (int newCurrentTabIndex, const String& newCurrentTabName)
	{
		if (!mInited) return;
		
		//printf("Octogris: currentTabChanged\n");
		mFilter->setGuiTab(newCurrentTabIndex);
	}
	
	void initDone() { mInited = true; }
	
private:
	OctogrisAudioProcessor *mFilter;
	bool mInited;
};

//======================================= BOX ===========================================================================
class Box : public Component
{
public:
	Box(bool useViewport)
	{
		if (useViewport)
		{
			mContent = new Component();
			mViewport = new Viewport();
			mViewport->setViewedComponent(mContent, false);
			mViewport->setScrollBarsShown(true, false);
			mViewport->setScrollBarThickness(5);
			addAndMakeVisible(mViewport);
		}
	}
	
	~Box()
	{
	
	}
	
	Component * getContent()
	{
		return mContent.get() ? mContent.get() : this;
	}
	
	void paint(Graphics &g)
	{
		const Rectangle<int> &box = getLocalBounds();

		g.setColour(Colour::fromRGB(200,200,200));
		g.fillRect(box);
		g.setColour(Colours::black);
		g.drawRect(box);
	}
	
	void resized()
	{
		if (mViewport)
			mViewport->setSize(getWidth(), getHeight());
	}
	
private:
	ScopedPointer<Component> mContent;
	ScopedPointer<Viewport> mViewport;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Box)
};


//================================================== PARAMSLIDER ======================================================
class ParamSlider : public Slider
{
public:
	ParamSlider(int paramIndex, int paramType, ToggleButton *link, OctogrisAudioProcessor *filter)
	:
		mParamIndex(paramIndex),
		mParamType(paramType),
		mLink(link),
		mFilter(filter)
	{
		jassert(mLink || mParamType != kParamSource);
	}
	
	void mouseDown (const MouseEvent &e)
	{
		bool resetToDefault = e.mods.isAltDown();
		if (resetToDefault)
		{
			double newVal;
			switch(mParamType)
			{
				case kParamSource: newVal = normalize(kSourceMinDistance, kSourceMaxDistance, kSourceDefaultDistance); break;
				case kParamSpeaker: newVal = normalize(kSpeakerMinAttenuation, kSpeakerMaxAttenuation, kSpeakerDefaultAttenuation); break;
				case kParamSmooth: newVal = normalize(kSmoothMin, kSmoothMax, kSmoothDefault); break;
				case kParamVolumeFar: newVal = normalize(kVolumeFarMin, kVolumeFarMax, kVolumeFarDefault); break;
				case kParamVolumeMid: newVal = normalize(kVolumeMidMin, kVolumeMidMax, kVolumeMidDefault); break;
				case kParamVolumeNear: newVal = normalize(kVolumeNearMin, kVolumeNearMax, kVolumeNearDefault); break;
				case kParamFilterFar: newVal = normalize(kFilterFarMin, kFilterFarMax, kFilterFarDefault); break;
				case kParamFilterMid: newVal = normalize(kFilterMidMin, kFilterMidMax, kFilterMidDefault); break;
				case kParamFilterNear: newVal = normalize(kFilterNearMin, kFilterNearMax, kFilterNearDefault); break;
			}
		
			if (mParamType == kParamSource && mLink->getToggleState())
			{
				for (int i = 0; i < mFilter->getNumberOfSources(); i++)
				{
					int paramIndex = mFilter->getParamForSourceD(i);
					if (mFilter->getParameter(paramIndex) != newVal)
						mFilter->setParameterNotifyingHost(paramIndex, newVal);
				}
			}
			else
			{
				if (mFilter->getParameter(mParamIndex) != newVal)
					mFilter->setParameterNotifyingHost(mParamIndex, newVal);
			}
		}
		else
		{
			Slider::mouseDown(e);
		}
	}
	
	void valueChanged()
	{
		if (mParamType == kParamSource)
		{
			const float newVal = 1.f - (float)getValue();
			
			if (mLink->getToggleState())
			{
				for (int i = 0; i < mFilter->getNumberOfSources(); i++)
				{
					int paramIndex = mFilter->getParamForSourceD(i);
					if (mFilter->getParameter(paramIndex) != newVal)
						mFilter->setParameterNotifyingHost(paramIndex, newVal);
				}
			}
			else
			{
				if (mFilter->getParameter(mParamIndex) != newVal)
					mFilter->setParameterNotifyingHost(mParamIndex, newVal);
			}
		}
		else
		{
			const float newVal = (float)getValue();
			if (mFilter->getParameter(mParamIndex) != newVal)
				mFilter->setParameterNotifyingHost(mParamIndex, newVal);
		}
	}
	
	String getTextFromValue (double value)
	{
		switch(mParamType)
		{
			case kParamSource: value = denormalize(kSourceMinDistance, kSourceMaxDistance, value); break;
			case kParamSpeaker: value = denormalize(kSpeakerMinAttenuation, kSpeakerMaxAttenuation, value); break;
			case kParamSmooth: value = denormalize(kSmoothMin, kSmoothMax, value); break;
			case kParamVolumeFar: value = denormalize(kVolumeFarMin, kVolumeFarMax, value); break;
			case kParamVolumeMid: value = denormalize(kVolumeMidMin, kVolumeMidMax, value); break;
			case kParamVolumeNear: value = denormalize(kVolumeNearMin, kVolumeNearMax, value); break;
			//case kParamFilterFar: value = denormalize(kFilterFarMin, kFilterFarMax, value); break;
			//case kParamFilterMid: value = denormalize(kFilterMidMin, kFilterMidMax, value); break;
			//case kParamFilterNear: value = denormalize(kFilterNearMin, kFilterNearMax, value); break;
			case kParamFilterFar: value = denormalize(-100, 0, value); break;
			case kParamFilterMid: value = denormalize(-100, 0, value); break;
			case kParamFilterNear: value = denormalize(-100, 0, value); break;
		}
		
		if (mParamType >= kParamSmooth || mParamType <= kParamFilterNear) return String(roundToInt(value));
		return String(value, 1);
	}
	
	double getValueFromText (const String& text)
	{
		double value = Slider::getValueFromText(text);
		switch(mParamType)
		{
			case kParamSource: value = normalize(kSourceMinDistance, kSourceMaxDistance, value); break;
			case kParamSpeaker: value = normalize(kSpeakerMinAttenuation, kSpeakerMaxAttenuation, value); break;
			case kParamSmooth: value = normalize(kSmoothMin, kSmoothMax, value); break;
			case kParamVolumeFar: value = normalize(kVolumeFarMin, kVolumeFarMax, value); break;
			case kParamVolumeMid: value = normalize(kVolumeMidMin, kVolumeMidMax, value); break;
			case kParamVolumeNear: value = normalize(kVolumeNearMin, kVolumeNearMax, value); break;
			//case kParamFilterFar: value = normalize(kFilterFarMin, kFilterFarMax, value); break;
			//case kParamFilterMid: value = normalize(kFilterMidMin, kFilterMidMax, value); break;
			//case kParamFilterNear: value = normalize(kFilterNearMin, kFilterNearMax, value); break;
			case kParamFilterFar: value = normalize(-100, 0, value); break;
			case kParamFilterMid: value = normalize(-100, 0, value); break;
			case kParamFilterNear: value = normalize(-100, 0, value); break;
		}
		return value;
	}

private:
	int mParamIndex, mParamType;
	ToggleButton *mLink;
	OctogrisAudioProcessor *mFilter;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParamSlider)
};

//==================================== EDITOR ===================================================================

OctogrisAudioProcessorEditor::OctogrisAudioProcessorEditor (OctogrisAudioProcessor* ownerFilter)
    :
		AudioProcessorEditor (ownerFilter),
		mFilter(ownerFilter),
		mMover(ownerFilter)
{
	mHostChangedParameter = mFilter->getHostChangedParameter();
	mHostChangedProperty = mFilter->getHostChangedProperty();
	
	mNeedRepaint = false;
	mFieldNeedRepaint = false;
	
	startTimer(kTimerDelay);
	mFilter->addListener(this);
    
	// main field
	{
//		FieldComponent *fc = new FieldComponent(mFilter, &mMover);
//		addAndMakeVisible(fc);
//		mComponents.add(fc);
//		mField = fc;
        mField = new FieldComponent(mFilter, &mMover);
		addAndMakeVisible(mField);
		mComponents.add(mField);

	}
    
    // param box
	Colour tabBg = Colour::fromRGB(200,200,200);
	mTabs = new OctTabbedComponent(TabbedButtonBar::TabsAtTop, mFilter);
	mTabs->addTab("Settings", tabBg, new Component(), true);
	mTabs->addTab("V & F", tabBg, new Component(), true);
	mTabs->addTab("Speakers", tabBg, new Component(), true);
	mTabs->addTab("Sources", tabBg, new Component(), true);
	mTabs->addTab("Trajectories", tabBg, new Component(), true);
	{
		mOsc = CreateOscComponent(mFilter, this);
		if (mOsc) mTabs->addTab("OSC", tabBg, mOsc, true);
	}
	bool leapSupported = false;
	{
		Component *leap = CreateLeapComponent(mFilter, this);
		if (leap)
		{
			mTabs->addTab("Leap", tabBg, leap, true);
			leapSupported = true;
		}
	}
	mTabs->setSize(kCenterColumnWidth + kMargin + kRightColumnWidth, kParamBoxHeight);
	addAndMakeVisible(mTabs);
	mComponents.add(mTabs);
	
	
	// sources
    {
        mSourcesBox = new Box(true);
        addAndMakeVisible(mSourcesBox);
        mComponents.add(mSourcesBox);
        
        mSourcesBoxLabel = addLabel("Sources distance:", 0, 0, kCenterColumnWidth, kDefaultLabelHeight, this);

        Component *ct = mSourcesBox->getContent();
        
        
        int dh = kDefaultLabelHeight;
        
        int x = 0, y = 0, w = kCenterColumnWidth;
        
        mLinkDistances = addCheckbox("Link", mFilter->getLinkDistances(), x, y, w/2, dh, ct);
        addLabel("Distance", x+w/2, y, w/2, dh, ct);
        

        
        updateSources();
    }
    
	// speakers
    {
        mSpeakersBox = new Box(true);
        addAndMakeVisible(mSpeakersBox);
        mComponents.add(mSpeakersBox);
        
        int dh = kDefaultLabelHeight;
        
        int x = 0, y = 0, w = kRightColumnWidth;
        
        mSpeakersBoxLabel = addLabel("Speakers attenuation:", 0, 0, kRightColumnWidth, kDefaultLabelHeight, this);

        Component *ct = mSpeakersBox->getContent();
        const int muteWidth = 50;
        addLabel("Mute", x, y, muteWidth, dh, ct);
        addLabel("Attenuation (db)", x+muteWidth, y, w*2/3 - muteWidth, dh, ct);
        addLabel("Level", x+w*2/3, y, w/3, dh, ct);

		mSpSelect = new ComboBox();
        mTabs->getTabContentComponent(2)->addAndMakeVisible(mSpSelect);
        mComponents.add(mSpSelect);
        updateSpeakers();
    }
    

	
    
    int dh = kDefaultLabelHeight;
    
    //--------------- SETTINGS TAB ---------------- //
	Component *box = mTabs->getTabContentComponent(0);
	{
		int x = kMargin, y = kMargin, w = (box->getWidth() - kMargin) / 3 - kMargin;
	
		//-----------------------------
		// start 1st column
		
		addLabel("Movements:", x, y, w, dh, box);
		y += dh + 5;
		
		{
			ComboBox *cb = new ComboBox();
			int index = 1;
			cb->addItem("Independent", index++);
			if (mFilter->getNumberOfSources() == 2)
			{
				cb->addItem("Symmetric X", index++);
				cb->addItem("Symmetric Y", index++);
				cb->addItem("Symmetric X & Y", index++);
			}
			if (mFilter->getNumberOfSources() >= 2)
			{
				cb->addItem("Circular", index++);
				cb->addItem("Circular Fixed Radius", index++);
				cb->addItem("Circular Fixed Angle", index++);
				cb->addItem("Circular Fully Fixed", index++);
				cb->addItem("Delta Lock", index++);
			}
			cb->setSelectedId(mFilter->getMovementMode() + 1);
			cb->setSize(w, dh);
			cb->setTopLeftPosition(x, y);
			box->addAndMakeVisible(cb);
			mComponents.add(cb);
			y += dh + 5;
			
			cb->addListener(this);
			mMovementMode = cb;
		}
		
		mLinkMovement = addCheckbox("Link movement", mFilter->getLinkMovement(), x, y, w, dh, box);
		y += dh + 5;
		
		{
			addLabel("Param smoothing (ms):", x, y, w, dh, box);
			y += dh + 5;
		
			Slider *ds = addParamSlider(kParamSmooth, kSmooth, mFilter->getParameter(kSmooth), x, y, w, dh, box);
			ds->setTextBoxStyle(Slider::TextBoxLeft, false, 40, dh);
			mSmoothing = ds;
			y += dh + 5;
		}
		
		//-----------------------------
		// start 2nd column
		y = kMargin;
		x += w + kMargin;
		
		addLabel("Gui size:", x, y, w, dh, box);
		y += dh + 5;
		
		{
			ComboBox *cb = new ComboBox();
			int index = 1;
			cb->addItem("Small", index++);
			cb->addItem("Medium", index++);
			cb->addItem("Large", index++);
			cb->setSelectedId(mFilter->getGuiSize() + 1);
			cb->setSize(w, dh);
			cb->setTopLeftPosition(x, y);
			box->addAndMakeVisible(cb);
			mComponents.add(cb);
			y += dh + 5;
			
			cb->addListener(this);
			mGuiSize = cb;
		}
		
		mShowGridLines = addCheckbox("Show grid lines", mFilter->getShowGridLines(), x, y, w, dh, box);
		y += dh + 20;
		
		//-----------------------------
		// start 3rd column
		y = kMargin;
		x += w + kMargin;
		
		addLabel("Process mode:", x, y, w, dh, box);
		y += dh + 5;
		
		{
			ComboBox *cb = new ComboBox();
			int index = 1;
			cb->addItem("Free volume", index++);
			cb->addItem("Pan volume", index++);
			cb->setSelectedId(mFilter->getProcessMode() + 1);
			cb->setSize(w, dh);
			cb->setTopLeftPosition(x, y);
			box->addAndMakeVisible(cb);
			mComponents.add(cb);
			y += dh + 5;
			
			cb->addListener(this);
			mProcessMode = cb;
		}
		
		addLabel(leapSupported ? "OSC/Leap source:" : "OSC source:", x, y, w, dh, box);
		y += dh + 5;
		
		{
			ComboBox *cb = new ComboBox();
			int index = 1;
			for (int i = 0; i < mFilter->getNumberOfSources(); i++)
			{
				String s; s << i+1;
				cb->addItem(s, index++);
			}
			cb->setSelectedId(mFilter->getOscLeapSource() + 1);
			cb->setSize(w, dh);
			cb->setTopLeftPosition(x, y);
			box->addAndMakeVisible(cb);
			mComponents.add(cb);
			y += dh + 5;
			
			cb->addListener(this);
			mOscLeapSourceCb = cb;
		}
	}
	
        //--------------- V & F TAB ---------------- //
	box = mTabs->getTabContentComponent(1);
	{
		int x = kMargin, y = kMargin, w = (box->getWidth() - kMargin) / 3 - kMargin;
	
		//-----------------------------
		// start 1st column
		
		{
			addLabel("Volume center (db):", x, y, w, dh, box);
			y += dh + 5;
		
			Slider *ds = addParamSlider(kParamVolumeNear, kVolumeNear, mFilter->getParameter(kVolumeNear), x, y, w, dh, box);
			ds->setTextBoxStyle(Slider::TextBoxLeft, false, 40, dh);
			mVolumeNear = ds;
			y += dh + 5;
		}
		
		{
			addLabel("Filter center:", x, y, w, dh, box);
			y += dh + 5;
		
			Slider *ds = addParamSlider(kParamFilterNear, kFilterNear, mFilter->getParameter(kFilterNear), x, y, w, dh, box);
			ds->setTextBoxStyle(Slider::TextBoxLeft, false, 40, dh);
			mFilterNear = ds;
			y += dh + 5;
		}
		
		mApplyFilter = addCheckbox("Apply Filter", mFilter->getApplyFilter(),
									x, y, w, dh, box);
		y += dh + 5;
		
		//-----------------------------
		// start 2nd column
		y = kMargin;
		x += w + kMargin;
		
		{
			addLabel("Volume speakers (db):", x, y, w, dh, box);
			y += dh + 5;
		
			Slider *ds = addParamSlider(kParamVolumeMid, kVolumeMid, mFilter->getParameter(kVolumeMid), x, y, w, dh, box);
			ds->setTextBoxStyle(Slider::TextBoxLeft, false, 40, dh);
			mVolumeMid = ds;
			y += dh + 5;
		}
		
		{
			addLabel("Filter speakers:", x, y, w, dh, box);
			y += dh + 5;
		
			Slider *ds = addParamSlider(kParamFilterMid, kFilterMid, mFilter->getParameter(kFilterMid), x, y, w, dh, box);
			ds->setTextBoxStyle(Slider::TextBoxLeft, false, 40, dh);
			mFilterMid = ds;
			y += dh + 5;
		}
		
		//-----------------------------
		// start 3rd column
		y = kMargin;
		x += w + kMargin;
		
		{
			addLabel("Volume far (db):", x, y, w, dh, box);
			y += dh + 5;
		
			Slider *ds = addParamSlider(kParamVolumeFar, kVolumeFar, mFilter->getParameter(kVolumeFar), x, y, w, dh, box);
			ds->setTextBoxStyle(Slider::TextBoxLeft, false, 40, dh);
			mVolumeFar = ds;
			y += dh + 5;
		}
		
		{
			addLabel("Filter far:", x, y, w, dh, box);
			y += dh + 5;
		
			Slider *ds = addParamSlider(kParamFilterFar, kFilterFar, mFilter->getParameter(kFilterFar), x, y, w, dh, box);
			ds->setTextBoxStyle(Slider::TextBoxLeft, false, 40, dh);
			mFilterFar = ds;
			y += dh + 5;
		}
	}
	
    //--------------- SPEAKERS TAB ---------------- //
	box = mTabs->getTabContentComponent(2);
	{
		int x = kMargin, y = kMargin, w = (box->getWidth() - kMargin) / 3 - kMargin;
		int setw = 60, selectw = 50;
	
		//-------- column 1 --------
		addLabel("Reset speakers:", x, y, w, dh, box);
		y += dh + 5;
		mSpAlternate = addCheckbox("Alternate", true, x, y, w, dh, box);
		y += dh + 5;
		mSpStartAtTop = addCheckbox("Start at top", false, x, y, w, dh, box);
		y += dh + 5;
		mSpClockwise = addCheckbox("Clockwise", false, x, y, w, dh, box);
		y += dh + 5;
		mSpApply = addButton("Apply", x, y, setw, dh, box);
		y += dh + 5;

		//-------- column 2 --------
		y = kMargin;
		x += w + kMargin;
		
		addLabel("Set XY position:", x, y, w - selectw, dh, box);
		{
			mSpSelect->setSize(selectw, dh);
			mSpSelect->setTopLeftPosition(x + w - selectw, y);
		}
		
		y += dh + 5;
		
		addLabel("(-2 to 2)", x, y, w, dh, box);
		y += dh + 5;
		int lw = 30, lwm = lw + kMargin;
		addLabel("X:", x, y, lw, dh, box);
		mSpX = addTextEditor("0", x + lwm, y, w - lwm, dh, box);
		y += dh + 5;
		addLabel("Y:", x, y, lw, dh, box);
		mSpY = addTextEditor("0", x + lwm, y, w - lwm, dh, box);
		y += dh + 5;
		mSpSetXY = addButton("Set", x, y, setw, dh, box);
				
		//-------- column 3 --------
		y = kMargin;
		x += w + kMargin;
		
		addLabel("Set RA position:", x, y, w, dh, box);
		y += dh + 5;
		addLabel("R: 0 to 2, A: 0 to 360", x, y, w, dh, box);
		y += dh + 5;
		addLabel("R:", x, y, lw, dh, box);
		mSpR = addTextEditor("1", x + lwm, y, w - lwm, dh, box);
		y += dh + 5;
		addLabel("A:", x, y, lw, dh, box);
		mSpT = addTextEditor("0", x + lwm, y, w - lwm, dh, box);
		y += dh + 5;
		mSpSetRT = addButton("Set", x, y, setw, dh, box);
	}
	
    //--------------- SOURCES TAB ---------------- //
	box = mTabs->getTabContentComponent(3);
	{
		int x = kMargin, y = kMargin, w = (box->getWidth() - kMargin) / 3 - kMargin;
		int setw = 60, selectw = 50;
	
		// column 1
		addLabel("Reset sources:", x, y, w, dh, box);
		y += dh + 5;
		
		mSrcAlternate = addCheckbox("Alternate", true, x, y, w, dh, box);
		y += dh + 5;
		
		mSrcStartAtTop = addCheckbox("Start at top", false, x, y, w, dh, box);
		y += dh + 5;
		
		mSrcClockwise = addCheckbox("Clockwise", false, x, y, w, dh, box);
		y += dh + 5;
		
		mSrcApply = addButton("Apply", x, y, setw, dh, box);
		y += dh + 5;
		
		// column 2
		y = kMargin;
		x += w + kMargin;
		
		addLabel("Set XY position:", x, y, w - selectw, dh, box);
		{
			ComboBox *cb = new ComboBox();
			int index = 1;
			for (int i = 0; i < mFilter->getNumberOfSources(); i++)
			{
				String s; s << i+1;
				cb->addItem(s, index++);
			}
			cb->setSelectedId(1);
			cb->setSize(selectw, dh);
			cb->setTopLeftPosition(x + w - selectw, y);
			box->addAndMakeVisible(cb);
			mComponents.add(cb);
			
			mSrcSelect = cb;
		}
		
		y += dh + 5;
		
		addLabel("(-2 to 2)", x, y, w, dh, box);
		y += dh + 5;
		
		int lw = 30, lwm = lw + kMargin;
		
		addLabel("X:", x, y, lw, dh, box);
		mSrcX = addTextEditor("0", x + lwm, y, w - lwm, dh, box);
		y += dh + 5;
		
		addLabel("Y:", x, y, lw, dh, box);
		mSrcY = addTextEditor("0", x + lwm, y, w - lwm, dh, box);
		y += dh + 5;
		
		mSrcSetXY = addButton("Set", x, y, setw, dh, box);
		
		// column 3
		y = kMargin;
		x += w + kMargin;
		
		addLabel("Set RA position:", x, y, w, dh, box);
		y += dh + 5;
		
		addLabel("R: 0 to 2, A: 0 to 360", x, y, w, dh, box);
		y += dh + 5;
		
		addLabel("R:", x, y, lw, dh, box);
		mSrcR = addTextEditor("1", x + lwm, y, w - lwm, dh, box);
		y += dh + 5;
		
		addLabel("A:", x, y, lw, dh, box);
		mSrcT = addTextEditor("0", x + lwm, y, w - lwm, dh, box);
		y += dh + 5;
		
		mSrcSetRT = addButton("Set", x, y, setw, dh, box);
	}
	
    //--------------- TRAJECTORIES TAB ---------------- //
	box = mTabs->getTabContentComponent(4);
	{
		int x = kMargin, y = kMargin, w = (box->getWidth() - kMargin) / 3 - kMargin;
		
		int cbw = 130;
		{
			ComboBox *cb = new ComboBox();
			int index = 1;
			for (int i = 0; i < Trajectory::NumberOfTrajectories(); i++)
				cb->addItem(Trajectory::GetTrajectoryName(i), index++);
			cb->setSelectedId(1);
			cb->setSize(cbw, dh);
			cb->setTopLeftPosition(x, y);
			box->addAndMakeVisible(cb);
			mComponents.add(cb);
			
			mTrType = cb;
		}
		
		{
			ComboBox *cb = new ComboBox();
			int index = 1;
			cb->addItem("All sources", index++);
			for (int i = 0; i < mFilter->getNumberOfSources(); i++)
			{
				String s("Source "); s << i+1;
				cb->addItem(s, index++);
			}
			cb->setSelectedId(1);
			cb->setSize(100, dh);
			cb->setTopLeftPosition(x + cbw + 5, y);
			box->addAndMakeVisible(cb);
			mComponents.add(cb);
			
			mTrSrcSelect = cb;
		}
		y += dh + 5;
		
		int tew = 80;
		mTrDuration = addTextEditor("1", x, y, tew, dh, box);
		x += tew + kMargin;
		{
			ComboBox *cb = new ComboBox();
			int index = 1;
			cb->addItem("Beat(s)", index++);
			cb->addItem("Second(s)", index++);
			cb->setSelectedId(1);
			cb->setSize(tew, dh);
			cb->setTopLeftPosition(x, y);
			box->addAndMakeVisible(cb);
			mComponents.add(cb);
			
			mTrUnits = cb;
		}
		x += tew + kMargin;
		
		addLabel("per cycle", x, y, w, dh, box);
		
		y += dh + 5;
		x = kMargin;
		
		mTrRepeats = addTextEditor("1", x, y, tew, dh, box);
		x += tew + kMargin;
		
		addLabel("cycle(s)", x, y, w, dh, box);
		
		y += dh + 5;
		x = kMargin;
		
		mTrWrite = addButton("Ready", x, y, cbw, dh, box);
		y += dh + 5;
		
		mTrProgressBar = new MiniProgressBar();
		mTrProgressBar->setSize(tew, dh);
		mTrProgressBar->setTopLeftPosition(x, y);
		mTrProgressBar->setVisible(false);
		box->addChildComponent(mTrProgressBar);
		mComponents.add(mTrProgressBar);
		
		mTrState = kTrReady;
	}
	
	int selectedTab = mFilter->getGuiTab();
	if (selectedTab >= 0 && selectedTab < mTabs->getNumTabs())
	{
		bool sendChangeMessage = false;
		mTabs->setCurrentTabIndex(selectedTab, sendChangeMessage);
	}
	
	mTabs->initDone();
	
	mFilter->setCalculateLevels(true);
	refreshSize();
}

OctogrisAudioProcessorEditor::~OctogrisAudioProcessorEditor()
{
	mFilter->setCalculateLevels(false);
	mFilter->removeListener(this);
}

//==============================================================================
void OctogrisAudioProcessorEditor::refreshSize()
{
	int fieldSize = 500;
	
	int guiSize = mFilter->getGuiSize();
	fieldSize += (guiSize - 1) * 100;
	
	setSize(kMargin + fieldSize + kMargin + kCenterColumnWidth + kMargin + kRightColumnWidth + kMargin,
			kMargin + fieldSize + kMargin);
}
void OctogrisAudioProcessorEditor::resized()
{
	int w = getWidth();
	int h = getHeight();

	int fieldWidth = w - (kMargin + kMargin + kCenterColumnWidth + kMargin + kRightColumnWidth + kMargin);
	int fieldHeight = h - (kMargin + kMargin);
	int fieldSize = jmin(fieldWidth, fieldHeight);

	mField->setBounds(kMargin, kMargin, fieldSize, fieldSize);
	
	int x = kMargin + fieldSize  + kMargin;
	int y = kMargin;
	mSourcesBoxLabel->setTopLeftPosition(x, y);
	
	int lh = mSourcesBoxLabel->getHeight() + 2;
	mSourcesBox->setBounds(x, y + lh, kCenterColumnWidth, h - (kMargin + kParamBoxHeight + kMargin + y + lh));
	
	mTabs->setBounds(x, h - (kParamBoxHeight + kMargin), kCenterColumnWidth + kMargin + kRightColumnWidth, kParamBoxHeight);
	
	x += kCenterColumnWidth + kMargin;
	mSpeakersBoxLabel->setTopLeftPosition(x, y);
	mSpeakersBox->setBounds(x, y + lh, kRightColumnWidth, h - (kMargin + kParamBoxHeight + kMargin + y + lh));
}


void OctogrisAudioProcessorEditor::updateSources(){
    
  	int dh = kDefaultLabelHeight;

    int x = 0, y = 0, w = kCenterColumnWidth;
	
    Component *ct = mSourcesBox->getContent();

    y += dh + 5;
    
    
    //remove old stuff
    for (int iCurLevelComponent = 0; iCurLevelComponent < mDistances.size(); ++iCurLevelComponent){
        ct->removeChildComponent(mDistances.getUnchecked(iCurLevelComponent));
        ct->removeChildComponent(mLabels.getUnchecked(iCurLevelComponent));
    }
    
    mDistances.clear();
    mLabels.clear();
    
    for (int i = 0; i < mFilter->getNumberOfSources(); i++)
    {
        String s; s << i+1; s << ":";
        Component *label = addLabel(s, x, y, w/3, dh, ct);
        mLabels.add(label);
        
        Slider *slider = addParamSlider(kParamSource, i, mFilter->getSourceD(i), x + w/3, y, w*2/3, dh, ct);
        mDistances.add(slider);
        
        y += dh + 5;
    }
    
    ct->setSize(w, y);
    
}

void OctogrisAudioProcessorEditor::updateSpeakers(){

    //remove old stuff
    Component *ct = mSpeakersBox->getContent();
    for (int iCurLevelComponent = 0; iCurLevelComponent < mLevels.size(); ++iCurLevelComponent){
        ct->removeChildComponent(mMutes.getUnchecked(iCurLevelComponent));
        ct->removeChildComponent(mAttenuations.getUnchecked(iCurLevelComponent));
        ct->removeChildComponent(mLevels.getUnchecked(iCurLevelComponent));

        mComponents.removeObject(mLevels.getUnchecked(iCurLevelComponent));
    }
    mMutes.clear();
    mAttenuations.clear();
    mLevels.clear();
    mSpSelect->clear();
    

    
    //put new stuff
    int iCurSpeakers = mFilter->getNumberOfSpeakers();
   	int dh = kDefaultLabelHeight, x = 0, y = 0, w = kRightColumnWidth;
    
    const int muteWidth = 50;
    y += dh + 5;

    for (int i = 0; i < iCurSpeakers; i++)
    {
        String s; s << i+1; s << ":";
        
        ToggleButton *mute = addCheckbox(s, mFilter->getSpeakerM(i), x, y, muteWidth, dh, ct);
        mMutes.add(mute);
        
        Slider *slider = addParamSlider(kParamSpeaker, i, mFilter->getSpeakerA(i), x+muteWidth, y, w*2/3 - muteWidth, dh, ct);
        slider->setTextBoxStyle(Slider::TextBoxLeft, false, 40, dh);
        mAttenuations.add(slider);
        
        Rectangle<int> level(x+w*2/3, y + 3, w/3 - 10, dh - 6);
        
        LevelComponent *lc = new LevelComponent(mFilter, i);
        lc->setBounds(level);
        ct->addAndMakeVisible(lc);
        mComponents.add(lc);
        mLevels.add(lc);
        
        y += dh + 5;
    }
    
    ct->setSize(w, y);
    
    
    //speaker position combo box in speakers tab
    int index = 1;
    for (int i = 0; i < iCurSpeakers; i++)
    {
        String s; s << i+1;
        mSpSelect->addItem(s, index++);
    }
    mSpSelect->setSelectedId(1);
}


void OctogrisAudioProcessorEditor::setOscLeapSource(int s)
{
	if (s < 0) s = 0;
	if (s >= mFilter->getNumberOfSources()) s = mFilter->getNumberOfSources() - 1;
	mFilter->setOscLeapSource(s);
	
	const MessageManagerLock mmLock;
	mOscLeapSourceCb->setSelectedId(s + 1);
}

//==============================================================================
Component* OctogrisAudioProcessorEditor::addLabel(const String &s, int x, int y, int w, int h, Component *into)
{
	Label *label = new Label();
	label->setText(s, dontSendNotification);
	label->setSize(w, h);
	label->setJustificationType(Justification::left);
	label->setMinimumHorizontalScale(1);
	label->setTopLeftPosition(x, y);
	into->addAndMakeVisible(label);
	mComponents.add(label);
	return label;
}

ToggleButton* OctogrisAudioProcessorEditor::addCheckbox(const String &s, bool v, int x, int y, int w, int h, Component *into)
{
	ToggleButton *tb = new ToggleButton();
	tb->setButtonText(s);
	tb->setSize(w, h);
	tb->setTopLeftPosition(x, y);
	tb->addListener(this);
	tb->setToggleState(v, dontSendNotification);
	into->addAndMakeVisible(tb);
	mComponents.add(tb);
	return tb;
}

TextButton* OctogrisAudioProcessorEditor::addButton(const String &s, int x, int y, int w, int h, Component *into)
{
	TextButton *tb = new TextButton();
	tb->setButtonText(s);
	tb->setSize(w, h);
	tb->setTopLeftPosition(x, y);
	tb->addListener(this);
	into->addAndMakeVisible(tb);
	mComponents.add(tb);
	return tb;
}

TextEditor* OctogrisAudioProcessorEditor::addTextEditor(const String &s, int x, int y, int w, int h, Component *into)
{
	TextEditor *te = new TextEditor();
	te->setText(s);
	te->setSize(w, h);
	te->setTopLeftPosition(x, y);
	into->addAndMakeVisible(te);
	mComponents.add(te);
	return te;
}

Slider* OctogrisAudioProcessorEditor::addParamSlider(int paramType, int si, float v, int x, int y, int w, int h, Component *into)
{
	int index ;
	if (paramType == kParamSource) index = mFilter->getParamForSourceD(si);
	else if (paramType == kParamSpeaker) index = mFilter->getParamForSpeakerA(si);
	else index = si;
	
	if (paramType == kParamSource)
		v = 1.f - v;
	
	ParamSlider *ds = new ParamSlider(index, paramType, (paramType == kParamSource) ? mLinkDistances : NULL, mFilter);
	ds->setRange(0, 1);
	ds->setValue(v);
	ds->setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
	ds->setSize(w, h);
	ds->setTopLeftPosition(x, y);
	into->addAndMakeVisible(ds);
	mComponents.add(ds);
	return ds;
}


//==============================================================================
void OctogrisAudioProcessorEditor::buttonClicked (Button *button)
{
	for (int i = 0; i < mFilter->getNumberOfSpeakers(); i++)
	{
		if (button == mMutes[i])
		{
			float v = button->getToggleState() ? 1.f : 0.f;
			mFilter->setParameterNotifyingHost(mFilter->getParamForSpeakerM(i), v);
			mField->repaint();
			return;
		}
	}

	if (button == mShowGridLines)
	{
		mFilter->setShowGridLines(button->getToggleState());
		mField->repaint();
	}
	else if (button == mLinkDistances)
	{
		mFilter->setLinkDistances(button->getToggleState());
	}
	else if (button == mLinkMovement)
	{
		mFilter->setLinkMovement(button->getToggleState());
	}
	else if (button == mApplyFilter)
	{
		mFilter->setApplyFilter(button->getToggleState());
	}
	else if (button == mSpSetXY)
	{
		int sp = mSpSelect->getSelectedId() - 1;
		float x = mSpX->getText().getFloatValue();
		float y = mSpY->getText().getFloatValue();
		//mFilter->setSpeakerXY01(sp, FPoint((x+kRadiusMax)/(kRadiusMax*2), (y+kRadiusMax)/(kRadiusMax*2)));
        
        
        mFilter->setNumberOfSources(x);
        mFilter->setNumberOfSpeakers(y);
        mField->repaint();
        updateSources();
        updateSpeakers();
        
	}
	else if (button == mSpSetRT)
	{
		int sp = mSpSelect->getSelectedId() - 1;
		float r = mSpR->getText().getFloatValue();
		float t = mSpT->getText().getFloatValue();
		if (r < 0) r = 0; else if (r > kRadiusMax) r = kRadiusMax;
		mFilter->setSpeakerRT(sp, FPoint(r, t * M_PI / 180.));
	}
	else if (button == mSpApply)
	{
		bool alternate = mSpAlternate->getToggleState();
		bool startAtTop = mSpStartAtTop->getToggleState();
		bool clockwise = mSpClockwise->getToggleState();
		
		float anglePerSp = kThetaMax / mFilter->getNumberOfSpeakers();
		
		if (alternate)
		{
			float offset = startAtTop
							? (clockwise ? kQuarterCircle : (kQuarterCircle - anglePerSp))
							: (kQuarterCircle - anglePerSp/2);
			float start = offset;
			for (int i = clockwise ? 0 : 1; i < mFilter->getNumberOfSpeakers(); i += 2)
			{
				mFilter->setSpeakerRT(i, FPoint(1, offset));
				offset -= anglePerSp;
			}
			
			offset = start + anglePerSp;
			for (int i = clockwise ? 1 : 0; i < mFilter->getNumberOfSpeakers(); i += 2)
			{
				mFilter->setSpeakerRT(i, FPoint(1, offset));
				offset += anglePerSp;
			}
		}
		else
		{
			float offset = startAtTop
							? kQuarterCircle
							: (clockwise ? (kQuarterCircle - anglePerSp/2) : (kQuarterCircle + anglePerSp/2));
			float delta = clockwise ? -anglePerSp : anglePerSp;
			for (int i = 0; i < mFilter->getNumberOfSpeakers(); i++)
			{
				mFilter->setSpeakerRT(i, FPoint(1, offset));
				offset += delta;
			}
		}
	}
	else if (button == mSrcSetXY)
	{
		int sp = mSrcSelect->getSelectedId() - 1;
		float x = mSrcX->getText().getFloatValue();
		float y = mSrcY->getText().getFloatValue();
		mFilter->setSourceXY01(sp, FPoint((x+kRadiusMax)/(kRadiusMax*2), (y+kRadiusMax)/(kRadiusMax*2)));
	}
	else if (button == mSrcSetRT)
	{
		int sp = mSrcSelect->getSelectedId() - 1;
		float r = mSrcR->getText().getFloatValue();
		float t = mSrcT->getText().getFloatValue();
		if (r < 0) r = 0; else if (r > kRadiusMax) r = kRadiusMax;
		mFilter->setSourceRT(sp, FPoint(r, t * M_PI / 180.));
	}
	else if (button == mSrcApply)
	{
		bool alternate = mSrcAlternate->getToggleState();
		bool startAtTop = mSrcStartAtTop->getToggleState();
		bool clockwise = mSrcClockwise->getToggleState();
		
		float anglePerSp = kThetaMax / mFilter->getNumberOfSources();
		
		if (alternate)
		{
			float offset = startAtTop
							? (clockwise ? kQuarterCircle : (kQuarterCircle - anglePerSp))
							: (kQuarterCircle - anglePerSp/2);
			float start = offset;
			for (int i = clockwise ? 0 : 1; i < mFilter->getNumberOfSources(); i += 2)
			{
				mFilter->setSourceRT(i, FPoint(1, offset));
				offset -= anglePerSp;
			}
			
			offset = start + anglePerSp;
			for (int i = clockwise ? 1 : 0; i < mFilter->getNumberOfSources(); i += 2)
			{
				mFilter->setSourceRT(i, FPoint(1, offset));
				offset += anglePerSp;
			}
		}
		else
		{
			float offset = startAtTop
							? kQuarterCircle
							: (clockwise ? (kQuarterCircle - anglePerSp/2) : (kQuarterCircle + anglePerSp/2));
			float delta = clockwise ? -anglePerSp : anglePerSp;
			for (int i = 0; i < mFilter->getNumberOfSources(); i++)
			{
				mFilter->setSourceRT(i, FPoint(1, offset));
				offset += delta;
			}
		}
	}
	else if (button == mTrWrite)
	{
		Trajectory::Ptr t = mFilter->getTrajectory();
		if (t)
		{
			mFilter->setTrajectory(NULL);
			mTrWrite->setButtonText("Ready");
			mTrProgressBar->setVisible(false);
			mTrState = kTrReady;
			t->stop();
		}
		else
		{
			float duration = mTrDuration->getText().getFloatValue();
			bool beats = mTrUnits->getSelectedId() == 1;
			float repeats = mTrRepeats->getText().getFloatValue();
			int type = mTrType->getSelectedId() - 1;
			int source = mTrSrcSelect->getSelectedId() - 2;
			
			mFilter->setTrajectory(Trajectory::CreateTrajectory(type, mFilter, duration, beats, repeats, source));
			mTrWrite->setButtonText("Cancel");
			mTrState = kTrWriting;
			
			mTrProgressBar->setValue(0);
			mTrProgressBar->setVisible(true);
		}
	}
	else
	{
		printf("unknown button clicked...\n");
	}
}

void OctogrisAudioProcessorEditor::comboBoxChanged (ComboBox* comboBox)
{
	if (comboBox == mMovementMode)
	{
		mFilter->setMovementMode(comboBox->getSelectedId() - 1);
	}
	else if (comboBox == mGuiSize)
	{
		mFilter->setGuiSize(comboBox->getSelectedId() - 1);
		refreshSize();
	}
	else if (comboBox == mProcessMode)
	{
		mFilter->setProcessMode(comboBox->getSelectedId() - 1);
		repaint();
	}
	else if (comboBox == mOscLeapSourceCb)
	{
		mFilter->setOscLeapSource(comboBox->getSelectedId() - 1);
	}
	else
	{
		printf("unknown combobox clicked...\n");
	}
}


//==============================================================================
void OctogrisAudioProcessorEditor::timerCallback()
{
    
    
	switch(mTrState)
	{
		case kTrWriting:
		{
			Trajectory::Ptr t = mFilter->getTrajectory();
			if (t)
			{
				mTrProgressBar->setValue(t->progress());
			}
			else
			{
				mTrWrite->setButtonText("Ready");
				mTrProgressBar->setVisible(false);
				mTrState = kTrReady;
			}
		}
		break;
	}


		
	uint64_t hcp = mFilter->getHostChangedProperty();
	if (hcp != mHostChangedProperty)
	{
		mHostChangedProperty = hcp;
		
		mMovementMode->setSelectedId(mFilter->getMovementMode() + 1);
		mProcessMode->setSelectedId(mFilter->getProcessMode() + 1);
		mGuiSize->setSelectedId(mFilter->getGuiSize() + 1);
		
		mLinkMovement->setToggleState(mFilter->getLinkMovement(), dontSendNotification);
		mShowGridLines->setToggleState(mFilter->getShowGridLines(), dontSendNotification);
		mLinkDistances->setToggleState(mFilter->getLinkDistances(), dontSendNotification);
		mApplyFilter->setToggleState(mFilter->getApplyFilter(), dontSendNotification);
		
		refreshSize();
	}

	hcp = mFilter->getHostChangedParameter();
	if (hcp != mHostChangedParameter)
	{
		mHostChangedParameter = hcp;
		mNeedRepaint = true;
	}
	
	if (mFieldNeedRepaint || mNeedRepaint){
		mField->repaint();
    }
    
    for (int i = 0; i < mFilter->getNumberOfSpeakers(); i++)
		mLevels.getUnchecked(i)->refreshIfNeeded();

	if (mNeedRepaint)
	{
		mSmoothing->setValue(mFilter->getParameter(kSmooth));
		mVolumeFar->setValue(mFilter->getParameter(kVolumeFar));
		mVolumeMid->setValue(mFilter->getParameter(kVolumeMid));
		mVolumeNear->setValue(mFilter->getParameter(kVolumeNear));
		
		for (int i = 0; i < mFilter->getNumberOfSources(); i++)
		{
			mDistances.getUnchecked(i)->setValue(1.f - mFilter->getSourceD(i), dontSendNotification);
		
			mMutes.getUnchecked(i)->setToggleState(mFilter->getSpeakerM(i), dontSendNotification);
		}
		
		for (int i = 0; i < mFilter->getNumberOfSpeakers(); i++)
			mAttenuations.getUnchecked(i)->setValue(mFilter->getSpeakerA(i), dontSendNotification);
	}
	
	mNeedRepaint = false;
	mFieldNeedRepaint = false;
	
	if (mOsc) mOsc->heartbeat();
	
	startTimer(kTimerDelay);
}

void OctogrisAudioProcessorEditor::audioProcessorChanged (AudioProcessor* processor)
{
	mNeedRepaint = true;
}

void OctogrisAudioProcessorEditor::audioProcessorParameterChanged(AudioProcessor* processor,
																 int parameterIndex,
																 float newValue)
{
	mNeedRepaint = true;
}

//==============================================================================
void OctogrisAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colours::white);
}

