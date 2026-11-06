#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// Форма дисторшна для выбранного режима
static inline float shape(float x, int mode)
{
    switch (mode)
    {
    case 0:  return std::tanh(x);                                   // Soft
    case 1:  return juce::jlimit(-1.f, 1.f, x);                     // Hard
    case 2:  return (2.f / juce::MathConstants<float>::pi)           // Fold
        * std::asin(std::sin(x * juce::MathConstants<float>::halfPi));
    case 3:  return std::tanh(x + 0.2f) - std::tanh(0.2f);         // Asym
    default: return x >= 0.f ? std::tanh(x * 3.f)                   // Fuzz
        : 0.7f * std::tanh(x);
    }
}

//==============================================================================
NECRODISTAudioProcessor::NECRODISTAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    apvts(*this, nullptr, "PARAMS", createLayout())
#endif
{
}

NECRODISTAudioProcessor::~NECRODISTAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout NECRODISTAudioProcessor::createLayout()
{
    using P = juce::AudioParameterFloat;
    using R = juce::NormalisableRange<float>;
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<P>(juce::ParameterID{ "drive", 1 }, "Drive", R(0.f, 40.f, 0.1f), 12.f));
    layout.add(std::make_unique<P>(juce::ParameterID{ "tone", 1 }, "Tone", R(500.f, 20000.f, 1.f, 0.3f), 8000.f));
    layout.add(std::make_unique<P>(juce::ParameterID{ "mix", 1 }, "Mix", R(0.f, 1.f, 0.01f), 1.f));
    layout.add(std::make_unique<P>(juce::ParameterID{ "output", 1 }, "Output", R(-24.f, 12.f, 0.1f), 0.f));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{ "power", 1 }, "Power", true));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ "mode", 1 }, "Mode",
        juce::StringArray{ "Soft", "Hard", "Fold", "Asym", "Fuzz" }, 0));

    return layout;
}

//==============================================================================
const juce::String NECRODISTAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NECRODISTAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool NECRODISTAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool NECRODISTAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double NECRODISTAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NECRODISTAudioProcessor::getNumPrograms()
{
    return 1;
}

int NECRODISTAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NECRODISTAudioProcessor::setCurrentProgram(int) {}

const juce::String NECRODISTAudioProcessor::getProgramName(int)
{
    return {};
}

void NECRODISTAudioProcessor::changeProgramName(int, const juce::String&) {}

//==============================================================================
void NECRODISTAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec{ sampleRate,
                                  (juce::uint32)samplesPerBlock,
                                  (juce::uint32)getTotalNumOutputChannels() };

    oversampling.initProcessing((size_t)samplesPerBlock);
    oversampling.reset();

    tone.prepare(spec);
    tone.setType(juce::dsp::StateVariableTPTFilterType::lowpass);

    dcBlock.prepare(spec);
    dcBlock.setType(juce::dsp::StateVariableTPTFilterType::highpass);
    dcBlock.setCutoffFrequency(20.f);

    mixer.prepare(spec);
    mixer.setWetLatency((float)oversampling.getLatencyInSamples());
    setLatencySamples((int)oversampling.getLatencyInSamples());

    // Сглаживание параметров (20-50 мс)
    const double osRate = sampleRate * (double)oversampling.getOversamplingFactor();

    const float driveGain = juce::Decibels::decibelsToGain(apvts.getRawParameterValue("drive")->load());
    const float outGain = juce::Decibels::decibelsToGain(apvts.getRawParameterValue("output")->load());
    const float toneHz = apvts.getRawParameterValue("tone")->load();

    driveSmooth.reset(osRate, 0.02);
    driveSmooth.setCurrentAndTargetValue(driveGain);

    outSmooth.reset(sampleRate, 0.02);
    outSmooth.setCurrentAndTargetValue(outGain);

    toneSmooth.reset(sampleRate, 0.05);
    toneSmooth.setCurrentAndTargetValue(toneHz);
    tone.setCutoffFrequency(toneHz);
}

void NECRODISTAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NECRODISTAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void NECRODISTAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    const bool  on = apvts.getRawParameterValue("power")->load() >= 0.5f;
    const int   mode = (int)apvts.getRawParameterValue("mode")->load();
    const float mixP = apvts.getRawParameterValue("mix")->load();

    driveSmooth.setTargetValue(juce::Decibels::decibelsToGain(apvts.getRawParameterValue("drive")->load()));
    outSmooth.setTargetValue(juce::Decibels::decibelsToGain(apvts.getRawParameterValue("output")->load()));
    toneSmooth.setTargetValue(apvts.getRawParameterValue("tone")->load());

    // Выключение = плавный уход Mix в ноль (без щелчков, задержка сохраняется)
    mixer.setWetMixProportion(on ? mixP : 0.f);

    juce::dsp::AudioBlock<float> block(buffer);
    mixer.pushDrySamples(block);

    // --- дисторшн на повышенной частоте дискретизации (4x) ---
    auto osBlock = oversampling.processSamplesUp(block);
    const size_t osCh = osBlock.getNumChannels();
    const size_t osN = osBlock.getNumSamples();

    for (size_t i = 0; i < osN; ++i)
    {
        const float g = driveSmooth.getNextValue();
        for (size_t ch = 0; ch < osCh; ++ch)
        {
            float* d = osBlock.getChannelPointer(ch);
            d[i] = shape(d[i] * g, mode);
        }
    }
    oversampling.processSamplesDown(block);

    // --- DC-блокер, Tone и Output со сглаживанием ---
    const size_t n = block.getNumSamples();
    const size_t nc = block.getNumChannels();

    for (size_t i = 0; i < n; ++i)
    {
        const float cutoff = toneSmooth.getNextValue();
        if ((i & 15) == 0)
            tone.setCutoffFrequency(cutoff);

        const float og = outSmooth.getNextValue();

        for (size_t ch = 0; ch < nc; ++ch)
        {
            float s = block.getSample((int)ch, (int)i);
            s = dcBlock.processSample((int)ch, s);
            s = tone.processSample((int)ch, s);
            block.setSample((int)ch, (int)i, s * og);
        }
    }

    mixer.mixWetSamples(block);
}

//==============================================================================
bool NECRODISTAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* NECRODISTAudioProcessor::createEditor()
{
    return new NECRODISTAudioProcessorEditor(*this);
}

//==============================================================================
void NECRODISTAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void NECRODISTAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NECRODISTAudioProcessor();
}