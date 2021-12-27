/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/

class OpenPianoAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::MidiKeyboardState::Listener
{
public:
    OpenPianoAudioProcessorEditor (OpenPianoAudioProcessor&);
    ~OpenPianoAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    OpenPianoAudioProcessor& audioProcessor;

    juce::MidiKeyboardState keyboardState;
    juce::MidiKeyboardComponent midiKeyboard;

    void handleNoteOn (juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;
    void handleNoteOff (juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenPianoAudioProcessorEditor)
};
