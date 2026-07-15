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
		inline constexpr auto Voices = "voices";
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
			"Rate",
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
			juce::ParameterID{ ParameterIDs::Voices, 1 },
			"Voices",
			1, 3, 1));

		return layout;
	}
}