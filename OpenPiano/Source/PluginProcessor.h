/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../OpenPianoCore/string_hammer.h"



//==============================================================================
/**
*/
class OpenPianoAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    OpenPianoAudioProcessor();
    ~OpenPianoAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenPianoAudioProcessor)

    Hammer* hammer_C2;
    String* string_C2;

    Hammer* hammer_D2;
    String* string_D2;

    Hammer* hammer_E2;
    String* string_E2;

    Hammer* hammer_F2;
    String* string_F2;

    Hammer* hammer_G2;
    String* string_G2;

    Hammer* hammer_A2;
    String* string_A2;

    Hammer* hammer_B2;
    String* string_B2;

    Hammer* hammer_C3;
    String* string_C3;

    Hammer* hammer_D3;
    String* string_D3;

    Hammer* hammer_E3;
    String* string_E3;

    Hammer* hammer_F3;
    String* string_F3;

    Hammer* hammer_G3;
    String* string_G3;

    Hammer* hammer_A3;
    String* string_A3;

    Hammer* hammer_B3;
    String* string_B3;

    Hammer* hammer_C4;
    String* string_C4;
};
