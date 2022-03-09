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
    init_keyboard();
    setResizable(true, true);

    int screen_width = Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea.getWidth();
    int screen_height = Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea.getHeight();
    setSize (screen_height*3/4, screen_height/12);
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
    juce::Grid grid;

    using Track = juce::Grid::TrackInfo;
    grid.templateRows = { Track(1_fr) };
    grid.templateColumns = { Track(1_fr) };

    midiKeyboard.setKeyWidth(getLocalBounds().getWidth()/(float)N_WHITE_KEYS);
    grid.items =
    {
        GridItem(midiKeyboard)
    };
    grid.performLayout(getLocalBounds());
}

void OpenPianoAudioProcessorEditor::handleNoteOn (juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity)
{
    keyboardState.noteOn(midiChannel, midiNoteNumber, velocity);
}

void OpenPianoAudioProcessorEditor::handleNoteOff (juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity)
{
    keyboardState.noteOff(midiChannel, midiNoteNumber, velocity);
}

void OpenPianoAudioProcessorEditor::init_keyboard()
{
    midiKeyboard.setName ("MIDI Keyboard");
    midiKeyboard.setAvailableRange(FIRST_NOTE+MIDI_NOTE_OFFSET, LAST_NOTE+MIDI_NOTE_OFFSET);
    midiKeyboard.setOctaveForMiddleC(4);
    addAndMakeVisible (midiKeyboard);

    keyboardState.addListener (this);
}
