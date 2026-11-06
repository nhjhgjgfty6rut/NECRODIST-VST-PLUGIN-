#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
// Внешний вид ручек и кнопки
class NecroLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
        float pos, float startAngle, float endAngle,
        juce::Slider&) override
    {
        auto bounds = juce::Rectangle<int>(x, y, w, h).toFloat().reduced(6.f);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.f;
        auto centre = bounds.getCentre();
        auto angle = startAngle + pos * (endAngle - startAngle);

        // тело ручки
        g.setColour(juce::Colour(0xff1b1d22));
        g.fillEllipse(centre.x - radius, centre.y - radius, radius * 2.f, radius * 2.f);

        // дорожка и заполнение
        auto arcR = radius - 4.f;
        juce::Path track, value;
        track.addCentredArc(centre.x, centre.y, arcR, arcR, 0.f, startAngle, endAngle, true);
        value.addCentredArc(centre.x, centre.y, arcR, arcR, 0.f, startAngle, angle, true);

        g.setColour(juce::Colour(0xff3a3d45));
        g.strokePath(track, juce::PathStrokeType(3.f));
        g.setColour(juce::Colour(0xffc01818));
        g.strokePath(value, juce::PathStrokeType(3.f, juce::PathStrokeType::curved,
            juce::PathStrokeType::rounded));

        // указатель
        juce::Path p;
        p.addRoundedRectangle(-1.5f, -arcR + 6.f, 3.f, arcR * 0.45f, 1.5f);
        g.setColour(juce::Colours::white);
        g.fillPath(p, juce::AffineTransform::rotation(angle).translated(centre));
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& b, bool, bool) override
    {
        auto r = b.getLocalBounds().toFloat().reduced(1.f);
        const bool on = b.getToggleState();

        g.setColour(on ? juce::Colour(0xffc01818) : juce::Colour(0xff2a2d33));
        g.fillRoundedRectangle(r, 6.f);
        g.setColour(juce::Colours::white.withAlpha(0.8f));
        g.drawRoundedRectangle(r, 6.f, 1.f);
        g.setFont(juce::FontOptions(14.f, juce::Font::bold));
        g.drawText(on ? "ON" : "OFF", b.getLocalBounds(), juce::Justification::centred);
    }
};

//==============================================================================
// Ручка, которая по правому клику показывает меню хоста (FL Studio)
class KnobSlider : public juce::Slider
{
public:
    KnobSlider()
    {
        setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 18);
    }

    juce::AudioProcessorParameter* param = nullptr;
    juce::AudioProcessorEditor* owner = nullptr;

    void mouseDown(const juce::MouseEvent& e) override
    {
        if (e.mods.isPopupMenu() && param != nullptr && owner != nullptr)
        {
            if (auto* ctx = owner->getHostContext())
            {
                if (auto menu = ctx->getContextMenuForParameter(param))
                {
                    // Позиция клика относительно окна редактора (не экрана)
                    auto pos = owner->getLocalPoint(this, e.getPosition());
                    menu->showNativeMenu(pos);
                    return;
                }
            }
        }
        juce::Slider::mouseDown(e);
    }
};

//==============================================================================
class NECRODISTAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    NECRODISTAudioProcessorEditor(NECRODISTAudioProcessor&);
    ~NECRODISTAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    NECRODISTAudioProcessor& audioProcessor;
    NecroLookAndFeel laf;

    KnobSlider drive, tone, mix, output;
    juce::Label driveLabel, toneLabel, mixLabel, outputLabel;
    juce::ToggleButton powerButton;
    juce::ComboBox modeBox;

    using SliderAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAtt = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAtt = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    std::unique_ptr<SliderAtt> driveAtt, toneAtt, mixAtt, outputAtt;
    std::unique_ptr<ButtonAtt> powerAtt;
    std::unique_ptr<ComboAtt>  modeAtt;

    void setupKnob(KnobSlider& s, juce::Label& l, const juce::String& name,
        const juce::String& paramID, const juce::String& suffix,
        std::unique_ptr<SliderAtt>& att);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NECRODISTAudioProcessorEditor)
};