#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace CozyChorus
{
	namespace Palette
	{
		const juce::Colour Background{ 0xff2a2320 };  // deep warm brown (existing)
		const juce::Colour Plate{ 0xff342b26 };		  // chassis face, a touch lighter than Background
		const juce::Colour Screen{ 0xff1c1815 };	  // recessed viz "screen"
		const juce::Colour TitleText{ 0xfff2e6d0 };	  // cream (existing)
		const juce::Colour CaptionText{ 0xffcbb89a }; // tan (existing)
		const juce::Colour KnobBody{ 0xff3d322c };	  // flat knob cap
		const juce::Colour Track{ 0xff4c3f38 };		  // unfilled arc
		const juce::Colour Accent{ 0xffe0975a };	  // warm amber: value arc, pointer, lit LED  (fork §)
		const juce::Colour Screw{ 0xff5a4c42 };
	}

	class CCSLookAndFeel : public juce::LookAndFeel_V4
	{
	public:
		CCSLookAndFeel();

		void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height, float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider&) override;

		void drawComboBox(juce::Graphics&, int width, int height, bool isButtonDown, int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox&) override;

		void drawPopupMenuBackground(juce::Graphics&, int width, int height) override;

		void drawToggleButton(juce::Graphics&, juce::ToggleButton&, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
	};
}