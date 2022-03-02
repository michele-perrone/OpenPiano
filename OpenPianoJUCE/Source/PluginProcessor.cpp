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
OpenPianoAudioProcessor::OpenPianoAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::mono(), true)
                     #endif
                       )
#endif
{
    pedal_down_current = false;
}

OpenPianoAudioProcessor::~OpenPianoAudioProcessor()
{
}

//==============================================================================
const juce::String OpenPianoAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool OpenPianoAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool OpenPianoAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool OpenPianoAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double OpenPianoAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int OpenPianoAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int OpenPianoAudioProcessor::getCurrentProgram()
{
    return 0;
}

void OpenPianoAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String OpenPianoAudioProcessor::getProgramName (int index)
{
    return {};
}

void OpenPianoAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void OpenPianoAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    // Initialize the piano and the output buffer
    piano = new Piano(sampleRate, samplesPerBlock, std::thread::hardware_concurrency());
}

void OpenPianoAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    delete piano;
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool OpenPianoAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void OpenPianoAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());


    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    keyboardState.processNextMidiBuffer (midiMessages, 0, buffer.getNumSamples(), true);

    for (const auto metadata : midiMessages)
    {
        juce::MidiMessage message = metadata.getMessage();

        // To correctly deal with the pedal, we have to consider that some pedals send
        // continuous messages. Right now, damping is implemented on a simple on/off
        // basis, so we're only interested when the pedal goes up and down, nothing
        // in between. To do so, we compare the current state of the pedal with the
        // previous one. We damp the strings only when the pedal changes from down
        // to up, otherwise we end up with a mess of messages to deal with.
        if (message.isSustainPedalOn())
        {
            pedal_down_previous = pedal_down_current;
            pedal_down_current = true;
        }
        else if (message.isSustainPedalOff())
        {
            pedal_down_previous = pedal_down_current;
            pedal_down_current = false;
            if(pedal_down_previous != pedal_down_current)
            {
                for (int i = 0; i < N_STRINGS; i++)
                {
                    // Damp only those strings which are not currently held down on the keyboard
                    if(!keyboardState.isNoteOn(1, i+MIDI_NOTE_OFFSET))
                        piano->strings[i]->damp();
                }
            }
        }
        else if (message.isNoteOn())
        {
            piano->strings[message.getNoteNumber()-MIDI_NOTE_OFFSET]->hit(message.getVelocity()/30.0);
        }
        // Strings are damped only if pedal is not down
        else if (message.isNoteOff() && !pedal_down_current)
        {
            piano->strings[message.getNoteNumber()-MIDI_NOTE_OFFSET]->damp();
        }
    }

    int samplesPerBlock = buffer.getNumSamples();
    float* outputChannelData = buffer.getWritePointer(0);
    float gain = 150;
    piano->get_next_block_multithreaded(outputChannelData, samplesPerBlock, gain);
}

//==============================================================================
bool OpenPianoAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* OpenPianoAudioProcessor::createEditor()
{
    return new OpenPianoAudioProcessorEditor (*this);
}

//==============================================================================
void OpenPianoAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void OpenPianoAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OpenPianoAudioProcessor();
}
