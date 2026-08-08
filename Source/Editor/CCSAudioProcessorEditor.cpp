#include "CCSAudioProcessorEditor.h"
#include "EditorConstants.h"
#include "LabeledKnob.h"
#include "ModulationVisualiser.h"

namespace CozyChorus
{
	CCSAudioProcessorEditor::CCSAudioProcessorEditor(juce::AudioProcessor& processor)
		: juce::AudioProcessorEditor(processor), m_Processor(static_cast<PluginProcessor&>(processor)), m_APVTS(m_Processor.GetAPVTS())
	{
		setLookAndFeel(&m_LookAndFeel);
		addAndMakeVisible(m_EffectSelector);
		m_EffectSelector.addItemList(GetEffectTypeChoices(), 1);

		m_ModulationVisualiser = std::make_unique<ModulationVisualiser>(m_Processor);
		addAndMakeVisible(m_ModulationVisualiser.get());

		m_MixKnob = std::make_unique<LabeledKnob>("Mix");
		addAndMakeVisible(m_MixKnob.get());
		m_AgeKnob = std::make_unique<LabeledKnob>("Age");
		addAndMakeVisible(m_AgeKnob.get());
		m_WarmthKnob = std::make_unique<LabeledKnob>("Warmth");
		addAndMakeVisible(m_WarmthKnob.get());

		m_ChorusRateKnob = std::make_unique<LabeledKnob>("Rate");
		addAndMakeVisible(m_ChorusRateKnob.get());
		m_ChorusDepthKnob = std::make_unique<LabeledKnob>("Depth");
		addAndMakeVisible(m_ChorusDepthKnob.get());
		m_ChorusWidthKnob = std::make_unique<LabeledKnob>("Width");
		addAndMakeVisible(m_ChorusWidthKnob.get());
		m_ChorusVoicesKnob = std::make_unique<LabeledKnob>("Voices");
		addAndMakeVisible(m_ChorusVoicesKnob.get());

		m_FlangerRateKnob = std::make_unique<LabeledKnob>("Rate");
		addAndMakeVisible(m_FlangerRateKnob.get());
		m_FlangerDepthKnob = std::make_unique<LabeledKnob>("Depth");
		addAndMakeVisible(m_FlangerDepthKnob.get());
		m_FlangerWidthKnob = std::make_unique<LabeledKnob>("Width");
		addAndMakeVisible(m_FlangerWidthKnob.get());
		m_FlangerFeedbackKnob = std::make_unique<LabeledKnob>("Feedback");
		addAndMakeVisible(m_FlangerFeedbackKnob.get());
		m_FlangerBaseDelayKnob = std::make_unique<LabeledKnob>("Base Delay");
		addAndMakeVisible(m_FlangerBaseDelayKnob.get());

		m_PhaserRateKnob = std::make_unique<LabeledKnob>("Rate");
		addAndMakeVisible(m_PhaserRateKnob.get());
		m_PhaserDepthKnob = std::make_unique<LabeledKnob>("Depth");
		addAndMakeVisible(m_PhaserDepthKnob.get());
		m_PhaserWidthKnob = std::make_unique<LabeledKnob>("Width");
		addAndMakeVisible(m_PhaserWidthKnob.get());
		m_PhaserStagesKnob = std::make_unique<LabeledKnob>("Stages");
		addAndMakeVisible(m_PhaserStagesKnob.get());
		m_PhaserFeedbackKnob = std::make_unique<LabeledKnob>("Feedback");
		addAndMakeVisible(m_PhaserFeedbackKnob.get());

		m_VibeRateKnob = std::make_unique<LabeledKnob>("Rate");
		addAndMakeVisible(m_VibeRateKnob.get());
		m_VibeDepthKnob = std::make_unique<LabeledKnob>("Depth");
		addAndMakeVisible(m_VibeDepthKnob.get());
		m_VibeWidthKnob = std::make_unique<LabeledKnob>("Width");
		addAndMakeVisible(m_VibeWidthKnob.get());

		addAndMakeVisible(m_VibeModeButton);
		m_VibeModeButton.setButtonText("Vibrato");

		m_EffectAttachment = std::make_unique<ComboBoxAttachment>(m_APVTS, ParameterIDs::EffectType, m_EffectSelector);
		m_MixAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::Mix, m_MixKnob->getSlider());
		m_WarmthAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::Warmth, m_WarmthKnob->getSlider());
		m_AgeAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::Age, m_AgeKnob->getSlider());

		m_ChorusRateAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::ChorusRate, m_ChorusRateKnob->getSlider());
		m_ChorusDepthAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::ChorusDepth, m_ChorusDepthKnob->getSlider());
		m_ChorusWidthAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::ChorusWidth, m_ChorusWidthKnob->getSlider());
		m_ChorusVoicesAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::ChorusVoices, m_ChorusVoicesKnob->getSlider());

		m_FlangerRateAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::FlangerRate, m_FlangerRateKnob->getSlider());
		m_FlangerDepthAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::FlangerDepth, m_FlangerDepthKnob->getSlider());
		m_FlangerWidthAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::FlangerWidth, m_FlangerWidthKnob->getSlider());
		m_FlangerFeedbackAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::FlangerFeedback, m_FlangerFeedbackKnob->getSlider());
		m_FlangerBaseDelayAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::FlangerBaseDelay, m_FlangerBaseDelayKnob->getSlider());

		m_PhaserRateAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::PhaserRate, m_PhaserRateKnob->getSlider());
		m_PhaserDepthAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::PhaserDepth, m_PhaserDepthKnob->getSlider());
		m_PhaserWidthAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::PhaserWidth, m_PhaserWidthKnob->getSlider());
		m_PhaserStagesAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::PhaserStages, m_PhaserStagesKnob->getSlider());
		m_PhaserFeedbackAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::PhaserFeedback, m_PhaserFeedbackKnob->getSlider());

		m_VibeRateAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::VibeRate, m_VibeRateKnob->getSlider());
		m_VibeDepthAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::VibeDepth, m_VibeDepthKnob->getSlider());
		m_VibeWidthAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::VibeWidth, m_VibeWidthKnob->getSlider());
		m_VibeModeAtt = std::make_unique<ButtonAttachment>(m_APVTS, ParameterIDs::VibeMode, m_VibeModeButton);

		m_LastEffectIndex = static_cast<int>(m_APVTS.getRawParameterValue(ParameterIDs::EffectType)->load());

		startTimerHz(30);
		setSize(560, 440);
	}

	CCSAudioProcessorEditor::~CCSAudioProcessorEditor()
	{
		setLookAndFeel(nullptr);
	}

	void CCSAudioProcessorEditor::paint(juce::Graphics& graphics)
	{
		graphics.fillAll(Palette::Background);

		// Main panel
		auto bounds = getLocalBounds().reduced(kMargin);
		graphics.setColour(Palette::Plate);
		graphics.fillRoundedRectangle(bounds.toFloat(), kCornerRadius);

		// Border
		graphics.setColour(Palette::TitleText.withAlpha(0.06f));
		graphics.drawRoundedRectangle(bounds.toFloat().reduced(1.0f), kCornerRadius, 1.0f);

		// Brand plate
		auto plateBounds = getLocalBounds().reduced(kMargin).removeFromTop(kHeaderHeight);
		graphics.setColour(Palette::KnobBody);
		graphics.fillRoundedRectangle(plateBounds.toFloat(), kCornerRadiusSmall);
		graphics.setColour(Palette::TitleText);
		graphics.setFont(juce::Font(juce::FontOptions(18.0f, juce::Font::bold)));
		graphics.drawText("CozyChorus Suite", plateBounds.reduced(kGap), juce::Justification::centredLeft);

		// Character plate
		auto characterBounds = m_CharacterZone.reduced(kGap);
		graphics.setColour(Palette::Track);
		graphics.drawRoundedRectangle(characterBounds.toFloat(), kCornerRadiusSmall, 1.5f);
		graphics.setColour(Palette::CaptionText);
		graphics.setFont(juce::Font(juce::FontOptions(12.0f)));
		auto captionBounds = characterBounds.removeFromTop(kCaptionHeight);
		graphics.drawText("Character", captionBounds, juce::Justification::centred);
	}

	void CCSAudioProcessorEditor::resized()
	{
		RenderComponents();
	}

	void CCSAudioProcessorEditor::timerCallback()
	{
		const int idx = static_cast<int>(m_APVTS.getRawParameterValue(ParameterIDs::EffectType)->load());
		if (idx != m_LastEffectIndex)
		{
			m_LastEffectIndex = idx;
			RenderComponents();
		}

		const bool isMixDead = m_LastEffectIndex == static_cast<int>(EffectType::Vibe) && m_APVTS.getRawParameterValue(ParameterIDs::VibeMode)->load() >= 0.5f;
		m_MixKnob->getSlider().setEnabled(!isMixDead);
	}

	void CCSAudioProcessorEditor::HideAllEffectComponents()
	{
		m_ChorusDepthKnob->setVisible(false);
		m_ChorusRateKnob->setVisible(false);
		m_ChorusWidthKnob->setVisible(false);
		m_ChorusVoicesKnob->setVisible(false);
		m_FlangerDepthKnob->setVisible(false);
		m_FlangerRateKnob->setVisible(false);
		m_FlangerWidthKnob->setVisible(false);
		m_FlangerFeedbackKnob->setVisible(false);
		m_FlangerBaseDelayKnob->setVisible(false);
		m_PhaserDepthKnob->setVisible(false);
		m_PhaserRateKnob->setVisible(false);
		m_PhaserWidthKnob->setVisible(false);
		m_PhaserStagesKnob->setVisible(false);
		m_PhaserFeedbackKnob->setVisible(false);
		m_VibeDepthKnob->setVisible(false);
		m_VibeRateKnob->setVisible(false);
		m_VibeWidthKnob->setVisible(false);
		m_VibeModeButton.setVisible(false);
	}

	void CCSAudioProcessorEditor::RenderComponents()
	{
		auto area = getLocalBounds().reduced(kMargin);
		auto header = area.removeFromTop(kHeaderHeight);
		area.removeFromTop(kGap);
		auto bottomRow = area.removeFromBottom(kBottomRowHeight);
		area.removeFromBottom(kGap);
		auto mixZone = area.removeFromLeft(kMixZoneWidth);
		m_CharacterZone = area.removeFromRight(kCharacterZoneWidth);
		m_ModulationVisualiser->setBounds(area); // remaining space in the middle

		m_EffectSelector.setBounds(header.reduced(kGap).removeFromRight(kSelectorWidth));
		m_MixKnob->setBounds(mixZone.withSizeKeepingCentre(kKnobWidth, kKnobHeight));
		auto characterKnobs = m_CharacterZone.reduced(10.0f);
		auto warmthCell = characterKnobs.removeFromLeft(characterKnobs.getWidth() / 2.0f);
		auto ageCell = characterKnobs;
		m_WarmthKnob->setBounds(warmthCell.withSizeKeepingCentre(kKnobWidth, kKnobHeight));
		m_AgeKnob->setBounds(ageCell.withSizeKeepingCentre(kKnobWidth, kKnobHeight));

		HideAllEffectComponents();
		auto activeComponents = GetActiveComponents();
		int n = static_cast<int>(activeComponents.size());
		float totalWidth = (n * kKnobWidth) + ((n - 1) * kGap);
		float x = bottomRow.getCentreX() - (totalWidth / 2.0f);
		for (auto* comp : activeComponents)
		{
			comp->setBounds(x, bottomRow.getY(), kKnobWidth, kKnobHeight);
			comp->setVisible(true);
			x += kKnobWidth + kGap;
		}
	}

	std::vector<juce::Component*> CCSAudioProcessorEditor::GetActiveComponents()
	{
		const auto type = static_cast<EffectType>(m_LastEffectIndex);
		switch (type)
		{
		case EffectType::Chorus:
			return { m_ChorusRateKnob.get(), m_ChorusDepthKnob.get(), m_ChorusWidthKnob.get(), m_ChorusVoicesKnob.get() };
		case EffectType::Flanger:
			return { m_FlangerRateKnob.get(), m_FlangerDepthKnob.get(), m_FlangerWidthKnob.get(), m_FlangerFeedbackKnob.get(), m_FlangerBaseDelayKnob.get() };
		case EffectType::Phaser:
			return { m_PhaserRateKnob.get(), m_PhaserDepthKnob.get(), m_PhaserWidthKnob.get(), m_PhaserStagesKnob.get(), m_PhaserFeedbackKnob.get() };
		case EffectType::Vibe:
			return { m_VibeRateKnob.get(), m_VibeDepthKnob.get(), m_VibeWidthKnob.get(), &m_VibeModeButton };
		default:
			return {};
		}
	}
}