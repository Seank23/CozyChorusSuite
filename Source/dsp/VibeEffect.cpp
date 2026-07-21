#include "VibeEffect.h"

namespace CozyChorus
{
	VibeEffect::VibeEffect()
	{
	}

	VibeEffect::~VibeEffect()
	{
	}

	void VibeEffect::Prepare(const juce::dsp::ProcessSpec& spec)
	{
		m_SampleRate = spec.sampleRate;

		m_LFO.Prepare(m_SampleRate);

		const float logMin = std::log(MIN_FC_HZ);
		const float logMax = std::log(MAX_FC_HZ);
		m_LogCenter = 0.5f * (logMin + logMax);
		m_LogHalfSpan = 0.5f * (logMax - logMin);

		constexpr float ln2 = 0.6931472f;
		const std::array<float, NUM_STAGES> octaveOffset{ -0.75f, -0.25f, 0.25f, 0.75f };
		for (int s = 0; s < NUM_STAGES; ++s)
			m_StageLogOffset[s] = octaveOffset[s] * ln2;

		for (auto* smoothedVal : { &m_RateHz, &m_Depth, &m_Mix, &m_Width })
			smoothedVal->reset(m_SampleRate, 0.02);

		for (auto& channelState : m_AllPassState)
			channelState.fill(0.0f);
	}

	void VibeEffect::Process(const juce::dsp::ProcessContextReplacing<float>& context)
	{
		if (context.isBypassed)
			return;

		auto&& block = context.getOutputBlock();
		const int numChannels = block.getNumChannels();
		const int numSamples = block.getNumSamples();

		m_LFO.SetFrequency(m_RateHz.getNextValue());

		for (int n = 0; n < numSamples; ++n)
		{
			float depth = m_Depth.getNextValue();
			float mix = m_Mix.getNextValue();
			float width = m_Width.getNextValue();
			float widthOffset = width * 0.25f;

			float effectiveMix = m_Vibrato ? 1.0f : mix; 

			for (int ch = 0; ch < numChannels; ch++)
			{
				float channelSample = block.getChannelPointer(ch)[n];

				float channelWidthOffset = (ch == 0) ? 0.0f : widthOffset;
				float phase = m_LFO.GetPhase() + channelWidthOffset;
				float lfo = GetAsymmetricShape(phase);

				float logSweepCenter = m_LogCenter + m_LogHalfSpan * depth * lfo;

				float allPassInput = channelSample;

				for (int s = 0; s < NUM_STAGES; s++)
				{
					float logFreqCutoff = logSweepCenter + m_StageLogOffset[s];
					float freqCutoff = std::clamp(std::exp(logFreqCutoff), MIN_FC_HZ, MAX_FC_HZ);
					float g = std::tan(juce::MathConstants<float>::pi * freqCutoff / static_cast<float>(m_SampleRate));
					float G = g / (1.0f + g);

					float& state = m_AllPassState[ch][s];
					float highPass = (allPassInput - state) * G;
					float lowPass = highPass + state;
					m_AllPassState[ch][s] = lowPass + highPass;
					allPassInput = 2.0f * lowPass - allPassInput;
				}
				float wetSample = allPassInput;

				block.getChannelPointer(ch)[n] = channelSample * (1.0f - effectiveMix) + wetSample * effectiveMix;
			}
			m_LFO.Advance();
		}
	}

	void VibeEffect::Reset()
	{
		for (auto& channelState : m_AllPassState)
			channelState.fill(0.0f);
		for (auto* smoothedVal : { &m_RateHz, &m_Depth, &m_Mix, &m_Width })
			smoothedVal->reset(m_SampleRate, 0.02);
	}

	void VibeEffect::SetParameters(const VibeParameters& params)
	{
		m_RateHz.setTargetValue(params.RateHz);
		m_Depth.setTargetValue(params.Depth);
		m_Mix.setTargetValue(params.Mix);
		m_Width.setTargetValue(params.Width);
		m_Vibrato = params.Vibrato;
	}

	float VibeEffect::GetAsymmetricShape(float phase01)
	{
		float p = phase01 - std::floor(phase01);
		float warped = (p < ASYM_K) ? (0.5f * p / ASYM_K)
									: (0.5f + 0.5f * (p - ASYM_K) / (1.0f - ASYM_K));

		return std::sin(juce::MathConstants<float>::twoPi * warped);
	}
}