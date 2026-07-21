#include "CCSAudioProcessorEditor.h"

namespace CozyChorus
{
	namespace
	{
		// Layout metrics shared by resized() (positions the controls) and paint()
		// (draws the title/caption bands that resized() reserves). Keep them in sync.
		constexpr int kMargin = 12;
		constexpr int kTitleHeight = 26;
		constexpr int kSelectorHeight = 24;
		constexpr int kGap = 10;
		constexpr int kCaptionHeight = 16;
		constexpr int kCellPadX = 6;
		constexpr int kCellPadY = 4;
		constexpr int kMaxColumns = 4;

		// Cozy, warm palette.
		const juce::Colour kBackground{ 0xff2a2320 };
		const juce::Colour kTitleText{ 0xfff2e6d0 };
		const juce::Colour kCaptionText{ 0xffcbb89a };
	}

	CCSAudioProcessorEditor::CCSAudioProcessorEditor(juce::AudioProcessor& processor)
		: juce::AudioProcessorEditor(processor), m_Processor(static_cast<PluginProcessor&>(processor)), m_APVTS(m_Processor.GetAPVTS())
	{
		addAndMakeVisible(m_EffectSelector);
		m_EffectSelector.addItemList(GetEffectTypeChoices(), 1);
		addAndMakeVisible(m_MixSlider);
		m_MixSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
		m_MixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);

		addAndMakeVisible(m_ChorusRateSlider);
		m_ChorusRateSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
		m_ChorusRateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
		addAndMakeVisible(m_ChorusDepthSlider);
		m_ChorusDepthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
		m_ChorusDepthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
		addAndMakeVisible(m_ChorusWidthSlider);
		m_ChorusWidthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
		m_ChorusWidthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
		addAndMakeVisible(m_ChorusVoicesSlider);
		m_ChorusVoicesSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
		m_ChorusVoicesSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);

		addAndMakeVisible(m_FlangerRateSlider);
		m_FlangerRateSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
		m_FlangerRateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
		addAndMakeVisible(m_FlangerDepthSlider);
		m_FlangerDepthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
		m_FlangerDepthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
		addAndMakeVisible(m_FlangerWidthSlider);
		m_FlangerWidthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
		m_FlangerWidthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
		addAndMakeVisible(m_FlangerFeedbackSlider);
		m_FlangerFeedbackSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
		m_FlangerFeedbackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
		addAndMakeVisible(m_FlangerBaseDelaySlider);
		m_FlangerBaseDelaySlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
		m_FlangerBaseDelaySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);

		addAndMakeVisible(m_PhaserRateSlider);
		m_PhaserRateSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
		m_PhaserRateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
		addAndMakeVisible(m_PhaserDepthSlider);
		m_PhaserDepthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
		m_PhaserDepthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
		addAndMakeVisible(m_PhaserWidthSlider);
		m_PhaserWidthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
		m_PhaserWidthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
		addAndMakeVisible(m_PhaserStagesSlider);
		m_PhaserStagesSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
		m_PhaserStagesSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
		addAndMakeVisible(m_PhaserFeedbackSlider);
		m_PhaserFeedbackSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
		m_PhaserFeedbackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);

		addAndMakeVisible(m_VibeRateSlider);
		m_VibeRateSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
		m_VibeRateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
		addAndMakeVisible(m_VibeDepthSlider);
		m_VibeDepthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
		m_VibeDepthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
		addAndMakeVisible(m_VibeWidthSlider);
		m_VibeWidthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
		m_VibeWidthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
		addAndMakeVisible(m_VibeModeButton);
		m_VibeModeButton.setButtonText("Vibrato");

		m_EffectAttachment = std::make_unique<ComboBoxAttachment>(m_APVTS, ParameterIDs::EffectType, m_EffectSelector);
		m_MixAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::Mix, m_MixSlider);

		m_ChorusRateAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::ChorusRate, m_ChorusRateSlider);
		m_ChorusDepthAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::ChorusDepth, m_ChorusDepthSlider);
		m_ChorusWidthAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::ChorusWidth, m_ChorusWidthSlider);
		m_ChorusVoicesAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::ChorusVoices, m_ChorusVoicesSlider);

		m_FlangerRateAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::FlangerRate, m_FlangerRateSlider);
		m_FlangerDepthAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::FlangerDepth, m_FlangerDepthSlider);
		m_FlangerWidthAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::FlangerWidth, m_FlangerWidthSlider);
		m_FlangerFeedbackAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::FlangerFeedback, m_FlangerFeedbackSlider);
		m_FlangerBaseDelayAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::FlangerBaseDelay, m_FlangerBaseDelaySlider);

		m_PhaserRateAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::PhaserRate, m_PhaserRateSlider);
		m_PhaserDepthAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::PhaserDepth, m_PhaserDepthSlider);
		m_PhaserWidthAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::PhaserWidth, m_PhaserWidthSlider);
		m_PhaserStagesAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::PhaserStages, m_PhaserStagesSlider);
		m_PhaserFeedbackAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::PhaserFeedback, m_PhaserFeedbackSlider);

		m_VibeRateAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::VibeRate, m_VibeRateSlider);
		m_VibeDepthAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::VibeDepth, m_VibeDepthSlider);
		m_VibeWidthAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::VibeWidth, m_VibeWidthSlider);
		m_VibeModeAtt = std::make_unique<ButtonAttachment>(m_APVTS, ParameterIDs::VibeMode, m_VibeModeButton);

		startTimerHz(30);
		UpdateVisibility();
		setSize(400, 300);
	}

	CCSAudioProcessorEditor::~CCSAudioProcessorEditor()
	{
	}

	void CCSAudioProcessorEditor::paint(juce::Graphics& graphics)
	{
		graphics.fillAll(kBackground);

		auto bounds = getLocalBounds().reduced(kMargin);

		// Title band (resized() reserves the same strip at the top).
		graphics.setColour(kTitleText);
		graphics.setFont(juce::Font(juce::FontOptions(18.0f, juce::Font::bold)));
		graphics.drawText("CozyChorus Suite", bounds.removeFromTop(kTitleHeight), juce::Justification::centredLeft);

		// A caption above each visible slider, drawn into the band resized() left
		// empty above the control. Reading each slider's laid-out bounds keeps the
		// captions aligned with whatever grid resized() produced for this effect.
		graphics.setFont(juce::Font(juce::FontOptions(13.0f)));
		graphics.setColour(kCaptionText);

		const auto captionFor = [this](const juce::Component* comp) -> juce::String
		{
			if (comp == &m_MixSlider) return "Mix";

			if (comp == &m_ChorusRateSlider) return "Rate";
			if (comp == &m_ChorusDepthSlider) return "Depth";
			if (comp == &m_ChorusWidthSlider) return "Width";
			if (comp == &m_ChorusVoicesSlider) return "Voices";

			if (comp == &m_FlangerRateSlider) return "Rate";
			if (comp == &m_FlangerDepthSlider) return "Depth";
			if (comp == &m_FlangerWidthSlider) return "Width";
			if (comp == &m_FlangerFeedbackSlider)  return "Feedback";
			if (comp == &m_FlangerBaseDelaySlider) return "Base Delay";

			if (comp == &m_PhaserRateSlider) return "Rate";
			if (comp == &m_PhaserDepthSlider) return "Depth";
			if (comp == &m_PhaserWidthSlider) return "Width";
			if (comp == &m_PhaserStagesSlider) return "Stages";
			if (comp == &m_PhaserFeedbackSlider) return "Feedback";

			if (comp == &m_VibeRateSlider) return "Rate";
			if (comp == &m_VibeDepthSlider) return "Depth";
			if (comp == &m_VibeWidthSlider) return "Width";
			if (comp == &m_VibeModeButton) return "Vibrato";
			return {};
		};

		for (auto* comp : GetAllComponents())
		{
			if (!comp->isVisible())
				continue;

			const auto compBounds = comp->getBounds();
			const juce::Rectangle<int> caption(compBounds.getX(), compBounds.getY() - kCaptionHeight, compBounds.getWidth(), kCaptionHeight);
			graphics.drawText(captionFor(comp), caption, juce::Justification::centred);
		}
	}

	void CCSAudioProcessorEditor::resized()
	{
		RenderComponents();
	}

	void CCSAudioProcessorEditor::timerCallback()
	{
		const int idx = (int)m_APVTS.getRawParameterValue(ParameterIDs::EffectType)->load();
		if (idx != m_LastEffectIndex)
		{
			m_LastEffectIndex = idx;
			UpdateVisibility();
		}
	}

	void CCSAudioProcessorEditor::UpdateVisibility()
	{
		const auto type = static_cast<EffectType>(m_LastEffectIndex);

		m_MixSlider.setVisible(true);

		m_ChorusRateSlider.setVisible(type == EffectType::Chorus);
		m_ChorusDepthSlider.setVisible(type == EffectType::Chorus);
		m_ChorusWidthSlider.setVisible(type == EffectType::Chorus);
		m_ChorusVoicesSlider.setVisible(type == EffectType::Chorus);

		m_FlangerRateSlider.setVisible(type == EffectType::Flanger);
		m_FlangerDepthSlider.setVisible(type == EffectType::Flanger);
		m_FlangerWidthSlider.setVisible(type == EffectType::Flanger);
		m_FlangerFeedbackSlider.setVisible(type == EffectType::Flanger);
		m_FlangerBaseDelaySlider.setVisible(type == EffectType::Flanger);

		m_PhaserRateSlider.setVisible(type == EffectType::Phaser);
		m_PhaserDepthSlider.setVisible(type == EffectType::Phaser);
		m_PhaserWidthSlider.setVisible(type == EffectType::Phaser);
		m_PhaserStagesSlider.setVisible(type == EffectType::Phaser);
		m_PhaserFeedbackSlider.setVisible(type == EffectType::Phaser);

		m_VibeRateSlider.setVisible(type == EffectType::Vibe);
		m_VibeDepthSlider.setVisible(type == EffectType::Vibe);
		m_VibeWidthSlider.setVisible(type == EffectType::Vibe);
		m_VibeModeButton.setVisible(type == EffectType::Vibe);

		RenderComponents();
	}

	void CCSAudioProcessorEditor::RenderComponents()
	{
		auto area = getLocalBounds().reduced(kMargin);

		area.removeFromTop(kTitleHeight); // title band, painted in paint()

		m_EffectSelector.setBounds(area.removeFromTop(kSelectorHeight));
		area.removeFromTop(kGap);

		// Only the controls visible for the active effect take part in the grid, in a
		// fixed left-to-right order, wrapping after kMaxColumns.
		juce::Array<juce::Component*> visible;
		for (auto* slider : GetAllComponents())
		{
			if (slider->isVisible())
				visible.add(slider);
		}

		if (visible.isEmpty())
			return;

		const int columns = juce::jmin(kMaxColumns, visible.size());
		const int rows = (visible.size() + columns - 1) / columns;

		const int cellWidth = area.getWidth() / columns;
		const int cellHeight = area.getHeight() / rows;

		for (int i = 0; i < visible.size(); ++i)
		{
			const int column = i % columns;
			const int row = i / columns;

			juce::Rectangle<int> cell(area.getX() + column * cellWidth, area.getY() + row * cellHeight, cellWidth, cellHeight);

			cell.reduce(kCellPadX, kCellPadY);
			cell.removeFromTop(kCaptionHeight); // caption band, painted in paint()

			visible[i]->setBounds(cell);
		}
		repaint();
	}

	std::vector<juce::Component*> CCSAudioProcessorEditor::GetAllComponents()
	{
		return { &m_MixSlider, &m_ChorusRateSlider, &m_ChorusDepthSlider, &m_ChorusWidthSlider, &m_ChorusVoicesSlider, &m_FlangerRateSlider, &m_FlangerDepthSlider, &m_FlangerWidthSlider, &m_FlangerFeedbackSlider, &m_FlangerBaseDelaySlider, &m_PhaserRateSlider, &m_PhaserDepthSlider, &m_PhaserWidthSlider, &m_PhaserStagesSlider, &m_PhaserFeedbackSlider, &m_VibeRateSlider, &m_VibeDepthSlider, &m_VibeWidthSlider, &m_VibeModeButton };
	}
}