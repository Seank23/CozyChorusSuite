#include "LabeledKnob.h"
#include "EditorConstants.h"

namespace CozyChorus
{
	LabeledKnob::LabeledKnob(const std::string& caption)
	{
		m_Label.setText(caption, juce::dontSendNotification);
		m_Label.setJustificationType(juce::Justification::centred);
		addAndMakeVisible(m_Label);

		m_Slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
		m_Slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, kKnobWidth, kValueBoxHeight);
		addAndMakeVisible(m_Slider);
	}

	void LabeledKnob::resized()
	{
		auto bounds = getLocalBounds();
		m_Label.setBounds(bounds.removeFromTop(kCaptionHeight));
		m_Slider.setBounds(bounds);
	}
}