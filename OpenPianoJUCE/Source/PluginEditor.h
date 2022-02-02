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
