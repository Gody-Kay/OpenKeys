#include <juce_gui_basics/juce_gui_basics.h>
#include "PerformanceScreen.h"

class OpenKeysApp : public juce::JUCEApplication {
public:
    const juce::String getApplicationName() override { return "OpenKeys"; }
    const juce::String getApplicationVersion() override { return "0.0.1"; }
    void initialise(const juce::String&) override { window.reset(new MainWindow(getApplicationName())); }
    void shutdown() override { window = nullptr; }
    void systemRequestedQuit() override { quit(); }

private:
    class MainWindow : public juce::DocumentWindow {
    public:
        explicit MainWindow(const juce::String& name)
            : DocumentWindow(name, juce::Colour(0xff0e1217), DocumentWindow::allButtons) {
            setUsingNativeTitleBar(true);
            setContentOwned(new okui::PerformanceScreen(), true);
            setResizable(true, true);
            setResizeLimits(960, 540, 4000, 2400);
            centreWithSize(1280, 720);
            setVisible(true);
        }
        void closeButtonPressed() override { juce::JUCEApplication::getInstance()->systemRequestedQuit(); }
    };
    std::unique_ptr<MainWindow> window;
};

START_JUCE_APPLICATION(OpenKeysApp)
