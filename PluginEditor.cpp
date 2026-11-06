#include "PluginProcessor.h"
#include "PluginEditor.h"

NECRODISTAudioProcessorEditor::NECRODISTAudioProcessorEditor(NECRODISTAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&laf);

    setupKnob(drive, driveLabel, "DRIVE", "drive", " dB", driveAtt);
    setupKnob(tone, toneLabel, "TONE", "tone", " Hz", toneAtt);
    setupKnob(mix, mixLabel, "MIX", "mix", "", mixAtt);
    setupKnob(output, outputLabel, "OUTPUT", "output", " dB", outputAtt);

    addAndMakeVisible(powerButton);
    powerAtt = std::make_unique<ButtonAtt>(audioProcessor.apvts, "power", powerButton);

    // выбор режима дисторшна
    if (auto* choice = dynamic_cast<juce::AudioParameterChoice*> (audioProcessor.apvts.getParameter("mode")))
        modeBox.addItemList(choice->choices, 1);

    modeBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff1b1d22));
    modeBox.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff3a3d45));
    modeBox.setColour(juce::ComboBox::textColourId, juce::Colours::white);
    modeBox.setColour(juce::ComboBox::arrowColourId, juce::Colour(0xffc01818));
    addAndMakeVisible(modeBox);
    modeAtt = std::make_unique<ComboAtt>(audioProcessor.apvts, "mode", modeBox);

    setSize(460, 260);
}

NECRODISTAudioProcessorEditor::~NECRODISTAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void NECRODISTAudioProcessorEditor::setupKnob(KnobSlider& s, juce::Label& l,
    const juce::String& name,
    const juce::String& paramID,
    const juce::String& suffix,
    std::unique_ptr<SliderAtt>& att)
{
    addAndMakeVisible(s);
    s.setTextValueSuffix(suffix);
    s.param = audioProcessor.apvts.getParameter(paramID);
    s.owner = this;

    l.setText(name, juce::dontSendNotification);
    l.setJustificationType(juce::Justification::centred);
    l.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.85f));
    addAndMakeVisible(l);

    att = std::make_unique<SliderAtt>(audioProcessor.apvts, paramID, s);
}

void NECRODISTAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff121316));

    g.setColour(juce::Colour(0xffc01818));
    g.setFont(juce::FontOptions(26.f, juce::Font::bold));
    g.drawText("NECRODIST", 16, 10, 170, 36, juce::Justification::centredLeft);
}

void NECRODISTAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(12);

    auto header = area.removeFromTop(40);
    powerButton.setBounds(header.removeFromRight(70).reduced(0, 6));
    header.removeFromRight(8);
    modeBox.setBounds(header.removeFromRight(110).reduced(0, 7));

    area.removeFromTop(10);
    const int w = area.getWidth() / 4;

    KnobSlider* knobs[] = { &drive, &tone, &mix, &output };
    juce::Label* labels[] = { &driveLabel, &toneLabel, &mixLabel, &outputLabel };

    for (int i = 0; i < 4; ++i)
    {
        auto col = area.removeFromLeft(w);
        labels[i]->setBounds(col.removeFromTop(22));
        knobs[i]->setBounds(col);
    }
}