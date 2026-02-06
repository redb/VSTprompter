#include "TeleprompterComponent.h"
#include <cmath>

TeleprompterComponent::TeleprompterComponent()
{
    viewport.setViewedComponent (&content, false);
    viewport.setScrollBarsShown (false, false, false, false);
    addAndMakeVisible (viewport);

    content.onTextChanged = [this] (const juce::String& text)
    {
        updateContentHeight();
        if (onTextChanged)
            onTextChanged (text);
    };

    startTimerHz (60);
}

void TeleprompterComponent::setFontSize (float newSize)
{
    content.setFontSize (newSize);
    updateContentHeight();
}

void TeleprompterComponent::setTheme (bool useDarkTheme)
{
    content.setTheme (useDarkTheme);
}

void TeleprompterComponent::setActiveLine (int lineIndex)
{
    content.setActiveLine (lineIndex);
}

int TeleprompterComponent::getActiveLine() const
{
    return content.getActiveLine();
}

void TeleprompterComponent::setText (const juce::String& text)
{
    content.setText (text);
    updateContentHeight();
}

juce::String TeleprompterComponent::getText() const
{
    return content.getText();
}

int TeleprompterComponent::getNumLines() const
{
    return content.getNumLines();
}

void TeleprompterComponent::setScrollTargetForLine (int lineIndex)
{
    const int clampedLine = juce::jlimit (0, juce::jmax (0, getNumLines() - 1), lineIndex);
    const double lineTop = static_cast<double> (content.getPadding() + clampedLine * content.getLineHeight());
    const double viewHeight = static_cast<double> (viewport.getHeight());
    const double target = lineTop - (viewHeight * 0.5) + (content.getLineHeight() * 0.5);

    targetScrollY = juce::jlimit (0.0, static_cast<double> (getMaxScroll()), target);
}

void TeleprompterComponent::setScrollTargetNormalized (double proportion)
{
    const auto clamped = juce::jlimit (0.0, 1.0, proportion);
    targetScrollY = clamped * static_cast<double> (getMaxScroll());
}

void TeleprompterComponent::scrollToTop()
{
    targetScrollY = 0.0;
    viewport.setViewPosition (0, 0);
}

void TeleprompterComponent::resized()
{
    viewport.setBounds (getLocalBounds());
    updateContentHeight();
}

void TeleprompterComponent::timerCallback()
{
    const auto currentY = static_cast<double> (viewport.getViewPositionY());
    const double delta = (targetScrollY - currentY) * 0.2;
    const double newY = std::abs (delta) < 0.5 ? targetScrollY : currentY + delta;

    viewport.setViewPosition (0, static_cast<int> (std::round (newY)));
}

void TeleprompterComponent::updateContentHeight()
{
    const auto width = juce::jmax (1, viewport.getWidth());
    const auto height = juce::jmax (viewport.getHeight(), content.getNumLines() * content.getLineHeight() + content.getPadding() * 2);

    content.setSize (width, height);
    clampScrollTarget();
}

void TeleprompterComponent::clampScrollTarget()
{
    targetScrollY = juce::jlimit (0.0, static_cast<double> (getMaxScroll()), targetScrollY);
}

int TeleprompterComponent::getMaxScroll() const
{
    return juce::jmax (0, content.getHeight() - viewport.getHeight());
}

TeleprompterComponent::ContentComponent::ContentComponent()
{
    addAndMakeVisible (editor);

    editor.setMultiLine (false, true);
    editor.setReturnKeyStartsNewLine (true);
    editor.setScrollbarsShown (false);
    editor.setCaretVisible (true);
    editor.setPopupMenuEnabled (true);
    editor.setTabKeyUsedAsCharacter (false);

    editor.onTextChange = [this]
    {
        handleTextChanged();
    };

    setTheme (true);
    setFontSize (fontSize);
}

void TeleprompterComponent::ContentComponent::setFontSize (float newSize)
{
    fontSize = newSize;
    updateMetrics();
    resized();
    repaint();
}

void TeleprompterComponent::ContentComponent::setTheme (bool useDarkTheme)
{
    darkTheme = useDarkTheme;

    if (darkTheme)
    {
        backgroundColour = juce::Colour (0xff0f1115);
        textColour = juce::Colour (0xfff2f2f2);
        highlightColour = juce::Colour (0xff2f81f7);
    }
    else
    {
        backgroundColour = juce::Colour (0xfff5e94b);
        textColour = juce::Colour (0xff1a1a1a);
        highlightColour = juce::Colour (0xfff58a1f);
    }

    editor.setColour (juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    editor.setColour (juce::TextEditor::textColourId, textColour);
    editor.setColour (juce::TextEditor::highlightColourId, highlightColour.withAlpha (0.25f));
    editor.setColour (juce::TextEditor::highlightedTextColourId, textColour);
    editor.setColour (juce::TextEditor::outlineColourId, highlightColour.withAlpha (0.2f));
    editor.setColour (juce::TextEditor::focusedOutlineColourId, highlightColour.withAlpha (0.5f));

    repaint();
}

void TeleprompterComponent::ContentComponent::setActiveLine (int lineIndex)
{
    activeLine = juce::jlimit (0, juce::jmax (0, numLines - 1), lineIndex);
    repaint();
}

int TeleprompterComponent::ContentComponent::getActiveLine() const
{
    return activeLine;
}

void TeleprompterComponent::ContentComponent::setText (const juce::String& text)
{
    editor.setText (text, false);
    handleTextChanged();
}

juce::String TeleprompterComponent::ContentComponent::getText() const
{
    return editor.getText();
}

int TeleprompterComponent::ContentComponent::getNumLines() const
{
    return numLines;
}

int TeleprompterComponent::ContentComponent::getLineHeight() const
{
    return lineHeight;
}

int TeleprompterComponent::ContentComponent::getPadding() const
{
    return padding;
}

void TeleprompterComponent::ContentComponent::resized()
{
    editor.setBounds (padding, padding, juce::jmax (1, getWidth() - padding * 2), juce::jmax (1, getHeight() - padding * 2));
}

void TeleprompterComponent::ContentComponent::paint (juce::Graphics& g)
{
    g.fillAll (backgroundColour);

    if (numLines > 0)
    {
        const int clampedLine = juce::jlimit (0, numLines - 1, activeLine);
        const float y = static_cast<float> (padding + clampedLine * lineHeight);

        g.setColour (highlightColour.withAlpha (darkTheme ? 0.25f : 0.2f));
        g.fillRoundedRectangle (static_cast<float> (padding / 2), y, static_cast<float> (getWidth() - padding), static_cast<float> (lineHeight), 6.0f);
    }
}

void TeleprompterComponent::ContentComponent::handleTextChanged()
{
    numLines = countLines (editor.getText());
    activeLine = juce::jlimit (0, juce::jmax (0, numLines - 1), activeLine);

    if (onTextChanged)
        onTextChanged (editor.getText());
}

void TeleprompterComponent::ContentComponent::updateMetrics()
{
    juce::Font font (fontSize);
    editor.setFont (font);
    editor.setLineSpacing (font.getHeight() * 0.35f);

    lineHeight = static_cast<int> (std::ceil (font.getHeight() + editor.getLineSpacing()));
    padding = static_cast<int> (std::ceil (font.getHeight() * 0.6f));
}

int TeleprompterComponent::ContentComponent::countLines (const juce::String& text)
{
    if (text.isEmpty())
        return 1;

    int lines = 1;
    for (auto character : text)
        if (character == '\n')
            ++lines;

    return lines;
}
