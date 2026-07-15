#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#include "Parameters.h"
#include "dsp/NullEffect.h"
#include "dsp/ChorusEffect.h"

#include <atomic>

namespace CozyChorus
{
	// Top-level plugin: owns the APVTS and one instance of each effect, and routes
	// processBlock to the effect chosen by the effectType parameter. M0 ships only the
	// pass-through NullEffect; the real effects are added in later milestones.
	class PluginProcessor : public juce::AudioProcessor
	{
	public:
		PluginProcessor();
		~PluginProcessor() override = default;

		// --- juce::AudioProcessor overrides (JUCE's own naming) ---
		void prepareToPlay(double sampleRate, int samplesPerBlock) override;
		void releaseResources() override;
		bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
		void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) override;

		juce::AudioProcessorEditor* createEditor() override;
		bool hasEditor() const override { return true; }

		const juce::String getName() const override { return JucePlugin_Name; }

		bool acceptsMidi() const override { return false; }
		bool producesMidi() const override { return false; }
		bool isMidiEffect() const override { return false; }
		double getTailLengthSeconds() const override { return 0.0; }

		int getNumPrograms() override { return 1; }
		int getCurrentProgram() override { return 0; }
		void setCurrentProgram(int) override {}
		const juce::String getProgramName(int) override { return {}; }
		void changeProgramName(int, const juce::String&) override {}

		void getStateInformation(juce::MemoryBlock& destData) override;
		void setStateInformation(const void* data, int sizeInBytes) override;

		juce::AudioProcessorValueTreeState& GetAPVTS() { return m_APVTS; }

	private:
		// Selects the effect matching the current effectType parameter.
		ModulationEffect& GetActiveEffect();

		juce::AudioProcessorValueTreeState m_APVTS;

		// One instance per effect will live here; M0 has only the pass-through.
		NullEffect m_NullEffect;
		ChorusEffect m_ChorusEffect;

		// Cached atomic parameter pointer, read lock-free on the audio thread.
		std::atomic<float>* m_EffectTypeParam = nullptr;
		std::atomic<float>* m_RateParam = nullptr;
		std::atomic<float>* m_DepthParam = nullptr;
		std::atomic<float>* m_MixParam = nullptr;
		std::atomic<float>* m_WidthParam = nullptr;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
	};
}