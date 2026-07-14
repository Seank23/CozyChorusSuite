#pragma once

#include <juce_dsp/juce_dsp.h>

namespace CozyChorus
{
	// Abstract base for every modulation effect in the suite.
	//
	// Real-time contract:
	//   - Prepare() may allocate; it is called from prepareToPlay, off the audio thread.
	//   - Process() and Reset() run on the audio thread: no allocation, no locks.
	class ModulationEffect
	{
	public:
		virtual ~ModulationEffect() = default;

		// Allocate buffers / delay lines here from the given spec
		// (sample rate, max block size, channel count).
		virtual void Prepare(const juce::dsp::ProcessSpec& spec) = 0;

		// Real-time-safe processing over a single in-place audio block.
		virtual void Process(const juce::dsp::ProcessContextReplacing<float>& context) = 0;

		// Clear internal state without reallocating.
		virtual void Reset() = 0;
	};
}