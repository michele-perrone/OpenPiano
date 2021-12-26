/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
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
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
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

    // Initialize the hammer with its physical parameters
    hammer_C2 = new Hammer(sampleRate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
    hammer_D2 = new Hammer(sampleRate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
    hammer_E2 = new Hammer(sampleRate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
    hammer_F2 = new Hammer(sampleRate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
    hammer_G2 = new Hammer(sampleRate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
    hammer_A2 = new Hammer(sampleRate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
    hammer_B2 = new Hammer(sampleRate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
    hammer_C3 = new Hammer(sampleRate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
    hammer_D3 = new Hammer(sampleRate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
    hammer_E3 = new Hammer(sampleRate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
    hammer_F3 = new Hammer(sampleRate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
    hammer_G3 = new Hammer(sampleRate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
    hammer_A3 = new Hammer(sampleRate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
    hammer_B3 = new Hammer(sampleRate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);
    hammer_C4 = new Hammer(sampleRate, 4.9e-03, 2.3, 1e-04, 4e08, 0.12, 0.05);


    // Initialize the string with its physical parameters
    string_C2 = new String(sampleRate, 65.41, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_C2);
    string_D2 = new String(sampleRate, 73.42, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_D2);
    string_E2 = new String(sampleRate, 82.41, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_E2);
    string_F2 = new String(sampleRate, 87.31, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_F2);
    string_G2 = new String(sampleRate, 98.00, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_G2);
    string_A2 = new String(sampleRate, 110.00, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_A2);
    string_B2 = new String(sampleRate, 123.47, 1.92, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_B2);
    string_C3 = new String(sampleRate, 130.81, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_C3);
    string_D3 = new String(sampleRate, 146.83, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_D3);
    string_E3 = new String(sampleRate, 164.81, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_E3);
    string_F3 = new String(sampleRate, 174.61, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_F3);
    string_G3 = new String(sampleRate, 196, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_G3);
    string_A3 = new String(sampleRate, 220, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_A3);
    string_B3 = new String(sampleRate, 246.94, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_B3);
    string_C4 = new String(sampleRate, 261.63, 0.96, 0.0182, 0.001, 9e7, 0.003, 6.25e-9, hammer_C4);

}

void OpenPianoAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    delete hammer_C2;
    delete string_C2;

    delete hammer_D2;
    delete string_D2;

    delete hammer_E2;
    delete string_E2;

    delete hammer_F2;
    delete string_F2;

    delete hammer_G2;
    delete string_G2;

    delete hammer_A2;
    delete string_A2;

    delete hammer_B2;
    delete string_B2;

    delete hammer_C3;
    delete string_C3;

    delete hammer_D3;
    delete string_D3;

    delete hammer_E3;
    delete string_E3;

    delete hammer_F3;
    delete string_F3;

    delete hammer_G3;
    delete string_G3;

    delete hammer_A3;
    delete string_A3;

    delete hammer_B3;
    delete string_B3;

    delete hammer_C4;
    delete string_C4;
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
    juce::MidiBuffer processedMidi;

    for (const auto metadata : midiMessages)
    {
        juce::MidiMessage message = metadata.getMessage();

        if (message.isNoteOn())
        {
            switch (message.getNoteNumber())
            {
            case 36:
                string_C2->hit(message.getVelocity()/30.0);
                break;
            case 38:
                string_D2->hit(message.getVelocity()/30.0);
                break;
            case 40:
                string_E2->hit(message.getVelocity()/30.0);
                break;
            case 41:
                string_F2->hit(message.getVelocity()/30.0);
                break;
            case 43:
                string_G2->hit(message.getVelocity()/30.0);
                break;
            case 45:
                string_A2->hit(message.getVelocity()/30.0);
                break;
            case 47:
                string_B2->hit(message.getVelocity()/30.0);
                break;
            case 48:
                string_C3->hit(message.getVelocity()/30.0);
                break;
            case 50:
                string_D3->hit(message.getVelocity()/30.0);
                break;
            case 52:
                string_E3->hit(message.getVelocity()/30.0);
                break;
            case 53:
                string_F3->hit(message.getVelocity()/30.0);
                break;
            case 55:
                string_G3->hit(message.getVelocity()/30.0);
                break;
            case 57:
                string_A3->hit(message.getVelocity()/30.0);
                break;
            case 59:
                string_B3->hit(message.getVelocity()/30.0);
                break;
            case 60:
                string_C4->hit(message.getVelocity()/30.0);
                break;
            default:
                break;
            }
        }
    }

    int numSamples = buffer.getNumSamples();
    for (int i = 0; i < numSamples; ++i)
    {
        float currentSample = 250*(string_C2->get_next_sample()                                   
                                   + string_D2->get_next_sample()
                                   + string_E2->get_next_sample()
                                   + string_F2->get_next_sample()
                                   + string_G2->get_next_sample()
                                   + string_A2->get_next_sample()
                                   + string_B2->get_next_sample()
                                   + string_C3->get_next_sample()
                                   + string_D3->get_next_sample()
                                   + string_E3->get_next_sample()
                                   + string_F3->get_next_sample()
                                   + string_G3->get_next_sample()
                                   + string_A3->get_next_sample()
                                   + string_B3->get_next_sample()
                                   + string_C4->get_next_sample());
        for (int channel = 0; channel < totalNumOutputChannels; ++channel)
        {
            float* outputChannelData = buffer.getWritePointer (channel);
            outputChannelData[i] = currentSample;
        }
    }
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
