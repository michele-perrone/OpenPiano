/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
OpenPianoAudioProcessorEditor::OpenPianoAudioProcessorEditor (OpenPianoAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), midiKeyboard (audioProcessor.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    midiKeyboard.setName ("MIDI Keyboard");
    midiKeyboard.setAvailableRange(FIRST_NOTE+MIDI_NOTE_OFFSET, LAST_NOTE+MIDI_NOTE_OFFSET);
    midiKeyboard.setKeyWidth(24);
    midiKeyboard.setOctaveForMiddleC(4);
    addAndMakeVisible (midiKeyboard);

    keyboardState.addListener (this);

    setSize (24*N_WHITE_KEYS, 128); // 52 -> number of white keys
}

OpenPianoAudioProcessorEditor::~OpenPianoAudioProcessorEditor()
{
    keyboardState.removeListener (this);
}

//==============================================================================
void OpenPianoAudioProcessorEditor::paint (juce::Graphics& g)
{
}

void OpenPianoAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    midiKeyboard.setBounds(0, 0, getWidth(), getHeight());
}

void OpenPianoAudioProcessorEditor::handleNoteOn (juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity)
{
    keyboardState.noteOn(midiChannel, midiNoteNumber, velocity);
}

void OpenPianoAudioProcessorEditor::handleNoteOff (juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity)
{
    keyboardState.noteOff(midiChannel, midiNoteNumber, velocity);
}
