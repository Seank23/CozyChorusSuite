#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace
{
	// Layout metrics shared by resized() (positions the controls) and paint()
	// (draws the title/caption bands that resized() reserves). Keep them in sync.
	constexpr int kMargin = 12;
	constexpr int kSelectorWidth = 140;
	constexpr int kGap = 10;
	constexpr int kCaptionHeight = 16;
	constexpr int kValueBoxHeight = 18;
	constexpr float kCaptionFontHeight = 13.0f;
	constexpr int kCellPadX = 6;
	constexpr int kCellPadY = 4;
	constexpr int kMaxColumns = 4;
	constexpr int kHeaderHeight = 50;
	constexpr int kBottomRowHeight = 120;
	constexpr int kMixZoneWidth = 80;
	constexpr int kCharacterZoneWidth = 160;
	constexpr int kKnobWidth = 60;
	constexpr int kKnobHeight = 100;
	constexpr float kCornerRadius = 4.0f;
	constexpr float kCornerRadiusSmall = 2.0f;

	// Cozy, warm palette.
	const juce::Colour kBackground{ 0xff2a2320 };
	const juce::Colour kTitleText{ 0xfff2e6d0 };
	const juce::Colour kCaptionText{ 0xffcbb89a };
}