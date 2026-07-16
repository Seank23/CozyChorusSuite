#pragma once
#include "ModulationEffect.h"
#include "LFO.h"

namespace CozyChorus
{
	struct ChorusParameters
	{
		float RateHz = 0.8f;
		float Depth = 0.5f;
		float Mix = 0.5f;
		float Width = 0.5f;
		int Voices = 1;
	};

	class ChorusEffect : public ModulationEffect
	{
	public:
		ChorusEffect();
		~ChorusEffect();

		virtual void Prepare(const juce::dsp::ProcessSpec& spec) override;
		virtual void Process(const juce::dsp::ProcessContextReplacing<float>& context) override;
		virtual void Reset() override;

		void SetParameters(const ChorusParameters& params);

	private:
		juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> m_DelayLine;
		LFO m_LFO;

		juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> m_RateHz, m_Depth, m_Mix, m_Width, m_Voices;

		double m_SampleRate = 44100.0;
		float m_BaseDelayMs = 20.0f;
		int m_MaxDelaySamples = 0;

		static constexpr float MAX_DELAY_MS = 50.0f;
		static constexpr float MAX_DEPTH_MS = 7.0f;
	};
}