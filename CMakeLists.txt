cmake_minimum_required(VERSION 3.22)

project(RosettaPrompter VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(JUCE)

juce_add_plugin(RosettaPrompter
    COMPANY_NAME "CHEAPSMUSIC"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE CmpM
    PLUGIN_CODE RPrm
    FORMATS VST3
    PRODUCT_NAME "RosettaPrompter"
)

target_sources(RosettaPrompter PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/TeleprompterComponent.cpp
    Source/TeleprompterComponent.h
)

target_compile_definitions(RosettaPrompter PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(RosettaPrompter PRIVATE
    juce::juce_audio_processors
    juce::juce_gui_basics
    juce::juce_gui_extra
)
