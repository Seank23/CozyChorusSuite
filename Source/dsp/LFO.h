#pragma once

namespace CozyChorus
{
	class LFO
	{
	public:
		enum class Shape
		{
			Sine,
			Triangle,
			Sawtooth,
			Square
		};

		void Prepare(double sampleRate);
		void Advance();
		void Reset();

		void SetFrequency(float hz);
		void SetShape(Shape shape) { m_Shape = shape; }

		float GetValue(float phaseOffset = 0.0f) const;

	private:
		void UpdatePhaseIncrement();

		double m_SampleRate = 44100.0;
		float m_Frequency = 1.0f;
		float m_Phase = 0.0f;
		float m_PhaseIncrement = 0.0f;
		Shape m_Shape = Shape::Sine;
	};
}