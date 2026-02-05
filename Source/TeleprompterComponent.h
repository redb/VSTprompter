#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <functional>

class TeleprompterComponent : public juce::Component, private juce::Timer
{
public:
    TeleprompterComponent();

    void setFontSize (float newSize);
    void setTheme (bool useDarkTheme);

    void setActiveLine (int lineIndex);
    int getActiveLine() const;

    void setText (const juce::String& text);
    juce::String getText() const;

    int getNumLines() const;

    void setScrollTargetForLine (int lineIndex);
    void setScrollTargetNormalized (double proportion);
    void scrollToTop();

    std::function<void(const juce::String&)> onTextChanged;

    void resized() override;

private:
    class ContentComponent : public juce::Component
    {
    public:
        ContentComponent();

        void setFontSize (float newSize);
        void setTheme (bool useDarkTheme);
        void setActiveLine (int lineIndex);
        int getActiveLine() const;

        void setText (const juce::String& text);
        juce::String getText() const;

        int getNumLines() const;
        int getLineHeight() const;
        int getPadding() const;

        void resized() override;
        void paint (juce::Graphics& g) override;

        std::function<void(const juce::String&)> onTextChanged;

    private:
        void handleTextChanged();
        void updateMetrics();
        static int countLines (const juce::String& text);

        juce::TextEditor editor;
        int activeLine = 0;
        int numLines = 1;
        int lineHeight = 24;
        int padding = 12;
        float fontSize = 24.0f;
        bool darkTheme = true;

        juce::Colour backgroundColour;
        juce::Colour textColour;
        juce::Colour highlightColour;
    };

    void timerCallback() override;
    void updateContentHeight();
    void clampScrollTarget();
    int getMaxScroll() const;

    juce::Viewport viewport;
    ContentComponent content;

    double targetScrollY = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TeleprompterComponent)
};
