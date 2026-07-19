#pragma once
#include "../PluginProcessor.h"
#include <juce_audio_processors/juce_audio_processors.h>

namespace CozyChorus
{
	class CCSAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
	{
	public:
		explicit CCSAudioProcessorEditor(juce::AudioProcessor& processor);
		~CCSAudioProcessorEditor() override;

		void paint(juce::Graphics& graphics) override;
		void resized() override;

	private:
		void timerCallback() override;
		void UpdateVisibility();
		void RenderComponents();
		std::vector<juce::Component*> GetAllComponents();

		using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
		using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

		PluginProcessor& m_Processor;
		juce::AudioProcessorValueTreeState& m_APVTS; // = m_Processor.GetAPVTS()

		juce::ComboBox m_EffectSelector;

		juce::Slider m_RateSlider, m_DepthSlider, m_MixSlider, m_WidthSlider;

		juce::Slider m_VoicesSlider;

		juce::Slider m_FlangerFeedbackSlider, m_BaseDelaySlider;

		juce::Slider m_StagesSlider, m_PhaserFeedbackSlider;

		// Attachments — DECLARED AFTER the components so they destruct FIRST.
		std::unique_ptr<ComboBoxAttachment> m_EffectAttachment;
		std::unique_ptr<SliderAttachment> m_RateAtt, m_DepthAtt, m_MixAtt, m_WidthAtt;
		std::unique_ptr<SliderAttachment> m_VoicesAtt, m_FlangerFeedbackAtt, m_BaseDelayAtt;
		std::unique_ptr<SliderAttachment> m_StagesAtt, m_PhaserFeedbackAtt;

		int m_LastEffectIndex = -1;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CCSAudioProcessorEditor)
	};
}