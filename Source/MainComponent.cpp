#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "ErHuString.h"
#include "FlangerEditor.h"
#include <math.h>
#include <experimental/filesystem>

class MainContentComponent : public  AudioAppComponent,
	public  ChangeListener,
	public  ToggleButton::Listener,
	public  AudioTransportSource,
	private ComboBox::Listener,
	private MidiInputCallback,
	private MidiKeyboardStateListener
{
public:
	enum RadioButtonIds
	{
		XOR_realtion = 1001
	};

	int canvas_width = 800;
	int canvas_length = 800;
	int A4_midi = 81;
	int D4_midi = 74;
	int midi_chann = 1;
	int num_in_chann = 0;
	int num_out_chann = 2;
	int cent_c_octave = 3;
	float defualt_fs = 48000.00;


	MainContentComponent() : lastInputIndex(0),
		state(Stopped),
		isAddingFromMidiInput(false),
		keyboardComponent(keyboardState, MidiKeyboardComponent::horizontalKeyboard),
		A_string(canvas_width, Colours::red, A4_midi),
		D_string(canvas_width, Colours::blue, D4_midi),
		processor()
	{
		//=========================================================================================================================
		//set up the GUI's basic components
		setOpaque(true);

		addAndMakeVisible(bow_button);
		bow_button.setButtonText("Bowing");
		bow_button.setRadioGroupId(XOR_realtion);

		addAndMakeVisible(pluck_button);
		pluck_button.setButtonText("Plucking");
		pluck_button.setRadioGroupId(XOR_realtion);

		addAndMakeVisible(midiInputList_Label);
		midiInputList_Label.setText("MIDI Input: ", dontSendNotification);
		midiInputList_Label.attachToComponent(&midiInputList, true);

		addAndMakeVisible(midiInputList);
		midiInputList.setTextWhenNoChoicesAvailable("No MIDI Inputs Enabled");


		//=========================================================================================================================
		//set up Piano
		const StringArray midiInputs(MidiInput::getDevices());
		midiInputList.addItemList(midiInputs, 1);
		midiInputList.addListener(this);

		for (int i = 0; i < midiInputs.size(); i++)
		{
			if (deviceManager.isMidiInputEnabled(midiInputs[i]))
			{
				setMidiInput(i);
				break;
			}
		}

		if (midiInputList.getSelectedId() == 0)
		{
			setMidiInput(0);
		}

		addAndMakeVisible(keyboardComponent);
		keyboardState.addListener(this);

		//set up ErHu
		addAndMakeVisible(A_string);
		addAndMakeVisible(D_string);
		addAndMakeVisible(scaleLabel);
		addAndMakeVisible(scaleSlider);
		addAndMakeVisible(scaleLabel_name);
		scaleLabel_name.setText("Bowing Velocity: ", dontSendNotification);
		addAndMakeVisible(distortionLabel);
		addAndMakeVisible(distortionSlider);
		addAndMakeVisible(distortionLabel_name);
		distortionLabel_name.setText("Distortion", dontSendNotification);
		midiInputList_Label.attachToComponent(&scaleLabel, true);

		//message box at the bottom
		addAndMakeVisible(midiMessagesBox);
		midiMessagesBox.setMultiLine(true);
		midiMessagesBox.setReturnKeyStartsNewLine(true);
		midiMessagesBox.setReadOnly(true);
		midiMessagesBox.setScrollbarsShown(true);
		midiMessagesBox.setCaretVisible(false);
		midiMessagesBox.setPopupMenuEnabled(true);
		midiMessagesBox.setColour(TextEditor::backgroundColourId, Colours::black);
		midiMessagesBox.setColour(TextEditor::outlineColourId, Colours::white);

		//=========================================================================================================================
		//Flanger Setup
		addAndMakeVisible(flanger);
		flanger.setButtonText("Flanging");
		flanger.addListener(this);


		//=========================================================================================================================
		//Audio setup
		formatManager.registerBasicFormats();
		AudioDeviceManager::AudioDeviceSetup deviceSetup = AudioDeviceManager::AudioDeviceSetup();
		deviceSetup.sampleRate = defualt_fs;
		transportSource.addChangeListener(this);
		setAudioChannels(num_in_chann, num_out_chann);
		setSize(canvas_width, canvas_length);
	}

	~MainContentComponent()
	{
		keyboardState.removeListener(this);
		deviceManager.removeMidiInputCallback(MidiInput::getDevices()[midiInputList.getSelectedItemIndex()], this);
		midiInputList.removeListener(this);
		readerSource = nullptr;
		delete readerSource;
		shutdownAudio();
	}

	//core function
	void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
	{
		transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
		processor.prepareToPlay(sampleRate, samplesPerBlockExpected);
	}

	//core function
	void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override
	{
		if (readerSource == nullptr)
		{
			bufferToFill.clearActiveBufferRegion();
			return;
		}
		
		transportSource.getNextAudioBlock(bufferToFill);
		
		//Carry out distortion on channel 0
		if (distortionSlider.getValue()>0) {

			auto* inBuffer = bufferToFill.buffer->getReadPointer(0,bufferToFill.startSample);
			float* outBuffer = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
			

			for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
				outBuffer[sample] = inBuffer[sample]+  inBuffer[sample]*random.nextFloat() * distortionSlider.getValue();
		}
		auto flangerState = flanger.getState();
		String stateString = flangerState ? "ON" : "OFF";
		if (stateString == "ON") {
			 MidiBuffer *m = new MidiBuffer();
			 processor.processBlock(*bufferToFill.buffer,*m);
		}
		
	}

	//core function
	void releaseResources() override
	{
		transportSource.releaseResources();
	}

	//core function
	void changeListenerCallback(ChangeBroadcaster* source) override
	{
		if (source == &transportSource)
		{
			if (transportSource.isPlaying())
				changeState(Playing);
			else
				changeState(Stopped);
		}
	}

	//core function
	void paint(Graphics& g) override
	{
		g.fillAll(Colours::black);
	}

	//core function
	void resized() override
	{
		bow_button.setBounds(100, 250, getWidth() - 20, 40);
		pluck_button.setBounds(180, 250, getWidth() - 20, 40);
		flanger.setBounds(220, 600, getWidth() - 300, 30);
		A_string.setBounds(0, 10, getWidth(), 30);
		D_string.setBounds(0, 50, getWidth(), 30);
		keyboardComponent.setBounds(0, 150, getWidth() - 10, 70);
		scaleLabel.setBounds(0, 90, getWidth() - 10, 40);
		scaleLabel_name.setBounds(0, 60, getWidth() - 10, 40);
		scaleSlider.setBounds(30, 90, getWidth() - 10, 40);
		distortionLabel.setBounds(0, 550, getWidth() - 10, 40);
		distortionLabel_name.setBounds(0, 520, getWidth() - 10, 40);
		distortionSlider.setBounds(30, 550, getWidth() - 10, 40);
		midiInputList.setBounds(150, 300, getWidth() - 300, 30);
		midiMessagesBox.setBounds(150, 350, getWidth() - 200, 200);
	}

	//GUI function
	void buttonClicked(Button *button) override
	{
		if (button == &flanger) {
			auto flangerState = flanger.getState();
			String stateString = flangerState ? "ON" : "OFF";
			if (stateString == "ON") {

				//myFlangerWindow = new ResizableWindow("Flanger Effect", true);
				flangerAudioProcessorEditor->addToDesktop(ComponentPeer::windowIsTemporary);
				window.add(flangerAudioProcessorEditor);
				flangerAudioProcessorEditor->setBounds(0, 0, 300, 400);

				//addAndMakeVisible(flangerAudioProcessorEditor);
				Rectangle<int> area(0, 0, 300, 400);

				RectanglePlacement placement(RectanglePlacement::xLeft
					| RectanglePlacement::yBottom
					| RectanglePlacement::doNotResize);

				auto result = placement.appliedTo(area, Desktop::getInstance().getDisplays()
					.getMainDisplay().userArea.reduced(20));
				flangerAudioProcessorEditor->setBounds(result);

				flangerAudioProcessorEditor->setVisible(true);
				flangerAudioProcessorEditor->setResizable(true, true);
			}
		}
	}

private:
	//=========================================================================================================================
	//Function Needed for both keyboard and Erhu
	void logMessage(const String& m)
	{
		midiMessagesBox.moveCaretToEnd();
		midiMessagesBox.insertTextAtCaret(m + newLine);
	}

	// These methods handle callbacks from the midi device + on-screen keyboard..
	void handleIncomingMidiMessage(MidiInput* source, const MidiMessage& message) override
	{
		const ScopedValueSetter<bool> scopedInputFlag(isAddingFromMidiInput, true);
		keyboardState.processNextMidiEvent(message);
	}

	String createInstrumentFileName(String technique, int midiNoteNumber)
	{
		String midiName = MidiMessage::getMidiNoteName(midiNoteNumber, true, true, cent_c_octave);
		return technique + "_" + midiName + ".wav";
	}

	String getMidiname(int midiNoteNumber)
	{
		return MidiMessage::getMidiNoteName(midiNoteNumber, true, true, cent_c_octave);
	}

	void playSound(String stateString_bow, String stateString_pluck, int midiNoteNumber, float velocity, String instrument)
	{
		if (stateString_bow == "ON")
		{
			//create path
			String cwd = File::getCurrentWorkingDirectory().getParentDirectory().getParentDirectory().getFullPathName();
			String fileName = createInstrumentFileName("Bow", midiNoteNumber);
			String path = cwd + "/Resources/" + fileName;

			//Prase File
			AudioFormatReader* reader = formatManager.createReaderFor(path);
			File file(path);
			if (reader != nullptr)
			{
				if (instrument == "String")
				{
					//depends on the playing velocity
					float endPoint = std::floor(reader->lengthInSamples * (scaleSlider.getMaximum() - velocity) / scaleSlider.getMaximum());
					int   play_length = (int)endPoint;
					AudioSubsectionReader* subsectionReader = new AudioSubsectionReader(reader, 0, endPoint, true);
					ScopedPointer<AudioFormatReaderSource> newSource = new AudioFormatReaderSource(subsectionReader, true);
					transportSource.setSource(newSource, 0, nullptr, reader->sampleRate);
					readerSource = newSource.release();
				}
				else
				{
					ScopedPointer<AudioFormatReaderSource> newSource = new AudioFormatReaderSource(reader, true);
					transportSource.setSource(newSource, 0, nullptr, reader->sampleRate);
					readerSource = newSource.release();
				}

				//play	
				transportSource.start();
			}
			else
			{
				logMessage("Impossible to play this note on an ErHu");
			}
		}
		else if (stateString_pluck == "ON" && !isAddingFromMidiInput)
		{
			//create path
			String cwd = File::getCurrentWorkingDirectory().getParentDirectory().getParentDirectory().getFullPathName();
			String fileName = createInstrumentFileName("Pluck", midiNoteNumber);
			String path = cwd + "/Resources/" + fileName;

			//Prase File
			AudioFormatReader* reader = formatManager.createReaderFor(path);
			File file(path);
			if (reader != nullptr)
			{
				ScopedPointer<AudioFormatReaderSource> newSource = new AudioFormatReaderSource(reader, true);
				transportSource.setSource(newSource, 0, nullptr, reader->sampleRate);
				readerSource = newSource.release();
				transportSource.start();
			}
			else
			{
				logMessage("Impossible to play this note on an ErHu");
			}
		}
		else
		{
			logMessage("You Need to Pluck or Bow the ErHu");
		}
		logMessage("\n");
	}

	//=========================================================================================================================
	//Midikeyboard Enum
	enum TransportState
	{
		Stopped,
		Starting,
		Playing,
		Stopping
	};

	//Midikeyboard Function
	void changeState(TransportState newState)
	{
		if (state != newState)
		{
			state = newState;
			switch (state)
			{
			case Stopped:
				transportSource.setPosition(0.0);
				break;

			case Starting:
				transportSource.start();
				break;

			case Playing:
				break;

			case Stopping:
				transportSource.stop();
				break;
			}
		}
	}

	//Midikeyboard Function
	void setMidiInput(int index)
	{
		const StringArray list(MidiInput::getDevices());

		deviceManager.removeMidiInputCallback(list[lastInputIndex], this);

		const String newInput(list[index]);

		if (!deviceManager.isMidiInputEnabled(newInput))
			deviceManager.setMidiInputEnabled(newInput, true);

		deviceManager.addMidiInputCallback(newInput, this);
		midiInputList.setSelectedId(index + 1, dontSendNotification);

		lastInputIndex = index;
	}

	void comboBoxChanged(ComboBox* box) override
	{
		if (box == &midiInputList)
			setMidiInput(midiInputList.getSelectedItemIndex());
	}



	void handleNoteOn(MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override
	{
		String midiName = getMidiname(midiNoteNumber);
		logMessage("New Midi Value - " + midiName);

		auto bowState = bow_button.getToggleState();
		auto pluckState = pluck_button.getToggleState();

		String stateString_bow = bowState ? "ON" : "OFF";
		String stateString_pluck = pluckState ? "ON" : "OFF";

		playSound(stateString_bow, stateString_pluck, midiNoteNumber, velocity, "ErHu_Piano");
	}


	void handleNoteOff(MidiKeyboardState*, int midiChannel, int midiNoteNumber, float /*velocity*/) override
	{

		if (transportSource.isPlaying()) {
			//logMessage("Still playing");
			transportSource.stop();
			return;
		}

		if (!isAddingFromMidiInput)
		{
			MidiMessage m(MidiMessage::noteOff(midiChannel, midiNoteNumber));
			m.setTimeStamp(Time::getMillisecondCounterHiRes() * 0.001);
		}
	}

	//=========================================================================================================================
	//Function for ErHu Strings
	void mouseDown(const MouseEvent& e) override
	{
		mouseDrag(e);
	}

	//Function for ErHu Strings
	void mouseDrag(const MouseEvent& e) override
	{
		//Decide which string should be palyed
		if (A_string.getBounds().contains(e.getPosition()))
		{
			auto position = (e.position.x - A_string.getX()) / A_string.getWidth();
			A_string.stringPlucked(position);

			float relativePitch = e.position.x / (canvas_width / 13); //13 means an octave plus one extra note
			playStrings(A_string.getMidi() + (int)relativePitch, scaleSlider.getValue());
		}
		else if (D_string.getBounds().contains(e.getPosition()))
		{
			auto position = (e.position.x - D_string.getX()) / D_string.getWidth();
			D_string.stringPlucked(position);

			float relativePitch = e.position.x / (canvas_width / 13); //13 means an octave plus one extra note
			playStrings(D_string.getMidi() + (int)relativePitch, scaleSlider.getValue());
		}
	}

	//Function for ErHu Strings
	void playStrings(int midi_value, float velocity)
	{

		String midiName = getMidiname(midi_value);
		logMessage("New Midi Value - " + midiName);

		auto bowState = bow_button.getToggleState();
		auto pluckState = pluck_button.getToggleState();

		String stateString_bow = bowState ? "ON" : "OFF";
		String stateString_pluck = pluckState ? "ON" : "OFF";

		playSound(stateString_bow, stateString_pluck, midi_value, velocity, "String");
	}

	//==============================================================================
	// Fundamental Components
	int                lastInputIndex;
	bool               isAddingFromMidiInput;

	//General GUI Components
	ToggleButton          bow_button, pluck_button;
	ComboBox              midiInputList;
	Label                 midiInputList_Label;
	TextEditor            midiMessagesBox;
	TextButton            flanger;

	//ErHu Components
	Label                 scaleLabel{ "Velocity: " }, scaleLabel_name, distortionLabel{ "Distortion" }, distortionLabel_name;
	Slider                scaleSlider, distortionSlider;
	ErHuString            A_string, D_string;

	//MIDI KeyBoard Components
	MidiKeyboardState     keyboardState;
	MidiKeyboardComponent keyboardComponent;
	TransportState        state;

	// Audio Components
	AudioDeviceManager                     deviceManager;
	AudioTransportSource                   transportSource;
	ScopedPointer<AudioFormatReaderSource> readerSource;
	AudioFormatManager                     formatManager;

	//Audio Effect Components
	FlangerAudioProcessor processor;
	Array<FlangerAudioProcessorEditor::SafePointer<FlangerAudioProcessorEditor>> window;
	FlangerAudioProcessorEditor *flangerAudioProcessorEditor = new FlangerAudioProcessorEditor(processor);

	//Audio Processing
	Random random;
	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent);
};

// (This function is called by the app startup code to create our main component)
Component* createMainContentComponent() { return new MainContentComponent(); }
#endif  // MAINCOMPONENT_H_INCLUDED
