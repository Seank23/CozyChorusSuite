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
		using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

		PluginProcessor& m_Processor;
		juce::AudioProcessorValueTreeState& m_APVTS; // = m_Processor.GetAPVTS()

		juce::ComboBox m_EffectSelector;
		juce::Slider m_MixSlider;

		juce::Slider m_ChorusRateSlider, m_ChorusDepthSlider, m_ChorusWidthSlider, m_ChorusVoicesSlider;

		juce::Slider m_FlangerRateSlider, m_FlangerDepthSlider, m_FlangerWidthSlider, m_FlangerFeedbackSlider, m_FlangerBaseDelaySlider;

		juce::Slider m_PhaserRateSlider, m_PhaserDepthSlider, m_PhaserWidthSlider, m_PhaserStagesSlider, m_PhaserFeedbackSlider;

		juce::Slider m_VibeRateSlider, m_VibeDepthSlider, m_VibeWidthSlider;
		juce::ToggleButton m_VibeModeButton;

		// Attachments — DECLARED AFTER the components so they destruct FIRST.
		std::unique_ptr<ComboBoxAttachment> m_EffectAttachment;
		std::unique_ptr<SliderAttachment> m_MixAtt;
		std::unique_ptr<SliderAttachment> m_ChorusRateAtt, m_ChorusDepthAtt, m_ChorusWidthAtt, m_ChorusVoicesAtt;
		std::unique_ptr<SliderAttachment> m_FlangerRateAtt, m_FlangerDepthAtt, m_FlangerWidthAtt, m_FlangerFeedbackAtt, m_FlangerBaseDelayAtt;
		std::unique_ptr<SliderAttachment> m_PhaserRateAtt, m_PhaserDepthAtt, m_PhaserWidthAtt, m_PhaserStagesAtt, m_PhaserFeedbackAtt;
		std::unique_ptr<SliderAttachment> m_VibeRateAtt, m_VibeDepthAtt, m_VibeWidthAtt;
		std::unique_ptr<ButtonAttachment> m_VibeModeAtt;

		int m_LastEffectIndex = -1;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CCSAudioProcessorEditor)
	};
}