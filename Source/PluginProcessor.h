#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class RosettaPrompterAudioProcessor : public juce::AudioProcessor
{
public:
    struct ParamIDs
    {
        static constexpr const char* autoScroll = "AutoScroll";
        static constexpr const char* fontSize = "FontSize";
        static constexpr const char* manualScroll = "ManualScroll";
        static constexpr const char* startBar = "StartBar";
        static constexpr const char* endBar = "EndBar";
        static constexpr const char* resetOnStop = "ResetOnStop";
    };

    RosettaPrompterAudioProcessor();
    ~RosettaPrompterAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    float getParameterValue (const juce::String& paramID) const;

    bool isPlayheadValid() const;
    double getLastBarPosition() const;
    bool consumeStoppedFlag();

    bool setStartBarToCurrent();
    bool setEndBarToCurrent();

    void setLyricsText (const juce::String& text);
    juce::String getLyricsText() const;

    static void logMessage (const juce::String& message);
    static juce::File getCacheFolder();

private:
    void updatePlayheadInfo();

    std::atomic<double> lastBarPosition { 0.0 };
    std::atomic<bool> playheadValid { false };
    std::atomic<bool> isPlaying { false };
    std::atomic<bool> stoppedFlag { false };

    bool wasPlaying = false;
    juce::String lyricsText;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RosettaPrompterAudioProcessor)
};
