#include "CharacterStage.h"

namespace CozyChorus
{
	CharacterStage::CharacterStage()
	{
	}

	CharacterStage::~CharacterStage()
	{
	}

	void CharacterStage::Prepare(const juce::dsp::ProcessSpec& spec)
	{
		m_SampleRate = spec.sampleRate;
		m_Warmth.reset(m_SampleRate, 0.02);

		m_Oversampler = std::make_unique<juce::dsp::Oversampling<float>>(
			spec.numChannels,
			OS_FACTOR_LOG2,
			juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
			/*isMaximumQuality*/ true);
		m_Oversampler->initProcessing(spec.maximumBlockSize);

		m_WarmthFilter.prepare(spec);
		m_WarmthFilter.setType(juce::dsp::FirstOrderTPTFilterType::lowpass);
		m_WarmthFilter.setCutoffFrequency(MAX_WARMTH_CUTOFF_HZ);
	}

	void CharacterStage::Process(const juce::dsp::ProcessContextReplacing<float>& context)
	{
		auto&& block = context.getOutputBlock();
		const int numSamples = block.getNumSamples();

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
		m_Warmth.reset(m_SampleRate, 0.02);
	}

	void CharacterStage::SetParameters(const CharacterStageParameters& params)
	{
		m_Warmth.setTargetValue(params.Warmth);
	}
}