#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace CozyChorus
{
	class LabeledKnob : public juce::Component
	{
	public:
		explicit LabeledKnob(const std::string& caption);

		void resized() override;

		juce::Slider& getSlider() { return m_Slider; }

	private:
		juce::Label m_Label;
		juce::Slider m_Slider;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LabeledKnob)
	};
}