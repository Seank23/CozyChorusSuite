#pragma once
#include "../PluginProcessor.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace CozyChorus
{
	class ModulationVisualiser : public juce::Component
	{
	public:
		enum class Mode
		{
			LFO,
			Response
		};

		ModulationVisualiser(PluginProcessor& processor);
		~ModulationVisualiser() override;

		void paint(juce::Graphics& g) override;
		void mouseDown(const juce::MouseEvent& event) override;

		void SetMode(Mode mode);
		Mode GetMode() const { return m_Mode; }

	private:
		void UpdateVisualisation();
		void PaintLFOResponse(juce::Graphics& g, juce::Rectangle<int> area);
		void PaintDelayResponse(juce::Graphics& g, juce::Rectangle<int> area, EffectType type);
		void PaintSpectrumResponse(juce::Graphics& g, juce::Rectangle<int> area, EffectType type);

		float SampleShape(float phase, EffectType type);
		float SampleDepth(EffectType type);

		void EvaluateCutoffFrequency(EffectType type);
		float EvaluateTransferFunction(float frequency, EffectType type);

		bool IsDelayEffect(EffectType type) const { return type == EffectType::Chorus || type == EffectType::Flanger; }

		PluginProcessor& m_Processor;
		juce::AudioProcessorValueTreeState& m_APVTS;
		Mode m_Mode = Mode::Response;

		double m_SampleRate = 44100.0;
		float m_Phase = 0.0f;
		float m_DelayMs = 0.0f;
		std::vector<float> m_Cutoffs;

		juce::VBlankAttachment m_VBlankAttachment;

		static constexpr float MIN_MAG_DB = -40.0f;
		static constexpr float MAX_MAG_DB = 20.0f;
		static constexpr float MIN_FREQUENCY = 50.0f;
		static constexpr float MAX_FREQUENCY = 20000.0f;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulationVisualiser)
	};
}