#pragma once

#include "ModulationEffect.h"

namespace CozyChorus
{
	// Pass-through: leaves the in-place block untouched. Acts as the default until each
	// real effect is implemented, so processBlock always has something to dispatch to.
	class NullEffect : public ModulationEffect
	{
	public:
		void Prepare(const juce::dsp::ProcessSpec&) override {}
		void Process(const juce::dsp::ProcessContextReplacing<float>&) override {}
		void Reset() override {}
	};
}