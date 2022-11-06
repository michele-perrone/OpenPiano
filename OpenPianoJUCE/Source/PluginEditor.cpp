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
      spectrogramComponent(p.spectrogramComponent),
      harmonicRatioComponent(p.harmonicRatioComponent)
{
    init_control_board();
    init_keyboard();
    setResizable(true, true);

    plotType = 0; // By default, plot the STFT

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
    Rectangle<int> area_plot(area.removeFromTop(area.getHeight()*0.75));
    area_plot.removeFromTop(area_plot.getHeight()*0.11); // Buttons


    // Signal the area where the plot resides for being repainted...
    // (maybe this could be optimized a bit if this->repaint were called only
    //  after SpectrogramComponent::drawNextLineOfSpectrogram() had finished)
    repaint(area_plot);

    if(plotType == 0)
    {
        // Draw the text showing the frequencies
//        int bottom = area_plot.getBottom();
//        int height = area_plot.getHeight();
//        int step = (height-bottom)/5;
//        int coord = bottom;
//        for(int i = 0; i < 5; i++)
//        {
//            std::cout << "coord: " << coord << std::endl;
//            g.setColour(juce::Colours::white);
//            g.drawSingleLineText(std::to_string(spectrogramComponent->FFT_frequencies[i]), 0, area_plot.getCentreY(), Justification::left);
//            coord += step;
//        }
//        std::cout << "center: " << area_plot.getCentreY() << std::endl;

        // Draw the STFT
        g.drawImage(spectrogramComponent->getSpectrogramImage(), area_plot.toFloat());
    }
    else if(plotType == 1)
    {
        // Draw the text showing the harmonic ratio range (ticks)
        // ...

        // Draw the harmonic ratio range
        g.drawImage(harmonicRatioComponent->getharmonicRatioImage(), area_plot.toFloat());
    }
}

void OpenPianoAudioProcessorEditor::resized()
{
    Rectangle<int> area(getLocalBounds());
    int control_board_height = area.getHeight()*0.75;
    int keyboard_height = area.getHeight()*0.25;

    /*************************************************************************/
    /****************************** Control Board ****************************/
    /*************************************************************************/
    // This area is currently used by the spectrogram/harmonic ratio component!
    Rectangle<int> area_control_board(area.removeFromTop(control_board_height));
    Rectangle<int> area_buttons(area_control_board.removeFromTop(area_control_board.getHeight()*0.1));
    spectrogramButton.setBounds(area_buttons.removeFromTop(area_buttons.getHeight()*0.5));
    harmonicRatioButton.setBounds(area_buttons);


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

void OpenPianoAudioProcessorEditor::init_control_board()
{
    spectrogramButton.setClickingTogglesState(false);
    spectrogramButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::darkred);
    spectrogramButton.setButtonText("Show STFT");
    spectrogramButton.onClick = [this] { this->plotType = 0; };
    addAndMakeVisible(spectrogramButton);

    harmonicRatioButton.setClickingTogglesState(false);
    harmonicRatioButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::darkred);
    harmonicRatioButton.setButtonText("Show harmonic ratio");
    harmonicRatioButton.onClick = [this] { this->plotType = 1; };
    addAndMakeVisible(harmonicRatioButton);
}
