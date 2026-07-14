#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include <memory>

namespace CozyChorus
{
	// Centralised parameter identifiers and the APVTS layout for the whole suite.
	namespace ParameterIDs
	{
		inline constexpr auto Mix = "mix";
		inline constexpr auto EffectType = "effectType";
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

		layout.add(std::make_unique<juce::AudioParameterFloat>(
			juce::ParameterID{ ParameterIDs::Mix, 1 },
			"Mix",
			juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
			50.0f));

		layout.add(std::make_unique<juce::AudioParameterChoice>(
			juce::ParameterID{ ParameterIDs::EffectType, 1 },
			"Effect",
			GetEffectTypeChoices(),
			0));

		return layout;
	}
}