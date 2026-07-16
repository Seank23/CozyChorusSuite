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

		for (auto* smoothedVal : { &m_RateHz, &m_Depth, &m_Mix, &m_Width, &m_Voices })
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
			int voices = static_cast<int>(m_Voices.getNextValue());
			float depthSample = depth * MAX_DEPTH_MS * 0.001 * m_SampleRate;
			float widthOffset = width * 0.25f;

			for (int ch = 0; ch < numChannels; ch++)
			{
				float channelSample = block.getChannelPointer(ch)[n];
				m_DelayLine.pushSample(ch, channelSample);

				float wetSum = 0.0f;
				for (int v = 0; v < voices; v++)
				{
					float channelWidthOffset = (ch == 0) ? 0.0f : widthOffset;
					float voicePhase = (static_cast<float>(v) / voices) + channelWidthOffset;
					float lfo = m_LFO.GetValue(voicePhase);
					float voiceBaseMs = m_BaseDelayMs + (v - (voices - 1) * 0.5f) * 4.0f;
					float baseSample = voiceBaseMs * 0.001 * m_SampleRate;
					float delaySample = std::clamp(baseSample + lfo * depthSample, 0.0f, static_cast<float>(m_MaxDelaySamples - 1));
					bool last = (v == voices - 1);
					wetSum += m_DelayLine.popSample(ch, delaySample, last);
				}
				float wetSample = wetSum / voices;
				block.getChannelPointer(ch)[n] = channelSample * (1.0f - mix) + wetSample * mix;
			}
			m_LFO.Advance();
		}
	}

	void ChorusEffect::Reset()
	{
		m_DelayLine.reset();
		m_LFO.Reset();
		for (auto* smoothedVal : { &m_RateHz, &m_Depth, &m_Mix, &m_Width, &m_Voices })
			smoothedVal->reset(m_SampleRate, 0.02);
	}

	void ChorusEffect::SetParameters(const ChorusParameters& params)
	{
		m_RateHz.setTargetValue(params.RateHz);
		m_Mix.setTargetValue(params.Mix);
		m_Depth.setTargetValue(params.Depth);
		m_Width.setTargetValue(params.Width);
		m_Voices.setTargetValue(static_cast<float>(params.Voices));
	}
}