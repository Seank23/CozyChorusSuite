#include "CCSLookAndFeel.h"
#include "EditorConstants.h"

namespace CozyChorus
{
	CCSLookAndFeel::CCSLookAndFeel()
	{
		setColour(juce::Slider::textBoxTextColourId, Palette::CaptionText);
		setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);

		setColour(juce::ComboBox::backgroundColourId, Palette::KnobBody);
		setColour(juce::ComboBox::textColourId, Palette::TitleText);
		setColour(juce::ComboBox::outlineColourId, Palette::Track);

		setColour(juce::PopupMenu::backgroundColourId, Palette::Plate);
		setColour(juce::PopupMenu::highlightedBackgroundColourId, Palette::Accent.withAlpha(0.35f));
		setColour(juce::PopupMenu::textColourId, Palette::TitleText);

		setColour(juce::Label::textColourId, Palette::CaptionText);
	}

	void CCSLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider&)
	{
		const float radius = juce::jmin(width / 2.0f, height / 2.0f) * 0.85f;
		const float centreX = x + width * 0.5f;
		const float centreY = y + height * 0.5f;
		const float rx = centreX - radius;
		const float ry = centreY - radius;
		const float rw = radius * 2.0f;
		const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

		float thinkness = radius * 0.18f;

		juce::Path trackArc;
		trackArc.addCentredArc(centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
		g.setColour(Palette::Track);
		g.strokePath(trackArc, juce::PathStrokeType(thinkness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

		juce::Path valueArc;
		valueArc.addCentredArc(centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, angle, true);
		g.setColour(Palette::Accent);
		g.strokePath(valueArc, juce::PathStrokeType(thinkness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

		g.setColour(Palette::KnobBody);
		g.fillEllipse(juce::Rectangle<float>(rx, ry, rw, rw));

		juce::Point<float> pointerStart = juce::Point<float>(centreX, centreY) - juce::Point(std::cos(angle + juce::MathConstants<float>::halfPi), std::sin(angle + juce::MathConstants<float>::halfPi)) * (radius - 10);
		juce::Point<float> pointerEnd = juce::Point<float>(centreX, centreY) - juce::Point(std::cos(angle + juce::MathConstants<float>::halfPi), std::sin(angle + juce::MathConstants<float>::halfPi)) * radius;
		g.setColour(Palette::Accent);
		g.drawLine(juce::Line<float>(pointerStart, pointerEnd), thinkness * 0.5f);
	}

	void CCSLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown, int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox&)
	{
		juce::Rectangle<float> bounds = juce::Rectangle<float>(0.0f, 0.0f, (float)width, (float)height);
		g.setColour(Palette::KnobBody);
		g.fillRoundedRectangle(bounds, kCornerRadius);
		g.setColour(Palette::Track);
		g.drawRoundedRectangle(bounds, kCornerRadius, 1.0f);
	}

	void CCSLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height)
	{
		juce::Rectangle<float> bounds = juce::Rectangle<float>(0.0f, 0.0f, (float)width, (float)height);
		g.setColour(Palette::Plate);
		g.fillRoundedRectangle(bounds, kCornerRadius);
		g.setColour(Palette::Track);
		g.drawRoundedRectangle(bounds, kCornerRadius, 1.0f);
	}

	void CCSLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& btn, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
	{
		auto bounds = btn.getLocalBounds();
		auto captionBounds = bounds.removeFromTop(kCaptionHeight);
		bounds.removeFromBottom(kValueBoxHeight);

		const bool isLit = btn.getToggleState();
		const float ledRadius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.22f;
		const auto led = juce::Rectangle<float>(ledRadius * 2.0f, ledRadius * 2.0f).withCentre(bounds.getCentre().toFloat());

		if (isLit)
		{
			g.setColour(Palette::Accent.withAlpha(0.25f));
			g.fillEllipse(led.expanded(ledRadius * 0.5f));
		}

		auto ledColour = isLit ? Palette::Accent : Palette::Track;
		if (shouldDrawButtonAsDown)
			ledColour = ledColour.darker(0.2f);
		else if (shouldDrawButtonAsHighlighted)
			ledColour = ledColour.brighter(0.15f);

		g.setColour(ledColour);
		g.fillEllipse(led);
		g.setColour(Palette::Track);
		g.drawEllipse(led, 1.0f);

		g.setColour(Palette::CaptionText);
		g.setFont(juce::Font(juce::FontOptions(kCaptionFontHeight)));
		g.drawText(btn.getButtonText(), captionBounds, juce::Justification::centred, false);
	}
}