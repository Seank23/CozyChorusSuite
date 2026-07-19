#include "PhaserEffect.h"

namespace CozyChorus
{
	PhaserEffect::PhaserEffect()
	{
	}

	PhaserEffect::~PhaserEffect()
	{
	}

	void PhaserEffect::Prepare(const juce::dsp::ProcessSpec& spec)
	{
		m_SampleRate = spec.sampleRate;

		m_LFO.Prepare(m_SampleRate);

		const float logMin = std::log(MIN_FC_HZ);
		const float logMax = std::log(MAX_FC_HZ);
		m_LogCenter = 0.5f * (logMin + logMax);
		m_LogHalfSpan = 0.5f * (logMax - logMin);

		for (auto* smoothedVal : { &m_RateHz, &m_Depth, &m_Mix, &m_Width, &m_Feedback })
			smoothedVal->reset(m_SampleRate, 0.02);

		for (auto& channelState : m_AllPassState)
			channelState.fill(0.0f);
		m_FeedbackState.fill(0.0f);
	}

	void PhaserEffect::Process(const juce::dsp::ProcessContextReplacing<float>& context)
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
			float feedback = m_Feedback.getNextValue();
			float widthOffset = width * 0.25f;

			for (int ch = 0; ch < numChannels; ch++)
			{
				float channelSample = block.getChannelPointer(ch)[n];

				float channelWidthOffset = (ch == 0) ? 0.0f : widthOffset;
				float lfo = m_LFO.GetValue(channelWidthOffset);

				float logFreqCutoff = m_LogCenter + m_LogHalfSpan * depth * lfo;
				float freqCutoff = std::clamp(std::exp(logFreqCutoff), MIN_FC_HZ, MAX_FC_HZ);

				float g = std::tan(juce::MathConstants<float>::pi * freqCutoff / static_cast<float>(m_SampleRate));
				float G = g / (1.0f + g);

				float allPassInput = channelSample + m_FeedbackState[ch] * feedback;

				for (int s = 0; s < m_Stages; s++)
				{
					float& state = m_AllPassState[ch][s];
					float highPass = (allPassInput - state) * G;
					float lowPass = highPass + state;
					m_AllPassState[ch][s] = lowPass + highPass;
					allPassInput = 2.0f * lowPass - allPassInput;
				}
				m_FeedbackState[ch] = allPassInput;
				float wetSample = allPassInput;

				block.getChannelPointer(ch)[n] = channelSample * (1.0f - mix) + wetSample * mix;
			}
			m_LFO.Advance();
		}
	}

	void PhaserEffect::Reset()
	{
		for (auto& channelState : m_AllPassState)
			channelState.fill(0.0f);
		m_FeedbackState.fill(0.0f);
		for (auto* smoothedVal : { &m_RateHz, &m_Depth, &m_Mix, &m_Width, &m_Feedback })
			smoothedVal->reset(m_SampleRate, 0.02);
	}

	void PhaserEffect::SetParameters(const PhaserParameters& params)
	{
		m_RateHz.setTargetValue(params.RateHz);
		m_Depth.setTargetValue(params.Depth);
		m_Mix.setTargetValue(params.Mix);
		m_Width.setTargetValue(params.Width);
		m_Feedback.setTargetValue(params.Feedback);
		m_Stages = std::clamp(params.Stages, 2, MAX_STAGES);
	}
}