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

		addAndMakeVisible(m_RateSlider);
		m_RateSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
		m_RateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);

		addAndMakeVisible(m_MixSlider);
		m_MixSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
		m_MixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);

		addAndMakeVisible(m_DepthSlider);
		m_DepthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
		m_DepthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);

		addAndMakeVisible(m_WidthSlider);
		m_WidthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
		m_WidthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);

		addAndMakeVisible(m_VoicesSlider);
		m_VoicesSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
		m_VoicesSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);

		addAndMakeVisible(m_FlangerFeedbackSlider);
		m_FlangerFeedbackSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
		m_FlangerFeedbackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);

		addAndMakeVisible(m_BaseDelaySlider);
		m_BaseDelaySlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
		m_BaseDelaySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);

		addAndMakeVisible(m_StagesSlider);
		m_StagesSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
		m_StagesSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);

		addAndMakeVisible(m_PhaserFeedbackSlider);
		m_PhaserFeedbackSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
		m_PhaserFeedbackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);

		addAndMakeVisible(m_VibeModeButton);
		m_VibeModeButton.setButtonText("Vibrato");

		m_EffectAttachment = std::make_unique<ComboBoxAttachment>(m_APVTS, ParameterIDs::EffectType, m_EffectSelector);
		m_RateAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::Rate, m_RateSlider);
		m_MixAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::Mix, m_MixSlider);
		m_DepthAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::Depth, m_DepthSlider);
		m_WidthAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::Width, m_WidthSlider);
		m_VoicesAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::ChorusVoices, m_VoicesSlider);
		m_FlangerFeedbackAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::FlangerFeedback, m_FlangerFeedbackSlider);
		m_BaseDelayAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::FlangerBaseDelay, m_BaseDelaySlider);
		m_StagesAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::PhaserStages, m_StagesSlider);
		m_PhaserFeedbackAtt = std::make_unique<SliderAttachment>(m_APVTS, ParameterIDs::PhaserFeedback, m_PhaserFeedbackSlider);
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
			if (comp == &m_RateSlider) return "Rate";
			if (comp == &m_DepthSlider) return "Depth";
			if (comp == &m_MixSlider) return "Mix";
			if (comp == &m_WidthSlider) return "Width";
			if (comp == &m_VoicesSlider) return "Voices";
			if (comp == &m_FlangerFeedbackSlider)  return "Feedback";
			if (comp == &m_BaseDelaySlider) return "Base Delay";
			if (comp == &m_StagesSlider) return "Stages";
			if (comp == &m_PhaserFeedbackSlider) return "Feedback";
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
		m_VoicesSlider.setVisible(type == EffectType::Chorus);
		m_FlangerFeedbackSlider.setVisible(type == EffectType::Flanger);
		m_BaseDelaySlider.setVisible(type == EffectType::Flanger);
		m_StagesSlider.setVisible(type == EffectType::Phaser);
		m_PhaserFeedbackSlider.setVisible(type == EffectType::Phaser);
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
		return { &m_RateSlider, &m_DepthSlider, &m_MixSlider, &m_WidthSlider, &m_VoicesSlider, &m_FlangerFeedbackSlider, &m_BaseDelaySlider, &m_StagesSlider, &m_PhaserFeedbackSlider, &m_VibeModeButton };
	}
}