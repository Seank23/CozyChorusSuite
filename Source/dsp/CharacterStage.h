#pragma once
#include <juce_dsp/juce_dsp.h>

namespace CozyChorus
{
	struct CharacterStageParameters
	{
		float Warmth = 0.3f;
	};

	class CharacterStage
	{
	public:
		CharacterStage();
		~CharacterStage();

		void Prepare(const juce::dsp::ProcessSpec& spec);
		void Process(const juce::dsp::ProcessContextReplacing<float>& context);
		void Reset();

		void SetParameters(const CharacterStageParameters& params);
		int GetLatencySamples() const { return m_Oversampler ? (int)std::round(m_Oversampler->getLatencyInSamples()) : 0; }

	private:
		juce::dsp::FirstOrderTPTFilter<float> m_WarmthFilter;
		std::unique_ptr<juce::dsp::Oversampling<float>> m_Oversampler;

		juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> m_Warmth;

		float m_SampleRate = 44100.0f;

		static constexpr float MIN_WARMTH_CUTOFF_HZ = 4000.0f;
		static constexpr float MAX_WARMTH_CUTOFF_HZ = 18000.0f;
		static constexpr float DC_BIAS = 0.15f;
		static constexpr size_t OS_FACTOR_LOG2 = 1;
		static constexpr float DRIVE_MAX = 2.5f;
	};
}