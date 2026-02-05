#include "PluginEditor.h"
#include <cmath>

RosettaPrompterAudioProcessorEditor::RosettaPrompterAudioProcessorEditor (RosettaPrompterAudioProcessor& p)
    : AudioProcessorEditor (&p),
      processor (p)
{
    setUsingNativeTitleBar (true);
    setResizable (true, true);
    setResizeLimits (420, 260, 2400, 1800);
    setSize (600, 280);

    addAndMakeVisible (teleprompter);

    autoScrollButton.setClickingTogglesState (true);
    resetOnStopButton.setClickingTogglesState (true);

    fontSizeSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    fontSizeSlider.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 60, 20);
    fontSizeSlider.setTextValueSuffix (" pt");

    manualScrollSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    manualScrollSlider.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 60, 20);

    startBarLabel.setJustificationType (juce::Justification::centredLeft);
    endBarLabel.setJustificationType (juce::Justification::centredLeft);

    themeBox.addItem ("Dark", 1);
    themeBox.addItem ("Light", 2);
    themeBox.setSelectedId (1, juce::dontSendNotification);
    themeBox.onChange = [this]
    {
        darkTheme = (themeBox.getSelectedId() == 1);
        teleprompter.setTheme (darkTheme);
        repaint();
    };

    importButton.onClick = [this]
    {
        juce::FileChooser chooser ("Import lyrics", juce::File(), "*.txt");
        chooser.launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this] (const juce::FileChooser& fc)
            {
                const auto file = fc.getResult();
                if (file.existsAsFile())
                {
                    const auto text = file.loadFileAsString();
                    teleprompter.setText (text);
                    processor.setLyricsText (text);
                }
            });
    };

    setStartButton.onClick = [this]
    {
        processor.setStartBarToCurrent();
    };

    setEndButton.onClick = [this]
    {
        processor.setEndBarToCurrent();
    };

    teleprompter.onTextChanged = [this] (const juce::String& text)
    {
        processor.setLyricsText (text);
    };

    teleprompter.setText (processor.getLyricsText());
    teleprompter.setTheme (darkTheme);

    addAndMakeVisible (autoScrollButton);
    addAndMakeVisible (resetOnStopButton);
    addAndMakeVisible (fontSizeSlider);
    addAndMakeVisible (manualScrollSlider);
    addAndMakeVisible (startBarLabel);
    addAndMakeVisible (endBarLabel);
    addAndMakeVisible (setStartButton);
    addAndMakeVisible (setEndButton);
    addAndMakeVisible (importButton);
    addAndMakeVisible (themeBox);

    autoScrollAttachment = std::make_unique<ButtonAttachment> (processor.apvts, RosettaPrompterAudioProcessor::ParamIDs::autoScroll, autoScrollButton);
    resetOnStopAttachment = std::make_unique<ButtonAttachment> (processor.apvts, RosettaPrompterAudioProcessor::ParamIDs::resetOnStop, resetOnStopButton);
    fontSizeAttachment = std::make_unique<SliderAttachment> (processor.apvts, RosettaPrompterAudioProcessor::ParamIDs::fontSize, fontSizeSlider);
    manualScrollAttachment = std::make_unique<SliderAttachment> (processor.apvts, RosettaPrompterAudioProcessor::ParamIDs::manualScroll, manualScrollSlider);

    refreshLabels();

    startTimerHz (30);
}

RosettaPrompterAudioProcessorEditor::~RosettaPrompterAudioProcessorEditor() = default;

void RosettaPrompterAudioProcessorEditor::paint (juce::Graphics& g)
{
    const auto background = darkTheme ? juce::Colour (0xff0f1115) : juce::Colour (0xfff5f1e8);
    g.fillAll (background);
}

void RosettaPrompterAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced (12);
    auto controls = bounds.removeFromTop (90);

    auto row1 = controls.removeFromTop (40);
    autoScrollButton.setBounds (row1.removeFromLeft (120));
    resetOnStopButton.setBounds (row1.removeFromLeft (140));
    themeBox.setBounds (row1.removeFromLeft (120));
    importButton.setBounds (row1.removeFromLeft (140));

    auto row2 = controls.removeFromTop (40);
    startBarLabel.setBounds (row2.removeFromLeft (140));
    setStartButton.setBounds (row2.removeFromLeft (140));
    endBarLabel.setBounds (row2.removeFromLeft (140));
    setEndButton.setBounds (row2.removeFromLeft (140));
    fontSizeSlider.setBounds (row2.removeFromLeft (200));
    manualScrollSlider.setBounds (row2);

    teleprompter.setBounds (bounds);
}

void RosettaPrompterAudioProcessorEditor::timerCallback()
{
    const float fontSize = processor.getParameterValue (RosettaPrompterAudioProcessor::ParamIDs::fontSize);
    if (! juce::approximatelyEqual (fontSize, lastFontSize))
    {
        teleprompter.setFontSize (fontSize);
        lastFontSize = fontSize;
    }

    refreshLabels();
    updateTransportDrivenUI();

    if (processor.consumeStoppedFlag())
    {
        const bool resetOnStop = processor.getParameterValue (RosettaPrompterAudioProcessor::ParamIDs::resetOnStop) > 0.5f;
        if (resetOnStop)
        {
            teleprompter.setActiveLine (0);
            teleprompter.scrollToTop();
        }
    }
}

void RosettaPrompterAudioProcessorEditor::updateTransportDrivenUI()
{
    const float startBar = processor.getParameterValue (RosettaPrompterAudioProcessor::ParamIDs::startBar);
    const float endBar = processor.getParameterValue (RosettaPrompterAudioProcessor::ParamIDs::endBar);
    const bool autoScrollOn = processor.getParameterValue (RosettaPrompterAudioProcessor::ParamIDs::autoScroll) > 0.5f;

    if (processor.isPlayheadValid())
    {
        const double barPos = processor.getLastBarPosition();
        double progress = 0.0;
        if (endBar > startBar)
            progress = (barPos - startBar) / static_cast<double> (endBar - startBar);

        progress = juce::jlimit (0.0, 1.0, progress);

        const int numLines = teleprompter.getNumLines();
        const int maxIndex = juce::jmax (0, numLines - 1);
        const int activeLine = static_cast<int> (std::floor (progress * maxIndex));

        teleprompter.setActiveLine (activeLine);

        if (autoScrollOn)
            teleprompter.setScrollTargetForLine (activeLine);
    }

    if (! autoScrollOn)
    {
        const float manual = processor.getParameterValue (RosettaPrompterAudioProcessor::ParamIDs::manualScroll);
        teleprompter.setScrollTargetNormalized (manual);
    }
}

void RosettaPrompterAudioProcessorEditor::refreshLabels()
{
    const float startBar = processor.getParameterValue (RosettaPrompterAudioProcessor::ParamIDs::startBar);
    const float endBar = processor.getParameterValue (RosettaPrompterAudioProcessor::ParamIDs::endBar);

    startBarLabel.setText ("Start: " + juce::String (startBar, 2), juce::dontSendNotification);
    endBarLabel.setText ("End: " + juce::String (endBar, 2), juce::dontSendNotification);
}
