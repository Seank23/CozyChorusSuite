#pragma once
#include "../PluginProcessor.h"
#include "CCSLookAndFeel.h"

#include <juce_audio_processors/juce_audio_processors.h>

namespace CozyChorus
{
	class LabeledKnob;
	class ModulationVisualiser;

	class CCSAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
	{
	public:
		explicit CCSAudioProcessorEditor(juce::AudioProcessor& processor);
		~CCSAudioProcessorEditor() override;

		void paint(juce::Graphics& graphics) override;
		void resized() override;

	private:
		void timerCallback() override;
		void HideAllEffectComponents();
		void RenderComponents();
		std::vector<juce::Component*> GetActiveComponents();

		CCSLookAndFeel m_LookAndFeel;

		std::unique_ptr<ModulationVisualiser> m_ModulationVisualiser;

		using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
		using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
		using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

		PluginProcessor& m_Processor;
		juce::AudioProcessorValueTreeState& m_APVTS;

		juce::ComboBox m_EffectSelector;
		std::unique_ptr<LabeledKnob> m_MixKnob, m_WarmthKnob, m_AgeKnob;

		std::unique_ptr<LabeledKnob> m_ChorusRateKnob, m_ChorusDepthKnob, m_ChorusWidthKnob, m_ChorusVoicesKnob;

		std::unique_ptr<LabeledKnob> m_FlangerRateKnob, m_FlangerDepthKnob, m_FlangerWidthKnob, m_FlangerFeedbackKnob, m_FlangerBaseDelayKnob;

		std::unique_ptr<LabeledKnob> m_PhaserRateKnob, m_PhaserDepthKnob, m_PhaserWidthKnob, m_PhaserStagesKnob, m_PhaserFeedbackKnob;

		std::unique_ptr<LabeledKnob> m_VibeRateKnob, m_VibeDepthKnob, m_VibeWidthKnob;
		juce::ToggleButton m_VibeModeButton;

		// Attachments — DECLARED AFTER the components so they destruct FIRST.
		std::unique_ptr<ComboBoxAttachment> m_EffectAttachment;
		std::unique_ptr<SliderAttachment> m_MixAtt, m_WarmthAtt, m_AgeAtt;
		std::unique_ptr<SliderAttachment> m_ChorusRateAtt, m_ChorusDepthAtt, m_ChorusWidthAtt, m_ChorusVoicesAtt;
		std::unique_ptr<SliderAttachment> m_FlangerRateAtt, m_FlangerDepthAtt, m_FlangerWidthAtt, m_FlangerFeedbackAtt, m_FlangerBaseDelayAtt;
		std::unique_ptr<SliderAttachment> m_PhaserRateAtt, m_PhaserDepthAtt, m_PhaserWidthAtt, m_PhaserStagesAtt, m_PhaserFeedbackAtt;
		std::unique_ptr<SliderAttachment> m_VibeRateAtt, m_VibeDepthAtt, m_VibeWidthAtt;
		std::unique_ptr<ButtonAttachment> m_VibeModeAtt;

		juce::Rectangle<int> m_CharacterZone;

		int m_LastEffectIndex = -1;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CCSAudioProcessorEditor)
	};
}