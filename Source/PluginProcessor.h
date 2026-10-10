#pragma once
#include <JuceHeader.h>

class OptoProcessor : public juce::AudioProcessor
{
public:
    OptoProcessor();
    ~OptoProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "OPTO-1B"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;
    std::atomic<float> grMeter  { 0.0f };   // gain reduction, dB (positive)
    std::atomic<float> inMeter  { 0.0f };   // block peak, linear
    std::atomic<float> outMeter { 0.0f };   // block peak, linear

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    double sr = 44100.0;
    float g1 = 0.0f, g2 = 0.0f;
    float dcX[2] { 0, 0 }, dcY[2] { 0, 0 };
    juce::dsp::IIR::Filter<float> hp[2];     // sidechain low cut

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OptoProcessor)
};
