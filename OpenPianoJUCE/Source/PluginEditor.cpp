/*
OpenPiano: an open source piano engine based on physical modeling
Copyright (C) 2021-2022 Michele Perrone
Github: https://github.com/michele-perrone/OpenPiano
Author e-mail: perrone(dot)michele(at)outlook(dot)com
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published
by the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
