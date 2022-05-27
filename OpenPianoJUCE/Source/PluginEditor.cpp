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
    : AudioProcessorEditor (&p),
      audioProcessor (p),
      midiKeyboard (audioProcessor.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard),
      spectrogramComponent(p.spectrogramComponent)
{
    init_keyboard();
    setResizable(true, true);

    int screen_width = Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea.getWidth();
    int screen_height = Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea.getHeight();
    setSize (screen_width*3/4, screen_height*5/6);
}

OpenPianoAudioProcessorEditor::~OpenPianoAudioProcessorEditor()
{
    keyboardState.removeListener (this);
}

//==============================================================================
void OpenPianoAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
    g.setOpacity (1.0f);

    Rectangle<int> area(getLocalBounds());
    Rectangle<int> area_spectrogram(area.removeFromTop(area.getHeight()*0.75));

    // Signal the area where the spectrogram resides for being repainted...
    // (maybe this could be optimized a bit if this->repaint were called only
    //  after SpectrogramComponent::drawNextLineOfSpectrogram() had finished)
    repaint(area_spectrogram);
    // ... and draw the current spectrogram.
    g.drawImage(spectrogramComponent.getSpectrogramImage(), area_spectrogram.toFloat());
}

void OpenPianoAudioProcessorEditor::resized()
{
    Rectangle<int> area(getLocalBounds());
    int control_board_height = area.getHeight()*0.75;
    int keyboard_height = area.getHeight()*0.25;

    /*************************************************************************/
    /****************************** Control Board ****************************/
    /*************************************************************************/
    // This area is currently used by the spectrogram component!
    area.removeFromTop(control_board_height);


    /*************************************************************************/
    /******************************** Keyboard *******************************/
    /*************************************************************************/
    Rectangle<int> area_keyboard(area.removeFromTop(keyboard_height));
    midiKeyboard.setKeyWidth(area.getWidth()/(float)N_WHITE_KEYS);
    midiKeyboard.setBounds(area_keyboard);
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
