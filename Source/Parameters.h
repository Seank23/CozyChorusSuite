#pragma once
#include "dsp/ChorusEffect.h"
#include "dsp/FlangerEffect.h"
#include "dsp/PhaserEffect.h"
#include "dsp/VibeEffect.h"

#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>

namespace CozyChorus
{
	// Centralised parameter identifiers and the APVTS layout for the whole suite.
	namespace ParameterIDs
	{
		inline constexpr auto EffectType = "effectType";
		inline constexpr auto Mix = "mix";

		inline constexpr auto ChorusRate = "chorusRate";
		inline constexpr auto ChorusDepth = "chorusDepth";
		inline constexpr auto ChorusWidth = "chorusWidth";
		inline constexpr auto ChorusVoices = "chorusVoices";

		inline constexpr auto FlangerRate = "flangerRate";
		inline constexpr auto FlangerDepth = "flangerDepth";
		inline constexpr auto FlangerWidth = "flangerWidth";
		inline constexpr auto FlangerFeedback = "flangerFeedback";
		inline constexpr auto FlangerBaseDelay = "flangerBaseDelay";

		inline constexpr auto PhaserRate = "phaserRate";
		inline constexpr auto PhaserDepth = "phaserDepth";
		inline constexpr auto PhaserWidth = "phaserWidth";
		inline constexpr auto PhaserStages = "phaserStages";
		inline constexpr auto PhaserFeedback = "phaserFeedback";

		inline constexpr auto VibeRate = "vibeRate";
		inline constexpr auto VibeDepth = "vibeDepth";
		inline constexpr auto VibeWidth = "vibeWidth";
		inline constexpr auto VibeMode = "vibeMode";
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
		ChorusParameters chorusParams;
		FlangerParameters flangerParams;
		PhaserParameters phaserParams;
		VibeParameters vibeParams;

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
			juce::ParameterID{ ParameterIDs::ChorusRate, 1 },
			"Rate (Hz)",
			juce::NormalisableRange<float>(0.05f, 5.0f, 0.05f, 0.35f),
			chorusParams.RateHz));
		layout.add(std::make_unique<juce::AudioParameterFloat>(
			juce::ParameterID{ ParameterIDs::ChorusDepth, 1 },
			"Depth",
			juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
			chorusParams.Depth * 100));
		layout.add(std::make_unique<juce::AudioParameterFloat>(
			juce::ParameterID{ ParameterIDs::ChorusWidth, 1 },
			"Stereo Width",
			juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
			chorusParams.Width * 100));
		layout.add(std::make_unique<juce::AudioParameterInt>(
			juce::ParameterID{ ParameterIDs::ChorusVoices, 1 },
			"Voices",
			1, 3, chorusParams.Voices));

		layout.add(std::make_unique<juce::AudioParameterFloat>(
			juce::ParameterID{ ParameterIDs::FlangerRate, 1 },
			"Rate (Hz)",
			juce::NormalisableRange<float>(0.05f, 5.0f, 0.05f, 0.35f),
			flangerParams.RateHz));
		layout.add(std::make_unique<juce::AudioParameterFloat>(
			juce::ParameterID{ ParameterIDs::FlangerDepth, 1 },
			"Depth",
			juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
			flangerParams.Depth * 100));
		layout.add(std::make_unique<juce::AudioParameterFloat>(
			juce::ParameterID{ ParameterIDs::FlangerWidth, 1 },
			"Stereo Width",
			juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
			flangerParams.Width * 100));
		layout.add(std::make_unique<juce::AudioParameterFloat>(
			juce::ParameterID{ ParameterIDs::FlangerFeedback, 1 },
			"Feedback",
			juce::NormalisableRange<float>(-95.0f, 95.0f, 0.1f),
			flangerParams.Feedback * 100));
		layout.add(std::make_unique<juce::AudioParameterFloat>(
			juce::ParameterID{ ParameterIDs::FlangerBaseDelay, 1 },
			"Base Delay (ms)",
			juce::NormalisableRange<float>(0.2f, 5.0f, 0.01f, 0.1f),
			flangerParams.BaseDelayMs));
		
		layout.add(std::make_unique<juce::AudioParameterFloat>(
			juce::ParameterID{ ParameterIDs::PhaserRate, 1 },
			"Rate (Hz)",
			juce::NormalisableRange<float>(0.05f, 5.0f, 0.05f, 0.35f),
			phaserParams.RateHz));
		layout.add(std::make_unique<juce::AudioParameterFloat>(
			juce::ParameterID{ ParameterIDs::PhaserDepth, 1 },
			"Depth",
			juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
			phaserParams.Depth * 100));
		layout.add(std::make_unique<juce::AudioParameterFloat>(
			juce::ParameterID{ ParameterIDs::PhaserWidth, 1 },
			"Stereo Width",
			juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
			phaserParams.Width * 100));
		layout.add(std::make_unique<juce::AudioParameterInt>(
			juce::ParameterID{ ParameterIDs::PhaserStages, 1 },
			"Stages",
			2, 12, phaserParams.Stages));
		layout.add(std::make_unique<juce::AudioParameterFloat>(
			juce::ParameterID{ ParameterIDs::PhaserFeedback, 1 },
			"Feedback",
			juce::NormalisableRange<float>(-95.0f, 95.0f, 0.1f),
			phaserParams.Feedback * 100));
		
		layout.add(std::make_unique<juce::AudioParameterFloat>(
			juce::ParameterID{ ParameterIDs::VibeRate, 1 },
			"Rate (Hz)",
			juce::NormalisableRange<float>(0.05f, 5.0f, 0.05f, 0.35f),
			vibeParams.RateHz));
		layout.add(std::make_unique<juce::AudioParameterFloat>(
			juce::ParameterID{ ParameterIDs::VibeDepth, 1 },
			"Depth",
			juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
			vibeParams.Depth * 100));
		layout.add(std::make_unique<juce::AudioParameterFloat>(
			juce::ParameterID{ ParameterIDs::VibeWidth, 1 },
			"Stereo Width",
			juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
			vibeParams.Width * 100));
		layout.add(std::make_unique<juce::AudioParameterBool>(
			juce::ParameterID{ ParameterIDs::VibeMode, 1 },
			"Vibrato",
			vibeParams.Vibrato));

		return layout;
	}
}