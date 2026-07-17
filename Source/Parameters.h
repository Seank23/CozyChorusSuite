#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include <memory>

namespace CozyChorus
{
	// Centralised parameter identifiers and the APVTS layout for the whole suite.
	namespace ParameterIDs
	{
		inline constexpr auto EffectType = "effectType";
		inline constexpr auto Mix = "mix";
		inline constexpr auto Rate = "rate";
		inline constexpr auto Depth = "depth";
		inline constexpr auto Width = "width";
		inline constexpr auto ChorusVoices = "chorusVoices";
		inline constexpr auto FlangerFeedback = "flangerFeedback";
		inline constexpr auto FlangerBaseDelay = "flangerBaseDelay";
	}

	// Selectable effect. Values must match the effectType choice order below.
	enum class EffectType
	{
		Chorus = 0,
		Flanger,
		Phaser,
		Vibe
	};

	inline juce::StringArray GetEffectTypeChoices()
	{
		return { "Chorus", "Flanger", "Phaser", "Vibe" };
	}

	inline juce::AudioProcessorValueTreeState::ParameterLayout CreateParameterLayout()
	{
		juce::AudioProcessorValueTreeState::ParameterLayout layout;

		layout.add(std::make_unique<juce::AudioParameterChoice>(
			juce::ParameterID{ ParameterIDs::EffectType, 1 },
			"Effect",
			GetEffectTypeChoices(),
			0));

		layout.add(std::make_unique<juce::AudioParameterFloat>(
			juce::ParameterID{ ParameterIDs::Mix, 1 },
			"Mix",
			juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
			50.0f));

		layout.add(std::make_unique<juce::AudioParameterFloat>(
			juce::ParameterID{ ParameterIDs::Rate, 1 },
			"Rate (Hz)",
			juce::NormalisableRange<float>(0.05f, 5.0f, 0.05f, 0.35f),
			0.8f));

		layout.add(std::make_unique<juce::AudioParameterFloat>(
			juce::ParameterID{ ParameterIDs::Depth, 1 },
			"Depth",
			juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
			50.0f));

		layout.add(std::make_unique<juce::AudioParameterFloat>(
			juce::ParameterID{ ParameterIDs::Width, 1 },
			"Stereo Width",
			juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
			50.0f));

		layout.add(std::make_unique<juce::AudioParameterInt>(
			juce::ParameterID{ ParameterIDs::ChorusVoices, 1 },
			"Voices",
			1, 3, 1));

		layout.add(std::make_unique<juce::AudioParameterFloat>(
			juce::ParameterID{ ParameterIDs::FlangerFeedback, 1 },
			"Feedback",
			juce::NormalisableRange<float>(-95.0f, 95.0f, 0.1f, 0.4f),
			45.0f));

		layout.add(std::make_unique<juce::AudioParameterFloat>(
			juce::ParameterID{ ParameterIDs::FlangerBaseDelay, 1 },
			"Base Delay (ms)",
			juce::NormalisableRange<float>(0.2f, 5.0f, 0.01f),
			1.0f));

		return layout;
	}
}