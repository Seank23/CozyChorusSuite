#pragma once
#include "ModulationEffect.h"
#include "LFO.h"

namespace CozyChorus
{
	struct PhaserParameters
	{
		float RateHz = 0.5f;
		float Depth = 0.5f;
		float Mix = 0.5f;
		float Width = 0.5f;
		float Feedback = 0.0f;
		int Stages = 6;
	};

	class PhaserEffect : public ModulationEffect
	{
	public:
		PhaserEffect();
		~PhaserEffect();

		virtual void Prepare(const juce::dsp::ProcessSpec& spec) override;
		virtual void Process(const juce::dsp::ProcessContextReplacing<float>& context) override;
		virtual void Reset() override;

		void SetParameters(const PhaserParameters& params);

	private:
		LFO m_LFO;

		juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> m_RateHz, m_Depth, m_Mix, m_Width, m_Feedback;
		int m_Stages = 6;
		double m_SampleRate = 44100.0;

		float m_LogCenter = 0.0f;
		float m_LogHalfSpan = 0.0f;

		static constexpr int MAX_STAGES = 12;
		static constexpr int MAX_CHANNELS = 2;
		static constexpr float MIN_FC_HZ = 200.0f;
		static constexpr float MAX_FC_HZ = 2000.0f;

		std::array<std::array<float, MAX_STAGES>, MAX_CHANNELS> m_AllPassState{};
		std::array<float, MAX_CHANNELS> m_FeedbackState{};
	};
}