#pragma once

#include <JuceHeader.h>

//==============================================================================
class NECRODISTAudioProcessor : public juce::AudioProcessor
{
public:
    NECRODISTAudioProcessor();
    ~NECRODISTAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

private:
    using Smoothed = juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative>;

    juce::dsp::Oversampling<float> oversampling{ 2, 2,
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR }; // 4x
    juce::dsp::StateVariableTPTFilter<float> tone;
    juce::dsp::StateVariableTPTFilter<float> dcBlock;
    juce::dsp::DryWetMixer<float> mixer;

    Smoothed driveSmooth, outSmooth, toneSmooth;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NECRODISTAudioProcessor)
};