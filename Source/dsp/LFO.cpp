#include "LFO.h"
#include <cmath>
#include <juce_core/juce_core.h>

namespace CozyChorus
{
	void LFO::Prepare(double sampleRate)
	{
		m_SampleRate = sampleRate;
		UpdatePhaseIncrement();
	}

	void LFO::Advance()
	{
		m_Phase += m_PhaseIncrement;
		if (m_Phase >= 1.0f)
			m_Phase -= 1.0f;
	}

	void LFO::Reset()
	{
		m_Phase = 0.0f;
	}

	void LFO::SetFrequency(float hz)
	{
		m_Frequency = hz;
		UpdatePhaseIncrement();
	}

	float LFO::GetValue(float phaseOffset) const
	{
		float finalPhase = m_Phase + phaseOffset;
		switch (m_Shape)
		{
		case Shape::Sine:
			return std::sin(juce::MathConstants<float>::twoPi * finalPhase);
		case Shape::Triangle:
			return 2.0f * std::abs(2.0f * (finalPhase - std::floor(finalPhase + 0.5f))) - 1.0f;
		case Shape::Sawtooth:
			return 2.0f * (finalPhase - std::floor(finalPhase)) - 1.0f;
		case Shape::Square:
			return (std::fmod(finalPhase, 1.0f) < 0.5f) ? 1.0f : -1.0f;
		default:
			return 0.0f;
		}
	}

	void LFO::UpdatePhaseIncrement()
	{
		m_PhaseIncrement = m_Frequency / static_cast<float>(m_SampleRate);
	}
}