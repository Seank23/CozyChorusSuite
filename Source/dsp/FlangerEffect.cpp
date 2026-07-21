#include "FlangerEffect.h"

namespace CozyChorus
{
	FlangerEffect::FlangerEffect()
	{
	}

	FlangerEffect::~FlangerEffect()
	{
	}

	void FlangerEffect::Prepare(const juce::dsp::ProcessSpec& spec)
	{
		m_SampleRate = spec.sampleRate;

		m_MaxDelaySamples = static_cast<int>(MAX_DELAY_MS * 0.001 * m_SampleRate) + 4;
		m_DelayLine.setMaximumDelayInSamples(m_MaxDelaySamples);
		m_DelayLine.prepare(spec);

		m_LFO.Prepare(m_SampleRate);

		for (auto* smoothedVal : { &m_RateHz, &m_Depth, &m_Mix, &m_Width, &m_Feedback, &m_BaseDelayMs })
			smoothedVal->reset(spec.sampleRate, 0.02);

		SetParameters(FlangerParameters{});
	}

	void FlangerEffect::Process(const juce::dsp::ProcessContextReplacing<float>& context)
	{
		if (context.isBypassed)
			return;

		auto&& block = context.getOutputBlock();
		const int numChannels = block.getNumChannels();
		const int numSamples = block.getNumSamples();

		m_LFO.SetFrequency(m_RateHz.getNextValue());

		for (int n = 0; n < numSamples; n++)
		{
			float depth = m_Depth.getNextValue();
			float mix = m_Mix.getNextValue();
			float width = m_Width.getNextValue();
			float feedback = m_Feedback.getNextValue();
			float baseDelay = m_BaseDelayMs.getNextValue();

			float baseSample = baseDelay * 0.001f * m_SampleRate;
			float sweepSample = depth * MAX_SWEEP_MS * 0.001 * m_SampleRate;
			float widthOffset = width * 0.25f;

			for (int ch = 0; ch < numChannels; ch++)
			{
				float channelSample = block.getChannelPointer(ch)[n];

				float channelWidthOffset = (ch == 0) ? 0.0f : widthOffset;
				float lfo = m_LFO.GetValue(channelWidthOffset);
				float lfoNormalised = 0.5f + 0.5f * lfo;
				float delaySample = std::clamp(baseSample + sweepSample * lfoNormalised, MIN_DELAY_SAMPLES, static_cast<float>(m_MaxDelaySamples - 1));

				float wetSample = m_DelayLine.popSample(ch, delaySample, true);
				m_DelayLine.pushSample(ch, channelSample + feedback * wetSample);

				block.getChannelPointer(ch)[n] = channelSample * (1.0f - mix) + wetSample * mix;
			}
			m_LFO.Advance();
		}
	}

	void FlangerEffect::Reset()
	{
		m_DelayLine.reset();
		m_LFO.Reset();
		for (auto* smoothedVal : { &m_RateHz, &m_Depth, &m_Mix, &m_Width, &m_Feedback, &m_BaseDelayMs })
			smoothedVal->reset(m_SampleRate, 0.02);
	}

	void FlangerEffect::SetParameters(const FlangerParameters& params)
	{
		m_RateHz.setTargetValue(params.RateHz);
		m_Mix.setTargetValue(params.Mix);
		m_Depth.setTargetValue(params.Depth);
		m_Width.setTargetValue(params.Width);
		m_Feedback.setTargetValue(params.Feedback);
		m_BaseDelayMs.setTargetValue(params.BaseDelayMs);
	}
}