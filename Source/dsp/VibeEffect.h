#pragma once
#include "ModulationEffect.h"
#include "LFO.h"

namespace CozyChorus
{
	struct VibeParameters
	{
		float RateHz = 0.5f;
		float Depth = 0.5f;
		float Mix = 0.5f;
		float Width = 0.5f;
		bool Vibrato = false;
	};

	class VibeEffect : public ModulationEffect
	{
	public:
		VibeEffect();
		~VibeEffect();

		virtual void Prepare(const juce::dsp::ProcessSpec& spec) override;
		virtual void Process(const juce::dsp::ProcessContextReplacing<float>& context) override;
		virtual void Reset() override;

		void SetParameters(const VibeParameters& params);

	private:
		float GetAsymmetricShape(float phase);

		LFO m_LFO;

		juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> m_RateHz, m_Depth, m_Mix, m_Width;
		bool m_Vibrato = false;
		double m_SampleRate = 44100.0;

		float m_LogCenter = 0.0f;
		float m_LogHalfSpan = 0.0f;

		static constexpr int NUM_STAGES = 4;
		static constexpr int MAX_CHANNELS = 2;
		static constexpr float MIN_FC_HZ = 200.0f;
		static constexpr float MAX_FC_HZ = 2000.0f;
		static constexpr float ASYM_K = 0.35f;

		std::array<float, NUM_STAGES> m_StageLogOffset{};
		std::array<std::array<float, NUM_STAGES>, MAX_CHANNELS> m_AllPassState{};
	};
}