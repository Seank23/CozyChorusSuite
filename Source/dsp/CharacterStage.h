#pragma once
#include "LFO.h"

#include <juce_dsp/juce_dsp.h>

namespace CozyChorus
{
	struct CharacterStageParameters
	{
		float Warmth = 0.3f;
		float Age = 0.3f;
	};

	class CharacterStage
	{
	public:
		void Prepare(const juce::dsp::ProcessSpec& spec);
		void Process(const juce::dsp::ProcessContextReplacing<float>& context);
		void Reset();

		void SetParameters(const CharacterStageParameters& params);
		int GetLatencySamples() const { return m_Oversampler ? (int)std::round(m_Oversampler->getLatencyInSamples()) : 0; }

	private:
		juce::dsp::FirstOrderTPTFilter<float> m_WarmthFilter;
		std::unique_ptr<juce::dsp::Oversampling<float>> m_Oversampler;

		LFO m_WowLFO;
		LFO m_FlutterLFO;
		juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> m_WowDelayLine;
		juce::Random m_Random;

		juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> m_Warmth, m_Age;

		double m_SampleRate = 44100.0;
		float m_NoiseCoeff = 0.0f;
		float m_NoiseState = 0.0f;

		static constexpr float MIN_WARMTH_CUTOFF_HZ = 4000.0f;
		static constexpr float MAX_WARMTH_CUTOFF_HZ = 18000.0f;
		static constexpr float DC_BIAS = 0.15f;
		static constexpr size_t OS_FACTOR_LOG2 = 1;
		static constexpr float DRIVE_MAX = 2.5f;

		static constexpr float CENTER_DELAY_MS = 2.0f;
		static constexpr float HALF_SPAN_DELAY_MS = 1.5f;

		static constexpr float WOW_FREQUENCY = 0.556f;
		static constexpr float FLUTTER_FREQUENCY = 12.0f;
		static constexpr float WOW_WEIGHT = 0.8f;
		static constexpr float FLUTTER_WEIGHT = 0.04f;
		static constexpr float NOISE_WEIGHT = 0.02f;
		static constexpr float NOISE_LPF_HZ = 2.0f;
	};
}