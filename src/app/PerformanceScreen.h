// OpenKeys Performance screen, Phase 0 draft with DUMMY data (no engine yet).
// Rule D020: everything on ONE screen, no scrolling. Own names and look, no Yamaha/Roland artwork or layout.
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <array>
#include <cmath>
#include <vector>

namespace okui {

inline const juce::Colour kBg(0xff0a0d11), kAccent(0xff4fc3f7), kText(0xffe9eff5), kDim(0xff8896a5),
                          kDanger(0xffef5350), kScene(0xffffb74d), kLed(0xff5dff8f);

inline juce::Colour layerColour(int i) { return juce::Colour::fromHSV((float) i / 16.f, 0.55f, 0.95f, 1.f); }

inline void drawPanel(juce::Graphics& g, juce::Rectangle<float> r, float radius = 12.f) {
    juce::Path p;
    p.addRoundedRectangle(r, radius);
    juce::DropShadow(juce::Colours::black.withAlpha(0.7f), 10, {0, 4}).drawForPath(g, p);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff252e39), r.getX(), r.getY(),
                                           juce::Colour(0xff11161c), r.getX(), r.getBottom(), false));
    g.fillRoundedRectangle(r, radius);
    g.setColour(juce::Colour(0xff4a5868).withAlpha(0.7f));
    g.drawRoundedRectangle(r.reduced(0.5f), radius, 1.f);
    g.setColour(juce::Colours::white.withAlpha(0.07f));
    g.drawLine(r.getX() + radius, r.getY() + 1.5f, r.getRight() - radius, r.getY() + 1.5f, 1.f);
}

// 3D hardware-style controls.
class OkLook : public juce::LookAndFeel_V4 {
public:
    void drawButtonBackground(juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down) override {
        auto r = b.getLocalBounds().toFloat().reduced(2.f);
        const bool lit = b.getToggleState();
        auto base = lit ? b.findColour(juce::TextButton::buttonOnColourId) : b.findColour(juce::TextButton::buttonColourId);
        juce::Path p;
        p.addRoundedRectangle(r, 7.f);
        if (lit) juce::DropShadow(base.withAlpha(0.7f), 14, {0, 0}).drawForPath(g, p);
        else juce::DropShadow(juce::Colours::black.withAlpha(0.7f), 6, {0, 3}).drawForPath(g, p);
        const float lift = over ? 0.08f : 0.f;
        auto top = base.brighter(0.35f + lift), bottom = base.darker(0.5f);
        if (down) std::swap(top, bottom);
        g.setGradientFill(juce::ColourGradient(top, r.getX(), r.getY(), bottom, r.getX(), r.getBottom(), false));
        g.fillRoundedRectangle(r, 7.f);
        g.setColour(juce::Colours::white.withAlpha(lit ? 0.5f : 0.18f));
        g.drawLine(r.getX() + 7.f, r.getY() + 1.5f, r.getRight() - 7.f, r.getY() + 1.5f, 1.f);
        g.setColour(base.darker(0.9f));
        g.drawRoundedRectangle(r, 7.f, 1.f);
    }
    void drawButtonText(juce::Graphics& g, juce::TextButton& b, bool, bool) override {
        g.setColour(b.getToggleState() ? kBg : kText);
        g.setFont(15.f);
        g.drawFittedText(b.getButtonText(), b.getLocalBounds().reduced(4, 0), juce::Justification::centred, 2);
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider& s) override {
        auto bounds = juce::Rectangle<int>(x, y, w, h).toFloat().reduced(6.f);
        const float d = juce::jmin(bounds.getWidth(), bounds.getHeight());
        auto c = bounds.getCentre();
        juce::Rectangle<float> ring(c.x - d / 2, c.y - d / 2, d, d);
        const float ang = a0 + pos * (a1 - a0);
        const float tw = juce::jmax(3.f, d * 0.07f);
        auto arcR = ring.reduced(tw / 2);
        auto fillCol = s.findColour(juce::Slider::rotarySliderFillColourId);

        juce::Path track;
        track.addCentredArc(c.x, c.y, arcR.getWidth() / 2, arcR.getHeight() / 2, 0.f, a0, a1, true);
        g.setColour(juce::Colours::black.withAlpha(0.7f));
        g.strokePath(track, juce::PathStrokeType(tw, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        juce::Path val;
        val.addCentredArc(c.x, c.y, arcR.getWidth() / 2, arcR.getHeight() / 2, 0.f, a0, ang, true);
        g.setColour(fillCol.withAlpha(0.3f));
        g.strokePath(val, juce::PathStrokeType(tw * 2.4f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour(fillCol);
        g.strokePath(val, juce::PathStrokeType(tw, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        auto body = ring.reduced(tw * 2.4f);
        g.setColour(juce::Colours::black.withAlpha(0.55f));
        g.fillEllipse(body.translated(0.f, 3.f));
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff7d8997), body.getX() + body.getWidth() * 0.3f, body.getY() + body.getHeight() * 0.2f,
                                               juce::Colour(0xff1a2027), body.getRight(), body.getBottom(), true));
        g.fillEllipse(body);
        g.setColour(juce::Colours::white.withAlpha(0.2f));
        g.drawEllipse(body, 1.f);

        const float len = body.getWidth() / 2;
        juce::Path pointer;
        pointer.addRoundedRectangle(-2.f, -len + 5.f, 4.f, len * 0.45f, 2.f);
        g.setColour(fillCol.brighter(0.3f));
        g.fillPath(pointer, juce::AffineTransform::rotation(ang).translated(c.x, c.y));
    }

    int getSliderThumbRadius(juce::Slider& s) override {
        return s.isVertical() ? 12 : juce::LookAndFeel_V4::getSliderThumbRadius(s);
    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int w, int h, float pos, float minPos, float maxPos,
                          juce::Slider::SliderStyle style, juce::Slider& s) override {
        if (style != juce::Slider::LinearVertical) {
            juce::LookAndFeel_V4::drawLinearSlider(g, x, y, w, h, pos, minPos, maxPos, style, s);
            return;
        }
        const float cx = (float) x + (float) w * 0.5f;
        const float slotW = 9.f;
        juce::Rectangle<float> slot(cx - slotW / 2, (float) y, slotW, (float) h);
        g.setColour(juce::Colours::black.withAlpha(0.9f));
        g.fillRoundedRectangle(slot, 4.f);
        g.setColour(juce::Colours::white.withAlpha(0.07f));
        g.drawRoundedRectangle(slot, 4.f, 1.f);

        auto col = s.findColour(juce::Slider::trackColourId);
        auto fill = slot.withTop(pos).reduced(1.5f, 0.f);
        if (fill.getHeight() > 0.f) {
            g.setColour(col.withAlpha(0.3f));
            g.fillRoundedRectangle(fill.expanded(2.f, 0.f), 4.f);
            g.setColour(col);
            g.fillRoundedRectangle(fill, 3.f);
        }
        if (w >= 44) {
            g.setColour(kDim.withAlpha(0.6f));
            for (int i = 0; i <= 10; ++i) {
                const float ty = (float) y + (float) h * (float) i / 10.f;
                const float len = (i % 5 == 0) ? 9.f : 5.f;
                g.drawLine(cx + slotW / 2 + 6.f, ty, cx + slotW / 2 + 6.f + len, ty, 1.f);
            }
        }
        const float capW = juce::jmin(36.f, (float) w * 0.9f), capH = 24.f;
        juce::Rectangle<float> cap(cx - capW / 2, pos - capH / 2, capW, capH);
        juce::Path p;
        p.addRoundedRectangle(cap, 5.f);
        juce::DropShadow(juce::Colours::black.withAlpha(0.75f), 7, {0, 3}).drawForPath(g, p);
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff94a0ad), cap.getX(), cap.getY(),
                                               juce::Colour(0xff262d35), cap.getX(), cap.getBottom(), false));
        g.fillRoundedRectangle(cap, 5.f);
        g.setColour(juce::Colours::white.withAlpha(0.3f));
        g.drawRoundedRectangle(cap, 5.f, 1.f);
        g.setColour(col);
        g.fillRect(cap.getX() + 5.f, cap.getCentreY() - 1.f, cap.getWidth() - 10.f, 2.f);
    }
};

// Fader or knob that glides to a target value (scene recall). Grabbing it cancels the glide.
class GlideSlider : public juce::Slider {
public:
    explicit GlideSlider(SliderStyle s = LinearVertical) : juce::Slider(s, NoTextBox) {
        setRange(0.0, 1.0, 0.001);
        setColour(trackColourId, kAccent);
        setColour(thumbColourId, kText);
        setColour(rotarySliderFillColourId, kAccent);
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
    std::array<bool, 16> on{};
    std::array<double, 16> vol{};
    std::array<double, 4> knob{};
};

class PerformanceScreen : public juce::Component, private juce::Timer, private juce::ListBoxModel {
public:
    PerformanceScreen() {
        setLookAndFeel(&look);
        setSize(1280, 720);

        title.setText("OpenKeys", juce::dontSendNotification);
        title.setFont(juce::Font(juce::FontOptions(30.f, juce::Font::bold)));
        title.setColour(juce::Label::textColourId, kAccent);
        addAndMakeVisible(title);

        auto setup = [this](juce::TextButton& b, const juce::String& text, juce::Colour on, int radio) {
            b.setButtonText(text);
            b.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff39434f));
            b.setColour(juce::TextButton::buttonOnColourId, on);
            b.setClickingTogglesState(true);
            if (radio != 0) b.setRadioGroupId(radio);
            addAndMakeVisible(b);
        };

        panic.setButtonText("PANIC");
        panic.setColour(juce::TextButton::buttonColourId, kDanger.darker(0.2f));
        panic.onClick = [this] { showMsg("PANIC: all notes off (dummy, engine not connected)"); };
        addAndMakeVisible(panic);

        for (int i = 0; i < 8; ++i) {
            setup(catBtn[(size_t) i], categories[(size_t) i], kAccent, 1);
            catBtn[(size_t) i].onClick = [this, i] { curCat = i; patchList.deselectAllRows(); patchList.updateContent(); patchList.repaint(); };
        }
        catBtn[0].setToggleState(true, juce::dontSendNotification);

        for (int i = 0; i < 8; ++i) {
            setup(sceneBtn[(size_t) i], sceneNames[(size_t) i], kScene, 2);
            sceneBtn[(size_t) i].onClick = [this, i] { recallScene(i); };
        }
        setup(layersBtn, "LAYERS", kAccent, 0);
        layersBtn.onClick = [this] { applyMode(); };

        for (int i = 0; i < 16; ++i) {
            const auto idx = (size_t) i;
            setup(layerBtn[idx], "Layer " + juce::String(i + 1), layerColour(i), 3);
            layerBtn[idx].onClick = [this, i] {
                sel = i;
                showMsg("Layer " + juce::String(i + 1) + " selected. Tap a sound on the left to put it here.");
            };
            setup(layerOn[idx], "ON", kLed, 0);
            layerOn[idx].onClick = [this, i] {
                showMsg("Layer " + juce::String(i + 1) + (layerOn[(size_t) i].getToggleState() ? " on" : " off"));
            };
            faders[idx].setColour(juce::Slider::trackColourId, layerColour(i));
            faders[idx].onValueChange = [this, i] {
                if (faders[(size_t) i].isMouseButtonDown())
                    showMsg("Layer " + juce::String(i + 1) + " volume " + juce::String(juce::roundToInt(faders[(size_t) i].getValue() * 100.0)) + "%");
            };
            addAndMakeVisible(faders[idx]);
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

        patchList.setModel(this);
        patchList.setRowHeight(36);
        patchList.setColour(juce::ListBox::backgroundColourId, juce::Colour(0xff0d1116));
        addAndMakeVisible(patchList);

        buildDummyScenes();
        layerBtn[0].setToggleState(true, juce::dontSendNotification);
        layerBtn[0].setButtonText("Concert Grand");
        layerBtn[1].setButtonText("String Ensemble");
        layerBtn[2].setButtonText("Warm Pad");
        sceneBtn[0].setToggleState(true, juce::dontSendNotification);
        recallScene(0);
        applyMode();
        showMsg("Tap LAYERS to open the 16 layers");
    }

    ~PerformanceScreen() override { setLookAndFeel(nullptr); }

    void paint(juce::Graphics& g) override {
        g.fillAll(kBg);
        drawPanel(g, soundsArea.toFloat());
        drawPanel(g, scenesArea.toFloat());
        drawPanel(g, lowerArea.toFloat());

        // LCD display
        auto lcd = lcdArea.toFloat();
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff10333a), lcd.getX(), lcd.getY(),
                                               juce::Colour(0xff061519), lcd.getX(), lcd.getBottom(), false));
        g.fillRoundedRectangle(lcd, 8.f);
        g.setColour(juce::Colour(0xff355c64));
        g.drawRoundedRectangle(lcd.reduced(0.5f), 8.f, 1.5f);
        auto a = lcdArea.reduced(16, 6);
        auto l1 = a.removeFromTop(a.getHeight() / 2);
        g.setColour(juce::Colour(0xff9ff0ff));
        g.setFont(21.f);
        g.drawText("SCENE   " + sceneName, l1, juce::Justification::centredLeft);
        g.setColour(juce::Colour(0xff5aa9bd));
        g.setFont(15.f);
        g.drawText(msg, a, juce::Justification::centredLeft);

        g.setColour(kDim);
        g.setFont(13.f);
        g.drawText("SOUNDS", lblSounds, juce::Justification::centredLeft);
        g.drawText("SCENES", lblScenes, juce::Justification::centredLeft);
        g.drawText(layersBtn.getToggleState() ? "LAYERS  (tap a layer, then pick its sound on the left)" : "KNOBS", lblLower, juce::Justification::centredLeft);
    }

    void resized() override {
        auto r = getLocalBounds().reduced(14);
        auto top = r.removeFromTop(72);
        panic.setBounds(top.removeFromRight(130).reduced(0, 10));
        title.setBounds(top.removeFromLeft(210));
        top.removeFromLeft(10);
        top.removeFromRight(14);
        lcdArea = top.withSizeKeepingCentre(juce::jmin(top.getWidth(), 640), 60);
        r.removeFromTop(10);

        soundsArea = r.removeFromLeft(r.getWidth() * 26 / 100);
        r.removeFromLeft(12);
        auto pa = soundsArea.reduced(12);
        lblSounds = pa.removeFromTop(20);
        auto cats = pa.removeFromTop(96);
        for (int i = 0; i < 8; ++i) {
            const int cw = cats.getWidth() / 4, ch = cats.getHeight() / 2;
            catBtn[(size_t) i].setBounds(cats.getX() + (i % 4) * cw, cats.getY() + (i / 4) * ch, cw, ch);
        }
        pa.removeFromTop(10);
        patchList.setBounds(pa);

        scenesArea = r.removeFromTop(112);
        r.removeFromTop(12);
        lowerArea = r;

        auto sp = scenesArea.reduced(12);
        lblScenes = sp.removeFromTop(20);
        layersBtn.setBounds(sp.removeFromRight(juce::jmax(150, sp.getWidth() * 18 / 100)).reduced(2));
        sp.removeFromRight(8);
        for (int i = 0; i < 8; ++i) {
            const int cw = sp.getWidth() / 8;
            sceneBtn[(size_t) i].setBounds(sp.getX() + i * cw, sp.getY(), cw, sp.getHeight());
        }

        auto lp = lowerArea.reduced(12);
        lblLower = lp.removeFromTop(20);
        const int cw = lp.getWidth() / 16;
        for (int i = 0; i < 16; ++i) {
            juce::Rectangle<int> cell(lp.getX() + i * cw, lp.getY(), cw, lp.getHeight());
            layerBtn[(size_t) i].setBounds(cell.removeFromTop(50).reduced(1, 0));
            layerOn[(size_t) i].setBounds(cell.removeFromBottom(32).reduced(1, 0));
            faders[(size_t) i].setBounds(cell.reduced(juce::jmax(2, cell.getWidth() / 6), 8));
        }
        const int kw = (lp.getWidth() * 66 / 100) / 4;
        for (int i = 0; i < 4; ++i) {
            juce::Rectangle<int> cell(lp.getX() + i * kw, lp.getY(), kw, lp.getHeight());
            knobLabels[(size_t) i].setBounds(cell.removeFromBottom(22));
            knobs[(size_t) i].setBounds(cell.reduced(2));
        }
        auto mk = lp.withTrimmedLeft(kw * 4);
        masterLabel.setBounds(mk.removeFromBottom(22));
        masterKnob.setBounds(mk.reduced(2));
    }

private:
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

    void showMsg(const juce::String& m) { msg = m; repaint(lcdArea); }

    void applyMode() {
        const bool lay = layersBtn.getToggleState();
        for (int i = 0; i < 16; ++i) {
            layerBtn[(size_t) i].setVisible(lay);
            layerOn[(size_t) i].setVisible(lay);
            faders[(size_t) i].setVisible(lay);
        }
        for (int i = 0; i < 4; ++i) { knobs[(size_t) i].setVisible(!lay); knobLabels[(size_t) i].setVisible(!lay); }
        masterKnob.setVisible(!lay);
        masterLabel.setVisible(!lay);
        repaint();
    }

    void buildDummyScenes() {
        scenes.clear();
        for (int s = 0; s < 8; ++s) {
            SceneState st;
            for (int i = 0; i < 16; ++i) {
                bool on = false;
                if (s == 4) on = (i == 2);
                else if (s != 7) on = (i < 3);
                st.on[(size_t) i] = on;
                st.vol[(size_t) i] = 0.2 + 0.7 * std::abs(std::sin(0.9 * (s + 1) * (i + 1)));
            }
            for (int i = 0; i < 4; ++i) st.knob[(size_t) i] = 0.15 + 0.7 * std::abs(std::sin(1.3 * (s + 2) + i));
            scenes.push_back(st);
        }
    }

    void recallScene(int s) {
        const auto& st = scenes[(size_t) s];
        for (int i = 0; i < 16; ++i) {
            layerOn[(size_t) i].setToggleState(st.on[(size_t) i], juce::dontSendNotification);
            faders[(size_t) i].glideTo(st.vol[(size_t) i]);
        }
        for (int i = 0; i < 4; ++i) knobs[(size_t) i].glideTo(st.knob[(size_t) i]);
        sceneName = sceneNames[(size_t) s];
        showMsg("Scene recalled");
        repaint(lcdArea);
        startTimerHz(60);
    }

    void timerCallback() override {
        bool moving = false;
        for (auto& f : faders) moving |= f.step();
        for (auto& k : knobs) moving |= k.step();
        if (!moving) stopTimer();
    }

    // ---- sound list: tapping a sound puts it on the selected layer ----
    int getNumRows() override { return patches[(size_t) curCat].size(); }
    void paintListBoxItem(int row, juce::Graphics& g, int w, int h, bool selected) override {
        if (selected) {
            g.setGradientFill(juce::ColourGradient(kAccent.withAlpha(0.45f), 0.f, 0.f, kAccent.withAlpha(0.15f), 0.f, (float) h, false));
            g.fillRect(0, 0, w, h);
            g.setColour(kAccent);
            g.fillRect(0, 0, 3, h);
        }
        g.setColour(juce::Colours::white.withAlpha(0.05f));
        g.drawHorizontalLine(h - 1, 0.f, (float) w);
        g.setColour(selected ? kText : kDim);
        g.setFont(17.f);
        g.drawText(patches[(size_t) curCat][row], 14, 0, w - 14, h, juce::Justification::centredLeft);
    }
    void listBoxItemClicked(int row, const juce::MouseEvent&) override {
        if (row < 0 || row >= patches[(size_t) curCat].size()) return;
        const auto name = patches[(size_t) curCat][row];
        layerBtn[(size_t) sel].setButtonText(name);
        layerOn[(size_t) sel].setToggleState(true, juce::dontSendNotification);
        showMsg("Layer " + juce::String(sel + 1) + " = " + name);
    }

    OkLook look;
    juce::Label title, masterLabel;
    juce::TextButton panic, layersBtn;
    std::array<juce::TextButton, 8> catBtn;
    std::array<juce::TextButton, 8> sceneBtn;
    std::array<juce::TextButton, 16> layerBtn, layerOn;
    juce::ListBox patchList { "patches", nullptr };
    std::array<GlideSlider, 16> faders;
    std::array<GlideSlider, 4> knobs;
    std::array<juce::Label, 4> knobLabels;
    GlideSlider masterKnob { juce::Slider::RotaryHorizontalVerticalDrag };
    std::vector<SceneState> scenes;
    int curCat = 0, sel = 0;
    juce::String sceneName, msg;
    juce::Rectangle<int> soundsArea, scenesArea, lowerArea, lcdArea, lblSounds, lblScenes, lblLower;
};

}  // namespace okui
