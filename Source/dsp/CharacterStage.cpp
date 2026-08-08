#include "CharacterStage.h"

namespace CozyChorus
{
	void CharacterStage::Prepare(const juce::dsp::ProcessSpec& spec)
	{
		m_SampleRate = spec.sampleRate;
		for (auto* smoothedVal : { &m_Warmth, &m_Age })
			smoothedVal->reset(spec.sampleRate, 0.02);

		m_WowLFO.Prepare(m_SampleRate);
		m_FlutterLFO.Prepare(m_SampleRate);
		m_WowLFO.SetFrequency(WOW_FREQUENCY);
		m_FlutterLFO.SetFrequency(FLUTTER_FREQUENCY);
		m_Random.setSeed(0x12345678);

		m_NoiseCoeff = 1.0f - std::exp(-2.0f * juce::MathConstants<float>::pi * NOISE_LPF_HZ / m_SampleRate);

		m_WowDelayLine.setMaximumDelayInSamples(static_cast<int>((CENTER_DELAY_MS + HALF_SPAN_DELAY_MS) * 0.001 * m_SampleRate) + 4);
		m_WowDelayLine.prepare(spec);

		m_Oversampler = std::make_unique<juce::dsp::Oversampling<float>>(
			spec.numChannels,
			OS_FACTOR_LOG2,
			juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
			true,
			true);
		m_Oversampler->initProcessing(spec.maximumBlockSize);

		m_WarmthFilter.prepare(spec);
		m_WarmthFilter.setType(juce::dsp::FirstOrderTPTFilterType::lowpass);
		m_WarmthFilter.setCutoffFrequency(MAX_WARMTH_CUTOFF_HZ);
	}

	void CharacterStage::Process(const juce::dsp::ProcessContextReplacing<float>& context)
	{
		if (context.isBypassed)
			return;

		auto&& block = context.getOutputBlock();
		const int numSamples = block.getNumSamples();
		const int numChannels = block.getNumChannels();

		for (int n = 0; n < numSamples; n++)
		{
			float noise = m_Random.nextFloat() * 2.0f - 1.0f;
			m_NoiseState += m_NoiseCoeff * (noise - m_NoiseState);
			float noiseMakeup = m_NoiseState * (1 / std::sqrt(m_NoiseCoeff / 2.0f));

			float summedModulation = WOW_WEIGHT * m_WowLFO.GetValue() + FLUTTER_WEIGHT * m_FlutterLFO.GetValue() + NOISE_WEIGHT * noiseMakeup;
			float delay = (CENTER_DELAY_MS + HALF_SPAN_DELAY_MS * summedModulation * m_Age.getNextValue()) * 0.001f * m_SampleRate;
			m_WowDelayLine.setDelay(delay);

			for (int ch = 0; ch < numChannels; ch++)
			{
				float sample = block.getChannelPointer(ch)[n];
				m_WowDelayLine.pushSample(ch, sample);
				block.getChannelPointer(ch)[n] = m_WowDelayLine.popSample(ch);
			}
			m_WowLFO.Advance();
			m_FlutterLFO.Advance();
		}

		float warmth = m_Warmth.getNextValue();
		m_Warmth.skip(std::max(numSamples - 1, 0));

		float drive = 1 + warmth * DRIVE_MAX;
		float tanhDb = std::tanh(drive * DC_BIAS);
		float invS0 = 1 / (drive * (1 - tanhDb * tanhDb));
		float freqCutoff = MAX_WARMTH_CUTOFF_HZ * std::pow(MIN_WARMTH_CUTOFF_HZ / MAX_WARMTH_CUTOFF_HZ, warmth);

		auto upSampled = m_Oversampler->processSamplesUp(block);
		const int osChannels = (int)upSampled.getNumChannels();
		const int osSamples = (int)upSampled.getNumSamples();
		for (int ch = 0; ch < osChannels; ch++)
		{
			float* channelPtr = upSampled.getChannelPointer(ch);
			for (int n = 0; n < osSamples; n++)
				channelPtr[n] = (std::tanh(drive * (channelPtr[n] + DC_BIAS)) - tanhDb) * invS0;
		}
		m_Oversampler->processSamplesDown(block);

		m_WarmthFilter.setCutoffFrequency(freqCutoff);
		m_WarmthFilter.process(juce::dsp::ProcessContextReplacing<float>(block));
	}

	void CharacterStage::Reset()
	{
		if (m_Oversampler)
			m_Oversampler->reset();
		m_WarmthFilter.reset();
		m_WowDelayLine.reset();
		m_WowLFO.Reset();
		m_FlutterLFO.Reset();
		m_NoiseState = 0.0f;
		for (auto* smoothedVal : { &m_Warmth, &m_Age })
			smoothedVal->reset(m_SampleRate, 0.02);
	}

	void CharacterStage::SetParameters(const CharacterStageParameters& params)
	{
		m_Warmth.setTargetValue(params.Warmth);
		m_Age.setTargetValue(params.Age);
	}
}