// OpenKeys Performance screen, Phase 0 draft with DUMMY data (no engine yet).
// Rule D020: everything on ONE screen, no scrolling. Own names and look, no Yamaha/Roland artwork or layout.
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <array>
#include <cmath>
#include <vector>

namespace okui {

inline const juce::Colour kBg(0xff0e1217), kPanel(0xff171d25), kAccent(0xff4fc3f7),
                          kText(0xffe6edf3), kDim(0xff7d8a99), kDanger(0xffef5350), kScene(0xffffb74d);

// Buttons that glow when lit.
class OkLook : public juce::LookAndFeel_V4 {
public:
    void drawButtonBackground(juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down) override {
        auto r = b.getLocalBounds().toFloat().reduced(2.f);
        const bool lit = b.getToggleState();
        auto col = lit ? b.findColour(juce::TextButton::buttonOnColourId) : b.findColour(juce::TextButton::buttonColourId);
        if (lit) {
            juce::Path p;
            p.addRoundedRectangle(r, 6.f);
            juce::DropShadow(col.withAlpha(0.7f), 12, {0, 0}).drawForPath(g, p);
        }
        g.setColour(col.brighter(down ? 0.2f : over ? 0.08f : 0.f));
        g.fillRoundedRectangle(r, 6.f);
        g.setColour(lit ? col.brighter(0.4f) : kDim.withAlpha(0.35f));
        g.drawRoundedRectangle(r, 6.f, 1.f);
    }
    void drawButtonText(juce::Graphics& g, juce::TextButton& b, bool, bool) override {
        g.setColour(b.getToggleState() ? kBg : kText);
        g.setFont(14.f);
        g.drawText(b.getButtonText(), b.getLocalBounds(), juce::Justification::centred);
    }
};

// Fader or knob that glides to a target value (scene recall). Grabbing it cancels the glide.
class GlideSlider : public juce::Slider {
public:
    explicit GlideSlider(SliderStyle s = LinearVertical) : juce::Slider(s, NoTextBox) {
        setRange(0.0, 1.0, 0.001);
        setColour(trackColourId, kAccent);
        setColour(thumbColourId, kText);
        setColour(backgroundColourId, kPanel.darker(0.3f));
        setColour(rotarySliderFillColourId, kAccent);
        setColour(rotarySliderOutlineColourId, kPanel.brighter(0.2f));
        onDragStart = [this] { gliding = false; };
    }
    void glideTo(double v) { target = v; gliding = true; }
    bool step() {
        if (!gliding) return false;
        const double cur = getValue(), d = target - cur;
        if (std::abs(d) < 0.002) { setValue(target, juce::dontSendNotification); gliding = false; return false; }
        setValue(cur + d * 0.22, juce::dontSendNotification);
        return true;
    }
private:
    double target = 0.0;
    bool gliding = false;
};

struct SceneState {
    std::array<bool, 16> part{};
    std::array<double, 8> fader{};
    std::array<double, 4> knob{};
};

class PerformanceScreen : public juce::Component, private juce::Timer, private juce::ListBoxModel {
public:
    PerformanceScreen() {
        setLookAndFeel(&look);
        setSize(1280, 720);

        title.setText("OpenKeys", juce::dontSendNotification);
        title.setFont(juce::Font(juce::FontOptions(28.f, juce::Font::bold)));
        title.setColour(juce::Label::textColourId, kAccent);
        status.setJustificationType(juce::Justification::centredRight);
        status.setColour(juce::Label::textColourId, kDim);
        addAndMakeVisible(title);
        addAndMakeVisible(status);

        panic.setButtonText("PANIC");
        panic.setColour(juce::TextButton::buttonColourId, kDanger.darker(0.3f));
        panic.onClick = [this] { status.setText("PANIC: all notes off (dummy, engine not connected)", juce::dontSendNotification); };
        addAndMakeVisible(panic);

        auto setup = [this](juce::TextButton& b, const juce::String& text, juce::Colour on, int radio) {
            b.setButtonText(text);
            b.setColour(juce::TextButton::buttonColourId, kPanel.brighter(0.15f));
            b.setColour(juce::TextButton::buttonOnColourId, on);
            b.setClickingTogglesState(true);
            if (radio != 0) b.setRadioGroupId(radio);
            addAndMakeVisible(b);
        };

        for (int i = 0; i < 8; ++i) {
            setup(catBtn[(size_t) i], categories[(size_t) i], kAccent, 1);
            catBtn[(size_t) i].onClick = [this, i] { curCat = i; patchList.updateContent(); patchList.selectRow(0); };
        }
        catBtn[0].setToggleState(true, juce::dontSendNotification);

        for (int i = 0; i < 16; ++i) {
            setup(partBtn[(size_t) i], "Part " + juce::String(i + 1), juce::Colour::fromHSV((float) i / 16.f, 0.55f, 0.95f, 1.f), 0);
            partBtn[(size_t) i].onClick = [this, i] {
                status.setText("Part " + juce::String(i + 1) + (partBtn[(size_t) i].getToggleState() ? " on" : " off"), juce::dontSendNotification);
            };
        }
        for (int i = 0; i < 8; ++i) {
            setup(sceneBtn[(size_t) i], sceneNames[(size_t) i], kScene, 2);
            sceneBtn[(size_t) i].onClick = [this, i] { recallScene(i); };
        }

        patchList.setModel(this);
        patchList.setRowHeight(30);
        patchList.setColour(juce::ListBox::backgroundColourId, kPanel.darker(0.25f));
        addAndMakeVisible(patchList);

        for (int i = 0; i < 8; ++i) {
            addAndMakeVisible(faders[(size_t) i]);
            faderLabels[(size_t) i].setText("P" + juce::String(i + 1), juce::dontSendNotification);
            faderLabels[(size_t) i].setJustificationType(juce::Justification::centred);
            faderLabels[(size_t) i].setColour(juce::Label::textColourId, kDim);
            addAndMakeVisible(faderLabels[(size_t) i]);
        }
        const char* knobNames[4] = { "Reverb", "Delay", "Chorus", "Tone" };
        for (int i = 0; i < 4; ++i) {
            knobs[(size_t) i].setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            addAndMakeVisible(knobs[(size_t) i]);
            knobLabels[(size_t) i].setText(knobNames[i], juce::dontSendNotification);
            knobLabels[(size_t) i].setJustificationType(juce::Justification::centred);
            knobLabels[(size_t) i].setColour(juce::Label::textColourId, kDim);
            addAndMakeVisible(knobLabels[(size_t) i]);
        }
        masterKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        masterKnob.setValue(0.5);
        masterKnob.setColour(juce::Slider::rotarySliderFillColourId, kScene);
        masterLabel.setText("MASTER KNOB", juce::dontSendNotification);
        masterLabel.setJustificationType(juce::Justification::centred);
        masterLabel.setColour(juce::Label::textColourId, kScene);
        addAndMakeVisible(masterKnob);
        addAndMakeVisible(masterLabel);

        buildDummyScenes();
        sceneBtn[0].setToggleState(true, juce::dontSendNotification);
        recallScene(0);
        patchList.selectRow(0);
    }

    ~PerformanceScreen() override { setLookAndFeel(nullptr); }

    void paint(juce::Graphics& g) override {
        g.fillAll(kBg);
        g.setColour(kPanel);
        for (auto* a : { &patchArea, &partsArea, &mixArea }) g.fillRoundedRectangle(a->toFloat(), 10.f);
        g.setColour(kDim);
        g.setFont(12.f);
        g.drawText("SOUNDS", lblSounds, juce::Justification::centredLeft);
        g.drawText("PARTS", lblParts, juce::Justification::centredLeft);
        g.drawText("SCENES", lblScenes, juce::Justification::centredLeft);
        g.drawText("MIXER", lblMix, juce::Justification::centredLeft);
    }

    void resized() override {
        auto r = getLocalBounds().reduced(12);
        auto top = r.removeFromTop(48);
        panic.setBounds(top.removeFromRight(120).reduced(0, 6));
        title.setBounds(top.removeFromLeft(220));
        status.setBounds(top);
        r.removeFromTop(8);

        patchArea = r.removeFromLeft(r.getWidth() * 28 / 100);
        r.removeFromLeft(12);
        auto pa = patchArea.reduced(10);
        lblSounds = pa.removeFromTop(18);
        auto cats = pa.removeFromTop(84);
        for (int i = 0; i < 8; ++i) {
            const int cw = cats.getWidth() / 4, ch = cats.getHeight() / 2;
            catBtn[(size_t) i].setBounds(cats.getX() + (i % 4) * cw, cats.getY() + (i / 4) * ch, cw, ch);
        }
        pa.removeFromTop(8);
        patchList.setBounds(pa);

        partsArea = r.removeFromTop((int) (r.getHeight() * 0.36f));
        r.removeFromTop(12);
        mixArea = r;

        auto pp = partsArea.reduced(10);
        lblParts = pp.removeFromTop(18);
        auto rows = pp.removeFromTop(pp.getHeight() * 52 / 100);
        for (int i = 0; i < 16; ++i) {
            const int cw = rows.getWidth() / 8, ch = rows.getHeight() / 2;
            partBtn[(size_t) i].setBounds(rows.getX() + (i % 8) * cw, rows.getY() + (i / 8) * ch, cw, ch);
        }
        lblScenes = pp.removeFromTop(18);
        auto sc = pp.removeFromTop(juce::jmin(pp.getHeight(), 56));
        for (int i = 0; i < 8; ++i) {
            const int cw = sc.getWidth() / 8;
            sceneBtn[(size_t) i].setBounds(sc.getX() + i * cw, sc.getY(), cw, sc.getHeight());
        }

        auto mp = mixArea.reduced(10);
        lblMix = mp.removeFromTop(18);
        const int mw = mp.getWidth();
        auto fa = mp.removeFromLeft(mw * 56 / 100);
        for (int i = 0; i < 8; ++i) {
            auto cell = fa.removeFromLeft(fa.getWidth() / (8 - i));
            faderLabels[(size_t) i].setBounds(cell.removeFromBottom(20));
            faders[(size_t) i].setBounds(cell.reduced(juce::jmax(2, cell.getWidth() / 5), 4));
        }
        auto ka = mp.removeFromLeft(mw * 24 / 100);
        for (int i = 0; i < 4; ++i) {
            const int cw = ka.getWidth() / 2, ch = ka.getHeight() / 2;
            juce::Rectangle<int> cell(ka.getX() + (i % 2) * cw, ka.getY() + (i / 2) * ch, cw, ch);
            knobLabels[(size_t) i].setBounds(cell.removeFromBottom(18));
            knobs[(size_t) i].setBounds(cell.reduced(4));
        }
        masterLabel.setBounds(mp.removeFromBottom(22));
        masterKnob.setBounds(mp.reduced(6));
    }

private:
    // ---- dummy data ----
    const std::array<juce::String, 8> categories { "Piano", "Keys", "Strings", "Pads", "Brass", "Organ", "Synth", "Church" };
    const std::array<juce::String, 8> sceneNames { "Intro", "Verse", "Chorus", "Bridge", "Pad Only", "Solo", "Outro", "Blank" };
    const std::array<juce::StringArray, 8> patches {{
        { "Concert Grand", "Bright Grand", "Warm Grand", "Upright", "Honky Tonk", "Studio Grand" },
        { "Tine EP", "Reed EP", "Clav", "Harpsichord", "Vibes", "Celesta" },
        { "String Ensemble", "Slow Strings", "Pizzicato", "Tremolo", "Cello Solo", "Violin Solo" },
        { "Warm Pad", "Glass Pad", "Choir Pad", "Air Pad", "Dark Pad", "Shimmer" },
        { "Brass Section", "Trumpet", "Trombone", "French Horn", "Tuba", "Soft Brass" },
        { "Drawbar Full", "Gospel Organ", "Jazz Organ", "Church Organ", "Rotary Slow", "Rotary Fast" },
        { "Saw Lead", "Square Lead", "Sub Bass", "Pluck", "Arp Seq", "Soft Lead" },
        { "Sunday Worship", "Hymn", "Praise Band", "Altar Pad", "Offering", "Closing" } }};

    void buildDummyScenes() {
        scenes.clear();
        for (int s = 0; s < 8; ++s) {
            SceneState st;
            for (int i = 0; i < 16; ++i) st.part[(size_t) i] = (s != 7) && ((s + i) % 3 != 0) && i < 8 + s;
            for (int i = 0; i < 8; ++i) st.fader[(size_t) i] = 0.2 + 0.7 * std::abs(std::sin(0.9 * (s + 1) * (i + 1)));
            for (int i = 0; i < 4; ++i) st.knob[(size_t) i] = 0.15 + 0.7 * std::abs(std::sin(1.3 * (s + 2) + i));
            scenes.push_back(st);
        }
    }

    void recallScene(int s) {
        const auto& st = scenes[(size_t) s];
        for (int i = 0; i < 16; ++i) partBtn[(size_t) i].setToggleState(st.part[(size_t) i], juce::dontSendNotification);
        for (int i = 0; i < 8; ++i) faders[(size_t) i].glideTo(st.fader[(size_t) i]);
        for (int i = 0; i < 4; ++i) knobs[(size_t) i].glideTo(st.knob[(size_t) i]);
        status.setText("Scene: " + sceneNames[(size_t) s], juce::dontSendNotification);
        startTimerHz(60);
    }

    void timerCallback() override {
        bool moving = false;
        for (auto& f : faders) moving |= f.step();
        for (auto& k : knobs) moving |= k.step();
        if (!moving) stopTimer();
    }

    // ---- patch list ----
    int getNumRows() override { return patches[(size_t) curCat].size(); }
    void paintListBoxItem(int row, juce::Graphics& g, int w, int h, bool selected) override {
        if (selected) { g.setColour(kAccent.withAlpha(0.25f)); g.fillRect(0, 0, w, h); }
        g.setColour(selected ? kText : kDim);
        g.setFont(16.f);
        g.drawText(patches[(size_t) curCat][row], 12, 0, w - 12, h, juce::Justification::centredLeft);
    }
    void selectedRowsChanged(int row) override {
        if (row >= 0 && row < patches[(size_t) curCat].size())
            status.setText(categories[(size_t) curCat] + ": " + patches[(size_t) curCat][row], juce::dontSendNotification);
    }

    OkLook look;
    juce::Label title, status, masterLabel;
    juce::TextButton panic;
    std::array<juce::TextButton, 8> catBtn;
    std::array<juce::TextButton, 16> partBtn;
    std::array<juce::TextButton, 8> sceneBtn;
    juce::ListBox patchList { "patches", nullptr };
    std::array<GlideSlider, 8> faders;
    std::array<juce::Label, 8> faderLabels;
    std::array<GlideSlider, 4> knobs;
    std::array<juce::Label, 4> knobLabels;
    GlideSlider masterKnob { juce::Slider::RotaryHorizontalVerticalDrag };
    std::vector<SceneState> scenes;
    int curCat = 0;
    juce::Rectangle<int> patchArea, partsArea, mixArea, lblSounds, lblParts, lblScenes, lblMix;
};

}  // namespace okui
