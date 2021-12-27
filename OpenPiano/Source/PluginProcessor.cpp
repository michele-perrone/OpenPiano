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
                       .withOutput ("Output", juce::AudioChannelSet::mono(), true)
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

    // Initialize the piano and the output buffer
    piano = new Piano(sampleRate);
    outputBuffer_oct_2 = (float*)malloc(samplesPerBlock*sizeof (float));
    outputBuffer_oct_3 = (float*)malloc(samplesPerBlock*sizeof (float));
}

void OpenPianoAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    delete piano;
    free(outputBuffer_oct_2);
    free(outputBuffer_oct_3);
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

        if (message.isNoteOn())
        {
            switch (message.getNoteNumber())
            {
            case 36:
                piano->string_C2->hit(message.getVelocity()/30.0);
                break;
            case 37:
                piano->string_C2s->hit(message.getVelocity()/30.0);
                break;
            case 38:
                piano->string_D2->hit(message.getVelocity()/30.0);
                break;
            case 39:
                piano->string_D2s->hit(message.getVelocity()/30.0);
                break;
            case 40:
                piano->string_E2->hit(message.getVelocity()/30.0);
                break;
            case 41:
                piano->string_F2->hit(message.getVelocity()/30.0);
                break;
            case 42:
                piano->string_F2s->hit(message.getVelocity()/30.0);
                break;
            case 43:
                piano->string_G2->hit(message.getVelocity()/30.0);
                break;
            case 44:
                piano->string_G2s->hit(message.getVelocity()/30.0);
                break;
            case 45:
                piano->string_A2->hit(message.getVelocity()/30.0);
                break;
            case 46:
                piano->string_A2s->hit(message.getVelocity()/30.0);
                break;
            case 47:
                piano->string_B2->hit(message.getVelocity()/30.0);
                break;
            case 48:
                piano->string_C3->hit(message.getVelocity()/30.0);
                break;
            case 49:
                piano->string_C3s->hit(message.getVelocity()/30.0);
                break;
            case 50:
                piano->string_D3->hit(message.getVelocity()/30.0);
                break;
            case 51:
                piano->string_D3s->hit(message.getVelocity()/30.0);
                break;
            case 52:
                piano->string_E3->hit(message.getVelocity()/30.0);
                break;
            case 53:
                piano->string_F3->hit(message.getVelocity()/30.0);
                break;
            case 54:
                piano->string_F3s->hit(message.getVelocity()/30.0);
                break;
            case 55:
                piano->string_G3->hit(message.getVelocity()/30.0);
                break;
            case 56:
                piano->string_G3s->hit(message.getVelocity()/30.0);
                break;
            case 57:
                piano->string_A3->hit(message.getVelocity()/30.0);
                break;
            case 58:
                piano->string_A3s->hit(message.getVelocity()/30.0);
                break;
            case 59:
                piano->string_B3->hit(message.getVelocity()/30.0);
                break;
            case 60:
                piano->string_C4->hit(message.getVelocity()/30.0);
                break;
            default:
                break;
            }
        }
    }

    int numSamples = buffer.getNumSamples();
    float* outputChannelData = buffer.getWritePointer (0);
    float gain = 200;
    piano->get_next_block_multithreaded(outputChannelData, outputBuffer_oct_2, outputBuffer_oct_3, numSamples, gain);

    /*for (int i = 0; i < numSamples; ++i)
    {
        float gain = 250;
        float currentSample = gain*piano->get_next_sample();
        for (int channel = 0; channel < totalNumOutputChannels; ++channel)
        {
            float* outputChannelData = buffer.getWritePointer (channel);
            outputChannelData[i] = currentSample;
        }
    }*/
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
