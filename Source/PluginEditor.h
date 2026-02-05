#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"
#include "TeleprompterComponent.h"

class RosettaPrompterAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit RosettaPrompterAudioProcessorEditor (RosettaPrompterAudioProcessor&);
    ~RosettaPrompterAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void updateTransportDrivenUI();
    void refreshLabels();

    RosettaPrompterAudioProcessor& processor;

    TeleprompterComponent teleprompter;

    juce::ToggleButton autoScrollButton { "Auto Scroll" };
    juce::ToggleButton resetOnStopButton { "Reset On Stop" };
    juce::Slider fontSizeSlider;
    juce::Slider manualScrollSlider;

    juce::Label startBarLabel;
    juce::Label endBarLabel;
    juce::TextButton setStartButton { "Set Start = Now" };
    juce::TextButton setEndButton { "Set End = Now" };

    juce::TextButton importButton { "Import .txt" };
    juce::ComboBox themeBox;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<ButtonAttachment> autoScrollAttachment;
    std::unique_ptr<ButtonAttachment> resetOnStopAttachment;
    std::unique_ptr<SliderAttachment> fontSizeAttachment;
    std::unique_ptr<SliderAttachment> manualScrollAttachment;

    float lastFontSize = 0.0f;
    bool darkTheme = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RosettaPrompterAudioProcessorEditor)
};
