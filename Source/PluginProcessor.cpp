#include "PluginProcessor.h"

namespace CozyChorus
{
	PluginProcessor::PluginProcessor()
		: juce::AudioProcessor(BusesProperties()
								   .withInput("Input", juce::AudioChannelSet::stereo(), true)
								   .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
		  m_APVTS(*this, nullptr, "PARAMETERS", CreateParameterLayout())
	{
		m_EffectTypeParam = m_APVTS.getRawParameterValue(ParameterIDs::EffectType);
	}

	void PluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
	{
		juce::dsp::ProcessSpec spec{};
		spec.sampleRate = sampleRate;
		spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
		spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());

		m_NullEffect.Prepare(spec);
	}

	void PluginProcessor::releaseResources()
	{
		m_NullEffect.Reset();
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
		case EffectType::Flanger:
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

		GetActiveEffect().Process(context);
	}

	juce::AudioProcessorEditor* PluginProcessor::createEditor()
	{
		return new juce::GenericAudioProcessorEditor(*this);
	}

	void PluginProcessor::getStateInformation(juce::MemoryBlock& destData)
	{
		if (auto xml = m_APVTS.copyState().createXml())
			copyXmlToBinary(*xml, destData);
	}

	void PluginProcessor::setStateInformation(const void* data, int sizeInBytes)
	{
		if (auto xml = getXmlFromBinary(data, sizeInBytes))
			if (xml->hasTagName(m_APVTS.state.getType()))
				m_APVTS.replaceState(juce::ValueTree::fromXml(*xml));
	}
}

// JUCE plugin entry point — must live in the global namespace.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new CozyChorus::PluginProcessor();
}