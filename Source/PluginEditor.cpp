#include "PluginEditor.h"

using namespace juce;

static const Colour kPanelTop (0xff1a566f);
static const Colour kPanelBot (0xff0e3a50);
static const Colour kText     (0xffeae6d6);
static const Colour kMetal    (0xffc9ccd1);

//==============================================================================
OptoLookAndFeel::OptoLookAndFeel()
{
    setColour (Slider::textBoxTextColourId,       kText);
    setColour (Slider::textBoxOutlineColourId,    Colours::transparentBlack);
    setColour (Slider::textBoxBackgroundColourId, Colours::transparentBlack);
    setColour (Slider::textBoxHighlightColourId,  Colour (0x66000000));
    setColour (Label::textColourId,               kText);
}

void OptoLookAndFeel::drawRotarySlider (Graphics& g, int x, int y, int w, int h,
                                        float pos, float startA, float endA, Slider& s)
{
    const bool mini = (bool) s.getProperties()["mini"];
    auto b = Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (2.0f);
    const float r  = jmin (b.getWidth(), b.getHeight()) * 0.5f;
    const auto  c  = b.getCentre();
    const float kr = mini ? r * 0.72f : r * 0.62f;
    const float ang = startA + pos * (endA - startA);

    auto pt = [&] (float a, float rad) { return Point<float> (c.x + std::sin (a) * rad, c.y - std::cos (a) * rad); };

    g.setColour (kText);
    if (mini)
    {
        for (int i = 0; i < 3; ++i)
        {
            const float a = startA + (float) i * 0.5f * (endA - startA);
            g.drawLine (Line<float> (pt (a, r * 0.80f), pt (a, r * 0.95f)), 2.0f);
        }
    }
    else
    {
        // minor ticks
        for (int i = 0; i <= 20; ++i)
        {
            const float a = startA + (float) i / 20.0f * (endA - startA);
            g.drawLine (Line<float> (pt (a, r * 0.74f), pt (a, r * 0.80f)), 1.0f);
        }

        // labelled ticks
        StringArray tv, tl;
        tv.addTokens (s.getProperties()["ticks"].toString(),  ",", "");
        tl.addTokens (s.getProperties()["labels"].toString(), "|", "");
        g.setFont (Font (FontOptions (jmax (8.0f, r * 0.15f), Font::bold)));
        for (int i = 0; i < tv.size(); ++i)
        {
            const float p = (float) s.valueToProportionOfLength (tv[i].getDoubleValue());
            const float a = startA + p * (endA - startA);
            g.drawLine (Line<float> (pt (a, r * 0.72f), pt (a, r * 0.86f)), 2.0f);
            const String txt = i < tl.size() ? tl[i] : tv[i];
            g.drawText (txt, Rectangle<float> (36, 12).withCentre (pt (a, r * 0.98f)), Justification::centred, false);
        }
    }

    // shadow
    g.setColour (Colour (0x66000000));
    g.fillEllipse (Rectangle<float> (kr * 2, kr * 2).withCentre (c.translated (2.0f, 3.0f)));

    // skirt
    g.setGradientFill (ColourGradient (Colour (0xff55524d), c.x - kr, c.y - kr,
                                       Colour (0xff0b0a09), c.x + kr, c.y + kr, false));
    g.fillEllipse (Rectangle<float> (kr * 2, kr * 2).withCentre (c));
    g.setColour (Colour (0x77000000));
    for (int i = 0; i < 60; ++i)
    {
        const float a = MathConstants<float>::twoPi * (float) i / 60.0f;
        g.drawLine (Line<float> (pt (a, kr * 0.9f), pt (a, kr)), 1.0f);
    }

    // cap
    const float cr = kr * 0.82f;
    g.setGradientFill (ColourGradient (Colour (0xff3d3a36), c.x - cr, c.y - cr,
                                       Colour (0xff121110), c.x + cr, c.y + cr, false));
    g.fillEllipse (Rectangle<float> (cr * 2, cr * 2).withCentre (c));
    g.setColour (Colour (0x28ffffff));
    g.drawEllipse (Rectangle<float> (cr * 2, cr * 2).withCentre (c), 1.0f);

    // pointer
    g.setColour (Colour (0xfff7f2e4));
    Path p;
    p.addRoundedRectangle (-1.9f, -cr * 0.94f, 3.8f, cr * 0.55f, 1.6f);
    g.fillPath (p, AffineTransform::rotation (ang).translated (c.x, c.y));
}

void OptoLookAndFeel::drawToggleButton (Graphics& g, ToggleButton& btn, bool, bool)
{
    auto b = btn.getLocalBounds().toFloat();
    auto upR   = b.removeFromTop (16.0f);
    auto downR = b.removeFromBottom (16.0f);

    g.setColour (kText);
    g.setFont (Font (FontOptions (11.0f, Font::bold)));
    g.drawText (btn.getProperties()["up"].toString(),   upR,   Justification::centred, false);
    g.drawText (btn.getProperties()["down"].toString(), downR, Justification::centred, false);

    const auto c = b.getCentre();
    const bool on = btn.getToggleState();

    g.setColour (Colour (0x66000000));
    g.fillEllipse (Rectangle<float> (34, 34).withCentre (c.translated (1.5f, 2.5f)));
    g.setGradientFill (ColourGradient (Colour (0xffe6e8ec), c.x - 15, c.y - 15,
                                       Colour (0xff7d8088), c.x + 15, c.y + 15, false));
    g.fillEllipse (Rectangle<float> (32, 32).withCentre (c));
    g.setColour (Colour (0xff23262b));
    g.fillEllipse (Rectangle<float> (14, 14).withCentre (c));

    const Point<float> tip (c.x, c.y + (on ? -21.0f : 21.0f));
    Path lever;
    lever.startNewSubPath (c);
    lever.lineTo (tip);
    g.setColour (Colour (0x66000000));
    g.strokePath (lever, PathStrokeType (8.0f, PathStrokeType::curved, PathStrokeType::rounded),
                  AffineTransform::translation (1.5f, 2.0f));
    g.setColour (kMetal);
    g.strokePath (lever, PathStrokeType (7.0f, PathStrokeType::curved, PathStrokeType::rounded));
    g.setGradientFill (ColourGradient (Colours::white, tip.x - 4, tip.y - 4, Colour (0xff8b8f97), tip.x + 4, tip.y + 4, false));
    g.fillEllipse (Rectangle<float> (11, 11).withCentre (tip));
}

//==============================================================================
OptoEditor::OptoEditor (OptoProcessor& p) : AudioProcessorEditor (&p), proc (p)
{
    setLookAndFeel (&laf);

    setupKnob (outK, "OUTPUT",    "output",    "-20,-10,0,10,20",      "");
    setupKnob (raK,  "RATIO",     "ratio",     "2,4,6,8,10",           "2:1|4|6|8|10:1");
    setupKnob (thK,  "THRESHOLD", "threshold", "-40,-30,-20,-10,0",   "");
    setupKnob (atK,  "ATTACK",    "attack",    "0.5,300",              "fast|slow");
    setupKnob (reK,  "RELEASE",   "release",   "50,10000",             "fast|slow");
    setupKnob (mxK,  "PARALLEL",  "mix",       "0,0.5,1",              "dry|50|wet");

    setupSwitch (arS,  "ATT / REL",  "manual",  "manual", "fixed");
    setupSwitch (genS, "GENERATION", "vintage", "vintage", "modern");
    setupSwitch (lcS,  "SC LOW CUT", "lowcut",  "90 Hz",  "off");

    meterSel.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    meterSel.setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
    meterSel.setRotaryParameters (MathConstants<float>::twoPi - 0.87f, MathConstants<float>::twoPi + 0.87f, true);
    meterSel.getProperties().set ("mini", true);
    addAndMakeVisible (meterSel);
    sAtt.push_back (std::make_unique<SA> (proc.apvts, "meter", meterSel));
    setupCaption (meterCaption, "METER");

    setSize (900, 390);
    startTimerHz (30);
}

OptoEditor::~OptoEditor() { setLookAndFeel (nullptr); }

void OptoEditor::setupCaption (Label& l, const String& text)
{
    l.setText (text, dontSendNotification);
    l.setJustificationType (Justification::centred);
    l.setFont (Font (FontOptions (12.0f, Font::bold)));
    addAndMakeVisible (l);
}

void OptoEditor::setupKnob (Knob& k, const String& caption, const String& id,
                            const String& ticks, const String& labels)
{
    k.slider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    k.slider.setTextBoxStyle (Slider::TextBoxBelow, false, 90, 18);
    k.slider.setRotaryParameters (MathConstants<float>::pi * 1.25f, MathConstants<float>::pi * 2.75f, true);
    k.slider.getProperties().set ("ticks",  ticks);
    k.slider.getProperties().set ("labels", labels);
    addAndMakeVisible (k.slider);
    setupCaption (k.label, caption);
    sAtt.push_back (std::make_unique<SA> (proc.apvts, id, k.slider));
}

void OptoEditor::setupSwitch (Switch& s, const String& caption, const String& id,
                              const String& up, const String& down)
{
    s.button.getProperties().set ("up", up);
    s.button.getProperties().set ("down", down);
    addAndMakeVisible (s.button);
    setupCaption (s.label, caption);
    bAtt.push_back (std::make_unique<BA> (proc.apvts, id, s.button));
}

void OptoEditor::resized()
{
    auto placeKnob = [] (Knob& k, int x, int y)
    {
        k.slider.setBounds (x, y, 120, 130);
        k.label.setBounds  (x, y + 130, 120, 16);
    };
    auto placeSwitch = [] (Switch& s, int x, int y)
    {
        s.button.setBounds (x, y, 80, 100);
        s.label.setBounds  (x - 10, y + 102, 100, 16);
    };

    placeKnob (outK, 40, 52);
    placeKnob (raK, 180, 52);
    placeKnob (thK, 320, 52);

    meterSel.setBounds (500, 70, 90, 90);
    meterCaption.setBounds (480, 168, 130, 16);
    meterArea = { 630, 58, 240, 140 };

    placeKnob (atK, 40, 212);
    placeKnob (reK, 180, 212);
    placeSwitch (arS,  340, 222);
    placeSwitch (genS, 440, 222);
    placeSwitch (lcS,  540, 222);
    placeKnob (mxK, 660, 212);
    lampPos = { 835.0f, 300.0f };
}

void OptoEditor::timerCallback()
{
    auto toDb = [] (float lin) { return jmax (-60.0f, Decibels::gainToDecibels (lin, -60.0f)); };
    dispGr  += 0.25f * (proc.grMeter.load() - dispGr);
    dispIn  += 0.25f * (toDb (proc.inMeter.load())  - dispIn);
    dispOut += 0.25f * (toDb (proc.outMeter.load()) - dispOut);
    repaint (meterArea.expanded (8));
    repaint ((int) lampPos.x - 30, (int) lampPos.y - 30, 60, 60);
}

void OptoEditor::drawMeter (Graphics& g, Rectangle<float> r)
{
    const int mode = (int) proc.apvts.getRawParameterValue ("meter")->load();   // 0 in, 1 gr, 2 out

    g.setColour (Colour (0xff15130f));
    g.fillRoundedRectangle (r.expanded (6), 8.0f);
    g.setGradientFill (ColourGradient (Colour (0xfffbefc4), r.getX(), r.getY(),
                                       Colour (0xffe0cd8f), r.getX(), r.getBottom(), false));
    g.fillRoundedRectangle (r, 5.0f);

    const Point<float> pivot (r.getCentreX(), r.getBottom() + r.getHeight() * 0.22f);
    const float rad   = r.getHeight() * 1.02f;
    const float sweep = degreesToRadians (46.0f);
    auto angFor = [&] (float t) { return -sweep + jlimit (0.0f, 1.0f, t) * 2.0f * sweep; };
    auto pt = [&] (float a, float rr) { return Point<float> (pivot.x + std::sin (a) * rr, pivot.y - std::cos (a) * rr); };

    Graphics::ScopedSaveState ss (g);
    g.reduceClipRegion (r.toNearestInt());

    const Colour ink (0xff2a2622), red (0xffb3261e);
    g.setFont (Font (FontOptions (11.0f, Font::bold)));

    if (mode == 1)   // gain reduction 0..20 dB
    {
        for (int db = 0; db <= 20; ++db)
        {
            const float a = angFor ((float) db / 20.0f);
            const bool major = db % 5 == 0;
            g.setColour (db >= 15 ? red : ink);
            g.drawLine (Line<float> (pt (a, rad), pt (a, rad - (major ? 12.0f : 6.0f))), major ? 1.8f : 1.0f);
            if (major) g.drawText (String (db), Rectangle<float> (24, 12).withCentre (pt (a, rad - 22.0f)), Justification::centred, false);
        }
    }
    else             // VU -20..+3 (0 VU = -18 dBFS)
    {
        for (int v = -20; v <= 3; ++v)
        {
            const float a = angFor ((float) (v + 20) / 23.0f);
            const bool major = (v == -20 || v == -10 || v == -7 || v == -5 || v == -3 || v == 0 || v == 3);
            g.setColour (v >= 0 ? red : ink);
            g.drawLine (Line<float> (pt (a, rad), pt (a, rad - (major ? 12.0f : 5.0f))), major ? 1.8f : 1.0f);
            if (major) g.drawText (v > 0 ? "+" + String (v) : String (v),
                                   Rectangle<float> (26, 12).withCentre (pt (a, rad - 22.0f)), Justification::centred, false);
        }
    }

    g.setColour (ink);
    g.setFont (Font (FontOptions (10.0f, Font::bold)));
    const char* names[] = { "INPUT   VU", "GAIN REDUCTION   dB", "OUTPUT   VU" };
    g.drawText (names[jlimit (0, 2, mode)], r.removeFromBottom (24).reduced (0, 4), Justification::centred, false);

    float t = 0.0f;
    if (mode == 1)      t = dispGr / 20.0f;
    else if (mode == 0) t = ((dispIn  + 18.0f) + 20.0f) / 23.0f;
    else                t = ((dispOut + 18.0f) + 20.0f) / 23.0f;

    const float a = angFor (t);
    g.setColour (Colour (0x44000000));
    g.drawLine (Line<float> (pt (a, rad * 0.35f).translated (2, 2), pt (a, rad - 4.0f).translated (2, 2)), 1.8f);
    g.setColour (red);
    g.drawLine (Line<float> (pt (a, rad * 0.35f), pt (a, rad - 4.0f)), 1.8f);
}

void OptoEditor::paint (Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    g.setGradientFill (ColourGradient (kPanelTop, 0, 0, kPanelBot, 0, b.getHeight(), false));
    g.fillAll();

    // fine horizontal brushing
    g.setColour (Colour (0x0cffffff));
    for (int y = 0; y < getHeight(); y += 3) g.drawHorizontalLine (y, 0.0f, b.getWidth());

    g.setColour (Colour (0x55000000));
    g.drawRect (b.reduced (6), 2.0f);

    for (auto p : { Point<float> (20, 20), Point<float> (b.getWidth() - 20, 20),
                    Point<float> (20, b.getHeight() - 20), Point<float> (b.getWidth() - 20, b.getHeight() - 20) })
    {
        g.setColour (Colour (0xff9aa0a8));
        g.fillEllipse (Rectangle<float> (12, 12).withCentre (p));
        g.setColour (Colour (0xff2c3036));
        g.drawLine (p.x - 4, p.y - 2, p.x + 4, p.y + 2, 1.5f);
    }

    g.setColour (kText);
    g.setFont (Font (FontOptions (22.0f, Font::bold)));
    g.drawText ("OPTO-1B", 36, 14, 200, 26, Justification::left);
    g.setFont (Font (FontOptions (10.0f, Font::bold)));
    g.drawText ("OPTICAL COMPRESSOR", 38, 38, 200, 14, Justification::left);

    // meter selector position labels
    const Point<float> mc = meterSel.getBounds().toFloat().getCentre();
    g.setFont (Font (FontOptions (10.0f, Font::bold)));
    g.drawText ("INPUT",  Rectangle<float> (50, 12).withCentre (mc.translated (-46, -34)), Justification::centred, false);
    g.drawText ("COMP",   Rectangle<float> (50, 12).withCentre (mc.translated (0,   -56)), Justification::centred, false);
    g.drawText ("OUTPUT", Rectangle<float> (50, 12).withCentre (mc.translated (46,  -34)), Justification::centred, false);

    drawMeter (g, meterArea.toFloat());

    // compression lamp
    const float inten = jlimit (0.0f, 1.0f, dispGr / 4.0f);
    g.setColour (Colour (0xff160e0d));
    g.fillEllipse (Rectangle<float> (40, 40).withCentre (lampPos));
    g.setColour (Colour::fromFloatRGBA (1.0f, 0.15f, 0.1f, 0.25f * inten));
    g.fillEllipse (Rectangle<float> (60, 60).withCentre (lampPos));
    g.setGradientFill (ColourGradient (Colour::fromFloatRGBA (1.0f, 0.55f, 0.45f, 0.35f + 0.65f * inten),
                                       lampPos.x - 4, lampPos.y - 4,
                                       Colour::fromFloatRGBA (0.45f, 0.02f, 0.02f, 1.0f),
                                       lampPos.x + 14, lampPos.y + 14, true));
    g.fillEllipse (Rectangle<float> (28, 28).withCentre (lampPos));
}
