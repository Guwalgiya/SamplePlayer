//==============================================================================
/*
*/
#include "../JuceLibraryCode/JuceHeader.h"

class ErHuString : public Component,
	private Timer
{
public:
	ErHuString(int lengthInPixels, Colour stringColour, int midi_value)
		: length(lengthInPixels), colour(stringColour), midi(midi_value)
	{
		// ignore mouse-clicks so that our parent can get them instead.
		setInterceptsMouseClicks(false, false);
		setSize(length, height);
		startTimerHz(60);
	}

	int getMidi()
	{
		return midi;
	}
	//==============================================================================
	void stringPlucked(float pluckPositionRelative)
	{
		amplitude = maxAmplitude * std::sin(pluckPositionRelative * MathConstants<float>::pi);
		phase = MathConstants<float>::pi;
	}

	//==============================================================================
	void paint(Graphics& g) override
	{
		g.setColour(colour);
		g.strokePath(generateStringPath(), PathStrokeType(2.0f));
	}

	Path generateStringPath() const
	{
		auto y = height / 2.0f;

		Path stringPath;
		stringPath.startNewSubPath(0, y);
		stringPath.quadraticTo(length / 2.0f, y + (std::sin(phase) * amplitude), (float)length, y);
		return stringPath;
	}

	//==============================================================================
	void timerCallback() override
	{
		updateAmplitude();
		updatePhase();
		repaint();
	}

	void updateAmplitude()
	{
		amplitude *= 0.99f;
	}

	void updatePhase()
	{
		auto phaseStep = 400.0f / length;

		phase += phaseStep;

		if (phase >= MathConstants<float>::twoPi)
			phase -= MathConstants<float>::twoPi;
	}

private:
	//==============================================================================
	int         length;
	int         midi;
	int         height       = 20;
	float       amplitude    = 0.0f;
	const float maxAmplitude = 30.0f;
	float       phase        = 0.0f;
	Colour      colour;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ErHuString)
};
