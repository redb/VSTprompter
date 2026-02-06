#include "PluginProcessor.h"
#include "PluginEditor.h"

RosettaPrompterAudioProcessor::RosettaPrompterAudioProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createParameterLayout())
{
}

RosettaPrompterAudioProcessor::~RosettaPrompterAudioProcessor() = default;

const juce::String RosettaPrompterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool RosettaPrompterAudioProcessor::acceptsMidi() const
{
    return false;
}

bool RosettaPrompterAudioProcessor::producesMidi() const
{
    return false;
}

bool RosettaPrompterAudioProcessor::isMidiEffect() const
{
    return false;
}

double RosettaPrompterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RosettaPrompterAudioProcessor::getNumPrograms()
{
    return 1;
}

int RosettaPrompterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RosettaPrompterAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String RosettaPrompterAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void RosettaPrompterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

void RosettaPrompterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (sampleRate, samplesPerBlock);
}

void RosettaPrompterAudioProcessor::releaseResources()
{
}

bool RosettaPrompterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainInputChannelSet() != layouts.getMainOutputChannelSet())
        return false;

    return true;
}

void RosettaPrompterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    updatePlayheadInfo();
}

bool RosettaPrompterAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* RosettaPrompterAudioProcessor::createEditor()
{
    return new RosettaPrompterAudioProcessorEditor (*this);
}

void RosettaPrompterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("lyricsText", lyricsText, nullptr);

    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void RosettaPrompterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        if (xml->hasTagName (apvts.state.getType()))
        {
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
            lyricsText = apvts.state.getProperty ("lyricsText").toString();
        }
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout RosettaPrompterAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterBool> (ParamIDs::autoScroll, "Auto Scroll", true));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (ParamIDs::fontSize, "Font Size",
        juce::NormalisableRange<float> (12.0f, 48.0f, 0.1f), 24.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (ParamIDs::manualScroll, "Manual Scroll",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.0001f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (ParamIDs::startBar, "Start Bar",
        juce::NormalisableRange<float> (0.0f, 4096.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (ParamIDs::endBar, "End Bar",
        juce::NormalisableRange<float> (0.0f, 4096.0f, 0.01f), 64.0f));
    params.push_back (std::make_unique<juce::AudioParameterBool> (ParamIDs::resetOnStop, "Reset On Stop", false));

    return { params.begin(), params.end() };
}

float RosettaPrompterAudioProcessor::getParameterValue (const juce::String& paramID) const
{
    if (auto* value = apvts.getRawParameterValue (paramID))
        return value->load();

    return 0.0f;
}

bool RosettaPrompterAudioProcessor::isPlayheadValid() const
{
    return playheadValid.load();
}

double RosettaPrompterAudioProcessor::getLastBarPosition() const
{
    return lastBarPosition.load();
}

bool RosettaPrompterAudioProcessor::consumeStoppedFlag()
{
    return stoppedFlag.exchange (false);
}

bool RosettaPrompterAudioProcessor::setStartBarToCurrent()
{
    if (! playheadValid.load())
        return false;

    const auto value = static_cast<float> (lastBarPosition.load());
    if (auto* param = apvts.getParameter (ParamIDs::startBar))
    {
        param->setValueNotifyingHost (param->convertTo0to1 (value));
        return true;
    }

    return false;
}

bool RosettaPrompterAudioProcessor::setEndBarToCurrent()
{
    if (! playheadValid.load())
        return false;

    const auto value = static_cast<float> (lastBarPosition.load());
    if (auto* param = apvts.getParameter (ParamIDs::endBar))
    {
        param->setValueNotifyingHost (param->convertTo0to1 (value));
        return true;
    }

    return false;
}

void RosettaPrompterAudioProcessor::setLyricsText (const juce::String& text)
{
    lyricsText = text;
}

juce::String RosettaPrompterAudioProcessor::getLyricsText() const
{
    return lyricsText;
}

void RosettaPrompterAudioProcessor::updatePlayheadInfo()
{
    bool gotInfo = false;
    bool isPlayingNow = false;

    if (auto* playHead = getPlayHead())
    {
#if JUCE_MAJOR_VERSION >= 7
        if (auto position = playHead->getPosition())
        {
            isPlayingNow = position->getIsPlaying();

            int numerator = 4;
            if (auto timeSig = position->getTimeSignature())
                numerator = timeSig->numerator > 0 ? timeSig->numerator : 4;

            if (auto ppq = position->getPpqPosition())
            {
                lastBarPosition.store (*ppq / static_cast<double> (numerator));
                gotInfo = true;
            }
        }
#else
        juce::AudioPlayHead::CurrentPositionInfo info;
        if (playHead->getCurrentPosition (info))
        {
            isPlayingNow = info.isPlaying;
            const int numerator = info.timeSigNumerator > 0 ? info.timeSigNumerator : 4;

            if (info.ppqPosition >= 0.0)
            {
                lastBarPosition.store (info.ppqPosition / static_cast<double> (numerator));
                gotInfo = true;
            }
        }
#endif
    }

    playheadValid.store (gotInfo);
    isPlaying.store (isPlayingNow);

    if (wasPlaying && ! isPlayingNow)
        stoppedFlag.store (true);

    wasPlaying = isPlayingNow;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RosettaPrompterAudioProcessor();
}
