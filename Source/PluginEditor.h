#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

// Original vector artwork: knobs, lever switches and VU meter drawn in code.
class OptoLookAndFeel : public juce::LookAndFeel_V4
{
public:
    OptoLookAndFeel();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h,
                           float pos, float startAngle, float endAngle, juce::Slider&) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool, bool) override;
};

class OptoEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit OptoEditor (OptoProcessor&);
    ~OptoEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    struct Knob   { juce::Slider slider; juce::Label label; };
    struct Switch { juce::ToggleButton button; juce::Label label; };

    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;

    void timerCallback() override;
    void setupKnob (Knob&, const juce::String& caption, const juce::String& id,
                    const juce::String& ticks, const juce::String& labels);
    void setupSwitch (Switch&, const juce::String& caption, const juce::String& id,
                      const juce::String& up, const juce::String& down);
    void setupCaption (juce::Label&, const juce::String&);
    void drawMeter (juce::Graphics&, juce::Rectangle<float>);

    OptoProcessor& proc;
    OptoLookAndFeel laf;

    Knob outK, raK, thK, atK, reK, mxK;
    Slider meterSel;
    juce::Label meterCaption;
    Switch arS, genS, lcS;
    std::vector<std::unique_ptr<SA>> sAtt;
    std::vector<std::unique_ptr<BA>> bAtt;

    juce::Rectangle<int> meterArea;
    juce::Point<float> lampPos { 840.0f, 290.0f };
    float dispGr = 0.0f, dispIn = -60.0f, dispOut = -60.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OptoEditor)
};
