#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "ErHuString.h"
class ErHu : public AudioAppComponent
{
public:
	ErHu()
#ifdef JUCE_DEMO_RUNNER
		: AudioAppComponent(getSharedAudioDeviceManager(0, 2))
#endif
	{
		createStringComponents();
		setSize(100, 100);

		// specify the number of input and output channels that we want to open
		auto audioDevice = deviceManager.getCurrentAudioDevice();
		auto numInputChannels = (audioDevice != nullptr ? audioDevice->getActiveInputChannels().countNumberOfSetBits() : 0);
		auto numOutputChannels = jmax(audioDevice != nullptr ? audioDevice->getActiveOutputChannels().countNumberOfSetBits() : 2, 2);

		setAudioChannels(numInputChannels, numOutputChannels);
	}

	~ErHu()
	{
		shutdownAudio();
	}

	//==============================================================================
	void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
	{
		transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
	}

	void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override
	{
		if (readerSource == nullptr)
		{
			bufferToFill.clearActiveBufferRegion();
			return;
		}

		transportSource.getNextAudioBlock(bufferToFill);
	}
	void releaseResources() override
	{
		transportSource.releaseResources();
	}


	//==============================================================================
	void paint(Graphics&) override {}

	void resized() override
	{
		auto xPos = 20;
		auto yPos = 20;
		auto yDistance = 50;

		for (auto stringLine : stringLines)
		{
			stringLine->setTopLeftPosition(xPos, yPos);
			yPos += yDistance;
			addAndMakeVisible(stringLine);
		}
	}

	ScopedPointer<AudioFormatReaderSource> readerSource;
	AudioTransportSource transportSource;
private:
	void mouseDown(const MouseEvent& e) override
	{
		mouseDrag(e);
	}

	void mouseDrag(const MouseEvent& e) override
	{
		for (auto i = 0; i < stringLines.size(); ++i)
		{
			auto* stringLine = stringLines.getUnchecked(i);

			if (stringLine->getBounds().contains(e.getPosition()))
			{
				auto position = (e.position.x - stringLine->getX()) / stringLine->getWidth();

				stringLine->stringPlucked(position);
				//stringSynths.getUnchecked (i)->stringPlucked (position);
				playSample(stringLine->getMidi());
			}
		}
	}



	void playSample(int midi_value)
	{
		String filePath = "C:\\Users\\GuwalgiyaGuan\\Desktop\\ErHu_Data\\Bow_D5.wav";
		if (midi_value == 69)
		{
			filePath = "C:\\Users\\GuwalgiyaGuan\\Desktop\\ErHu_Data\\Bow_A5.wav";
		}

		File file(filePath);
		AudioFormatManager formatManager;
		formatManager.registerBasicFormats();
		AudioFormatReader* reader = formatManager.createReaderFor(filePath);
		ScopedPointer<AudioFormatReaderSource> newSource = new AudioFormatReaderSource(reader, true);
		transportSource.setSource(newSource, 0, nullptr, reader->sampleRate);
		readerSource = newSource.release();
		transportSource.start();
	}



	//==============================================================================
	struct StringParameters
	{
		StringParameters(int midiNote)
			: midiValue(midiNote),
			lengthInPixels(800)
		{}

		double midiValue;
		int lengthInPixels;
	};

	static Array<StringParameters> getDefaultStringParameters()
	{
		return Array<StringParameters>(62, 69);
	}

	void createStringComponents()
	{

		for (auto stringParams : getDefaultStringParameters())
		{
			stringLines.add(new ErHuString(stringParams.lengthInPixels,
				Colour::fromHSV(Random().nextFloat(), 0.6f, 0.9f, 1.0f),
				stringParams.midiValue));
		}
	}


	//==============================================================================
	OwnedArray<ErHuString> stringLines;
	//OwnedArray<StringSynthesiser> stringSynths;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ErHu)
};