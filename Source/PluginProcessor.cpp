#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;

static float parseTime (const String& t)
{
    auto v = t.getFloatValue();
    return (t.containsIgnoreCase ("s") && ! t.containsIgnoreCase ("ms")) ? v * 1000.0f : v;
}

static String fmtTime (float ms)
{
    if (ms >= 1000.0f) return String (ms / 1000.0f, 2) + " s";
    return String (ms, ms < 10.0f ? 1 : 0) + " ms";
}

AudioProcessorValueTreeState::ParameterLayout OptoProcessor::createLayout()
{
    using P = AudioParameterFloat;
    using R = NormalisableRange<float>;
    using A = AudioParameterFloatAttributes;

    AudioProcessorValueTreeState::ParameterLayout l;

    auto dbStr  = [] (float v, int) { return String (v, 1) + " dB"; };
    auto dbVal  = [] (const String& t) { return t.getFloatValue(); };

    l.add (std::make_unique<P> (ParameterID { "output", 1 }, "Output", R (-20.f, 20.f, 0.1f), 0.f,
                                A().withStringFromValueFunction (dbStr).withValueFromStringFunction (dbVal)));

    l.add (std::make_unique<P> (ParameterID { "ratio", 1 }, "Ratio", R (2.f, 10.f, 0.1f), 4.f,
                                A().withStringFromValueFunction ([] (float v, int) { return String (v, 1) + " : 1"; })
                                   .withValueFromStringFunction ([] (const String& t) { return t.getFloatValue(); })));

    l.add (std::make_unique<P> (ParameterID { "threshold", 1 }, "Threshold", R (-40.f, 0.f, 0.1f), -15.f,
                                A().withStringFromValueFunction (dbStr).withValueFromStringFunction (dbVal)));

    l.add (std::make_unique<P> (ParameterID { "attack", 1 }, "Attack", R (0.5f, 300.f, 0.01f, 0.4f), 10.f,
                                A().withStringFromValueFunction ([] (float v, int) { return fmtTime (v); })
                                   .withValueFromStringFunction (parseTime)));

    l.add (std::make_unique<P> (ParameterID { "release", 1 }, "Release", R (50.f, 10000.f, 1.f, 0.35f), 500.f,
                                A().withStringFromValueFunction ([] (float v, int) { return fmtTime (v); })
                                   .withValueFromStringFunction (parseTime)));

    l.add (std::make_unique<P> (ParameterID { "mix", 1 }, "Parallel", R (0.f, 1.f, 0.01f), 1.f,
                                A().withStringFromValueFunction ([] (float v, int) { return String (roundToInt (v * 100.0f)) + " %"; })
                                   .withValueFromStringFunction ([] (const String& t) { return t.getFloatValue() / 100.0f; })));

    l.add (std::make_unique<AudioParameterBool>   (ParameterID { "manual",  1 }, "Attack/Release Manual", true));
    l.add (std::make_unique<AudioParameterBool>   (ParameterID { "vintage", 1 }, "Generation Vintage",    true));
    l.add (std::make_unique<AudioParameterBool>   (ParameterID { "lowcut",  1 }, "Sidechain Low Cut",     false));
    l.add (std::make_unique<AudioParameterChoice> (ParameterID { "meter",   1 }, "Meter",
                                                   StringArray { "Input", "Compression", "Output" }, 1));
    return l;
}

OptoProcessor::OptoProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  AudioChannelSet::stereo(), true)
                        .withOutput ("Output", AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createLayout())
{
}

void OptoProcessor::prepareToPlay (double sampleRate, int)
{
    sr = sampleRate;
    g1 = g2 = 0.0f;
    for (int i = 0; i < 2; ++i)
    {
        dcX[i] = dcY[i] = 0.0f;
        hp[i].coefficients = dsp::IIR::Coefficients<float>::makeHighPass (sr, 90.0f);
        hp[i].reset();
    }
}

bool OptoProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    auto out = l.getMainOutputChannelSet();
    if (out != AudioChannelSet::mono() && out != AudioChannelSet::stereo()) return false;
    return out == l.getMainInputChannelSet();
}

void OptoProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer&)
{
    ScopedNoDenormals noDenormals;

    const int nCh = jmin (buffer.getNumChannels(), 2);
    const int n   = buffer.getNumSamples();
    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, n);

    auto get = [this] (const char* id) { return apvts.getRawParameterValue (id)->load(); };

    const bool manual  = get ("manual")  > 0.5f;
    const bool vintage = get ("vintage") > 0.5f;
    const bool lowcut  = get ("lowcut")  > 0.5f;

    const float outGain = Decibels::decibelsToGain (get ("output"));
    const float attMs   = manual ? get ("attack")  : 12.0f;     // "fixed" = preset times
    const float relMs   = manual ? get ("release") : 450.0f;
    const float ratio   = get ("ratio");
    const float thr     = get ("threshold");
    const float mix     = get ("mix");
    const float tube    = vintage ? 0.30f : 0.04f;

    auto coef = [this] (float ms) { return (float) std::exp (-1.0 / (0.001 * (double) ms * sr)); };

    const float a1 = coef (attMs),        r1 = coef (relMs * 0.25f);
    const float a2 = coef (attMs * 3.0f), r2 = coef (relMs);

    const float slope = 1.0f - 1.0f / ratio;
    const float knee  = 6.0f;

    const float drive = 1.0f + 3.0f * tube;
    const float bias  = 0.12f * tube;
    const float biasT = std::tanh (bias);
    const float dcR   = 0.9995f;

    float* data[2] = { buffer.getWritePointer (0), nCh > 1 ? buffer.getWritePointer (1) : nullptr };
    float grMax = 0.0f, inPeak = 0.0f, outPeak = 0.0f;

    for (int i = 0; i < n; ++i)
    {
        float peak = 0.0f;
        for (int ch = 0; ch < nCh; ++ch)
        {
            const float x = data[ch][i];
            inPeak = jmax (inPeak, std::abs (x));
            float d = hp[ch].processSample (x);
            if (! lowcut) d = x;
            peak = jmax (peak, std::abs (d));
        }

        const float lvl  = Decibels::gainToDecibels (peak, -100.0f);
        const float over = lvl - thr;
        float target = 0.0f;
        if (2.0f * over > knee)            target = slope * over;
        else if (2.0f * over > -knee)      target = slope * square (over + knee * 0.5f) / (2.0f * knee);

        g1 = target > g1 ? a1 * g1 + (1.0f - a1) * target : r1 * g1 + (1.0f - r1) * target;
        g2 = target > g2 ? a2 * g2 + (1.0f - a2) * target : r2 * g2 + (1.0f - r2) * target;
        const float gr = 0.6f * g1 + 0.4f * g2;
        grMax = jmax (grMax, gr);

        const float grGain = Decibels::decibelsToGain (-gr);

        for (int ch = 0; ch < nCh; ++ch)
        {
            const float dry = data[ch][i];
            float wet = dry * grGain;

            if (tube > 0.0f)
            {
                const float sat  = (std::tanh (drive * wet + bias) - biasT) / drive;
                const float diff = sat - wet;
                const float y    = diff - dcX[ch] + dcR * dcY[ch];
                dcX[ch] = diff; dcY[ch] = y;
                wet += tube * y;
            }

            const float o = (dry * (1.0f - mix) + wet * mix) * outGain;
            data[ch][i] = o;
            outPeak = jmax (outPeak, std::abs (o));
        }
    }

    grMeter.store (grMax);
    inMeter.store (inPeak);
    outMeter.store (outPeak);
}

AudioProcessorEditor* OptoProcessor::createEditor() { return new OptoEditor (*this); }

void OptoProcessor::getStateInformation (MemoryBlock& dest)
{
    if (auto xml = apvts.copyState().createXml()) copyXmlToBinary (*xml, dest);
}

void OptoProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (ValueTree::fromXml (*xml));
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new OptoProcessor(); }
