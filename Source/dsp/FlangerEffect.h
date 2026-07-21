#pragma once
#include "ModulationEffect.h"

namespace CozyChorus
{
	struct FlangerParameters
	{
		float RateHz = 0.5f;
		float Depth = 0.8f;
		float Mix = 0.5f;
		float Width = 0.5f;
		float Feedback = 0.6f;
		float BaseDelayMs = 0.65f;
	};

	class FlangerEffect : public ModulationEffect
	{
	public:
		FlangerEffect();
		~FlangerEffect();

		virtual void Prepare(const juce::dsp::ProcessSpec& spec) override;
		virtual void Process(const juce::dsp::ProcessContextReplacing<float>& context) override;
		virtual void Reset() override;

		void SetParameters(const FlangerParameters& params);

	private:
		juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> m_DelayLine;

		juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> m_RateHz, m_Depth, m_Mix, m_Width, m_Feedback, m_BaseDelayMs;

		int m_MaxDelaySamples = 0;

		static constexpr float MAX_DELAY_MS = 15.0f;
		static constexpr float MAX_SWEEP_MS = 5.0f;
		static constexpr float MIN_DELAY_SAMPLES = 1.0f;
	};
}