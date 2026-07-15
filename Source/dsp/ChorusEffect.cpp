#include "ChorusEffect.h"

namespace CozyChorus
{
	ChorusEffect::ChorusEffect()
	{
	}

	ChorusEffect::~ChorusEffect()
	{
	}

	void ChorusEffect::Prepare(const juce::dsp::ProcessSpec& spec)
	{
		m_SampleRate = spec.sampleRate;

		m_MaxDelaySamples = static_cast<int>(MAX_DELAY_MS * 0.001 * m_SampleRate) + 4;
		m_DelayLine.setMaximumDelayInSamples(m_MaxDelaySamples);
		m_DelayLine.prepare(spec);

		m_LFO.Prepare(m_SampleRate);

		for (auto* smoothedVal : { &m_RateHz, &m_Depth, &m_Mix, &m_Width })
			smoothedVal->reset(spec.sampleRate, 0.02);
	}

	void ChorusEffect::Process(const juce::dsp::ProcessContextReplacing<float>& context)
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

			float baseSample = m_BaseDelayMs * 0.001 * m_SampleRate;
			float depthSample = depth * MAX_DEPTH_MS * 0.001 * m_SampleRate;
			float widthOffset = width * 0.25f;

			for (int ch = 0; ch < numChannels; ch++)
			{
				float channelLFO = m_LFO.GetValue(ch == 0 ? 0.0f : widthOffset);
				float delaySample = std::clamp(baseSample + channelLFO * depthSample, 0.0f, static_cast<float>(m_MaxDelaySamples - 1));

				float channelSample = block.getChannelPointer(ch)[n];
				m_DelayLine.pushSample(ch, channelSample);
				float wetSample = m_DelayLine.popSample(ch, delaySample);
				block.getChannelPointer(ch)[n] = channelSample * (1.0f - mix) + wetSample * mix;
			}
			m_LFO.Advance();
		}
	}

	void ChorusEffect::Reset()
	{
		m_DelayLine.reset();
		m_LFO.Reset();
		for (auto* smoothedVal : { &m_RateHz, &m_Depth, &m_Mix, &m_Width })
			smoothedVal->reset(m_SampleRate, 0.02);
	}

	void ChorusEffect::SetParameters(const ChorusParameters& params)
	{
		m_RateHz.setTargetValue(params.RateHz);
		m_Mix.setTargetValue(params.Mix);
		m_Depth.setTargetValue(params.Depth);
		m_Width.setTargetValue(params.Width);
	}
}