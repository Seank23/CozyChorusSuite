cmake_minimum_required(VERSION 3.25)

project(CozyChorusSuite VERSION 0.1.0 LANGUAGES CXX C)

# JUCE, pinned as a git submodule (JUCE/ @ 8.0.14).
add_subdirectory(JUCE)

# VST3 + Standalone everywhere; AU only builds on Apple platforms.
set(COZY_FORMATS VST3 Standalone)
if(APPLE)
    list(APPEND COZY_FORMATS AU)
endif()

juce_add_plugin(CozyChorusSuite
    COMPANY_NAME                "Seank23"
    BUNDLE_ID                   com.seank23.CozyChorusSuite
    PLUGIN_MANUFACTURER_CODE    Sk23
    PLUGIN_CODE                 Cczs
    FORMATS                     ${COZY_FORMATS}
    PRODUCT_NAME                "CozyChorus Suite"
    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            FALSE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     FALSE
    VST3_CATEGORIES             Fx Modulation
    AU_MAIN_TYPE                kAudioUnitType_Effect)

target_sources(CozyChorusSuite
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/Parameters.h
        Source/dsp/ModulationEffect.h
        Source/dsp/NullEffect.h)

target_compile_features(CozyChorusSuite PRIVATE cxx_std_20)

target_include_directories(CozyChorusSuite
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source)

target_compile_definitions(CozyChorusSuite
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(CozyChorusSuite
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)