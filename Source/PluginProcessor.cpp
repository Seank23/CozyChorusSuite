#include "PluginProcessor.h"
#include "Editor/CCSAudioProcessorEditor.h"

namespace CozyChorus
{
	PluginProcessor::PluginProcessor()
		: juce::AudioProcessor(BusesProperties()
								   .withInput("Input", juce::AudioChannelSet::stereo(), true)
								   .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
		  m_APVTS(*this, nullptr, "PARAMETERS", CreateParameterLayout())
	{
		m_EffectTypeParam = m_APVTS.getRawParameterValue(ParameterIDs::EffectType);
		m_RateParam = m_APVTS.getRawParameterValue(ParameterIDs::Rate);
		m_DepthParam = m_APVTS.getRawParameterValue(ParameterIDs::Depth);
		m_MixParam = m_APVTS.getRawParameterValue(ParameterIDs::Mix);
		m_WidthParam = m_APVTS.getRawParameterValue(ParameterIDs::Width);
		m_ChorusVoicesParam = m_APVTS.getRawParameterValue(ParameterIDs::ChorusVoices);
		m_FlangerFeedbackParam = m_APVTS.getRawParameterValue(ParameterIDs::FlangerFeedback);
		m_FlangerBaseDelayParam = m_APVTS.getRawParameterValue(ParameterIDs::FlangerBaseDelay);
	}

	void PluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
	{
		juce::dsp::ProcessSpec spec{};
		spec.sampleRate = sampleRate;
		spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
		spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());

		m_NullEffect.Prepare(spec);
		m_ChorusEffect.Prepare(spec);
		m_FlangerEffect.Prepare(spec);
	}

	void PluginProcessor::releaseResources()
	{
		m_NullEffect.Reset();
		m_ChorusEffect.Reset();
		m_FlangerEffect.Reset();
	}

	bool PluginProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
	{
		const auto mainOutput = layouts.getMainOutputChannelSet();
		const auto mainInput = layouts.getMainInputChannelSet();

		if (mainOutput != juce::AudioChannelSet::mono() && mainOutput != juce::AudioChannelSet::stereo())
			return false;

		// Require the input to match the output (mono->mono or stereo->stereo).
		return mainInput == mainOutput;
	}

	ModulationEffect& PluginProcessor::GetActiveEffect()
	{
		// Read the selection lock-free. Every type maps to the pass-through for now;
		// later milestones return the matching effect from these cases.
		const auto type = static_cast<EffectType>(static_cast<int>(m_EffectTypeParam->load()));

		switch (type)
		{
		case EffectType::Chorus:
			return m_ChorusEffect;
		case EffectType::Flanger:
			return m_FlangerEffect;
		case EffectType::Phaser:
		case EffectType::Vibe:
		default:
			return m_NullEffect;
		}
	}

	void PluginProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
	{
		juce::ScopedNoDenormals noDenormals;

		// Clear any output-only channels that carry no input.
		for (int channel = getTotalNumInputChannels(); channel < getTotalNumOutputChannels(); ++channel)
			buffer.clear(channel, 0, buffer.getNumSamples());

		juce::dsp::AudioBlock<float> block(buffer);
		juce::dsp::ProcessContextReplacing<float> context(block);

		const auto type = static_cast<EffectType>(static_cast<int>(m_EffectTypeParam->load()));

		switch (type)
		{
		case EffectType::Chorus:
		{
			ChorusParameters params{};
			params.RateHz = m_RateParam->load();
			params.Depth = std::clamp(m_DepthParam->load() / 100.0f, 0.0f, 1.0f);
			params.Mix = std::clamp(m_MixParam->load() / 100.0f, 0.0f, 1.0f);
			params.Width = std::clamp(m_WidthParam->load() / 100.0f, 0.0f, 1.0f);
			params.Voices = static_cast<int>(m_ChorusVoicesParam->load());
			m_ChorusEffect.SetParameters(params);
			break;
		}
		case EffectType::Flanger:
		{
			FlangerParameters params{};
			params.RateHz = m_RateParam->load();
			params.Depth = std::clamp(m_DepthParam->load() / 100.0f, 0.0f, 1.0f);
			params.Mix = std::clamp(m_MixParam->load() / 100.0f, 0.0f, 1.0f);
			params.Width = std::clamp(m_WidthParam->load() / 100.0f, 0.0f, 1.0f);
			params.Feedback = std::clamp(m_FlangerFeedbackParam->load() / 100.0f, -0.95f, 0.95f);
			params.BaseDelayMs = m_FlangerBaseDelayParam->load();
			m_FlangerEffect.SetParameters(params);
			break;
		}
		}
		

		GetActiveEffect().Process(context);
	}

	juce::AudioProcessorEditor* PluginProcessor::createEditor()
	{
		return new CCSAudioProcessorEditor(*this);
	}

	void PluginProcessor::getStateInformation(juce::MemoryBlock& destData)
	{
		if (auto xml = m_APVTS.copyState().createXml())
			copyXmlToBinary(*xml, destData);
	}

	void PluginProcessor::setStateInformation(const void* data, int sizeInBytes)
	{
		if (auto xml = getXmlFromBinary(data, sizeInBytes))
		{
			if (xml->hasTagName(m_APVTS.state.getType()))
				m_APVTS.replaceState(juce::ValueTree::fromXml(*xml));
		}
	}
}

// JUCE plugin entry point — must live in the global namespace.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new CozyChorus::PluginProcessor();
}