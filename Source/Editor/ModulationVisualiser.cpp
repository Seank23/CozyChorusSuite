#include "ModulationVisualiser.h"
#include "EditorConstants.h"
#include "CCSLookAndFeel.h"
#include "../dsp/PhaserEffect.h"
#include "../dsp/VibeEffect.h"

namespace CozyChorus
{
	ModulationVisualiser::ModulationVisualiser(PluginProcessor& processor)
		: m_Processor(processor), m_APVTS(processor.GetAPVTS())
	{
		m_VBlankAttachment = juce::VBlankAttachment(this, [this] { UpdateVisualisation(); });
		setMouseCursor(juce::MouseCursor::PointingHandCursor);
		m_SampleRate = m_Processor.GetProcessSpec().sampleRate;
	}

	ModulationVisualiser::~ModulationVisualiser()
	{
	}

	void ModulationVisualiser::paint(juce::Graphics& g)
	{
		auto type = m_Processor.GetActiveEffectType();
		auto area = getLocalBounds();
		g.setColour(Palette::Screen);
		g.fillRoundedRectangle(area.toFloat(), kCornerRadiusSmall);
		g.setColour(juce::Colours::black.withAlpha(0.25f));
		g.drawRoundedRectangle(area.toFloat().reduced(1.0f), kCornerRadiusSmall, 1.0f);
		g.setColour(Palette::CaptionText.withAlpha(0.6f));
		std::string title = m_Mode == Mode::LFO ? "LFO" : IsDelayEffect(type) ? "Signal" : "Spectrum";
		g.drawText(title, area.reduced(kGap).removeFromTop(kCaptionHeight), juce::Justification::centredLeft);

		if (m_Mode == Mode::LFO)
			PaintLFOResponse(g, area);
		else if (m_Mode == Mode::Response)
		{
			if (IsDelayEffect(type))
				PaintDelayResponse(g, area, type);
			else
				PaintSpectrumResponse(g, area, type);
		}
	}

	void ModulationVisualiser::mouseDown(const juce::MouseEvent& event)
	{
		if (event.mods.isLeftButtonDown())
		{
			m_Phase = 0.0;
			SetMode(m_Mode == Mode::LFO ? Mode::Response : Mode::LFO);
		}
	}

	void ModulationVisualiser::SetMode(Mode mode)
	{
		m_Mode = mode;
		repaint();
	}

	void ModulationVisualiser::UpdateVisualisation()
	{
		if (m_Processor.GetProcessSpec().sampleRate <= 0.0f)
			return;

		m_SampleRate = m_Processor.GetProcessSpec().sampleRate;
		m_Phase = m_Processor.GetVisualPhase();
		m_DelayMs = m_Processor.GetVisualDelayInSamples() / m_SampleRate * 1000.0f;
		repaint();
	}

	void ModulationVisualiser::PaintDelayResponse(juce::Graphics& g, juce::Rectangle<int> area, EffectType type)
	{
		float midY = area.getCentreY();
		float depth = SampleDepth(type) * 0.5f;
		float amplitude = depth * (area.getHeight() / 2.0f - kGap);
		int width = area.getWidth();
		float displayFreq = kVisCyclesShown / static_cast<float>(width);
		float shiftPx = m_DelayMs * kVisPxPerMs;

		juce::Path dry, wet, sum;
		int startX = area.getX();
		for (int x = 0; x < width; x++)
		{
			float yDry = midY - amplitude * std::sin(juce::MathConstants<float>::twoPi * (displayFreq * x));
			float yWet = midY - amplitude * std::sin(juce::MathConstants<float>::twoPi * (displayFreq * (x - shiftPx)));
			(x == 0) ? dry.startNewSubPath(startX, yDry) : dry.lineTo(startX + static_cast<float>(x), yDry);
			(x == 0) ? wet.startNewSubPath(startX, yWet) : wet.lineTo(startX + static_cast<float>(x), yWet);
		}
		g.setColour(Palette::Trace);
		g.strokePath(dry, juce::PathStrokeType(2.0f));
		g.setColour(Palette::Accent);
		g.strokePath(wet, juce::PathStrokeType(2.0f));
	}

	void ModulationVisualiser::PaintSpectrumResponse(juce::Graphics& g, juce::Rectangle<int> area, EffectType type)
	{
		EvaluateCutoffFrequency(type);

		constexpr int kAxisLabelHeight = 12;
		constexpr float kAxisFontHeight = 9.0f;
		constexpr int kAxisInset = 2;
		constexpr int kPaddingTop = 35;

		auto plotArea = area;

		g.setFont(juce::Font(juce::FontOptions(kAxisFontHeight)));

		// Magnitude (dB) gridlines and labels, drawn inside the plot area.
		const float dbTicks[] = { 0.0f, MIN_MAG_DB };
		for (float db : dbTicks)
		{
			float y = juce::jmap(db, MIN_MAG_DB, MAX_MAG_DB, static_cast<float>(plotArea.getBottom()), static_cast<float>(plotArea.getY()));
			g.setColour(db == 0.0f ? Palette::Track.brighter(0.2f) : Palette::Track);
			g.drawLine(static_cast<float>(plotArea.getX()), y, static_cast<float>(plotArea.getRight()), y, 1.0f);
			g.setColour(Palette::CaptionText.withAlpha(0.7f));
			juce::String label = juce::String(static_cast<int>(db)) + " dB";
			bool labelBelowLine = db == MAX_MAG_DB;
			int labelY = labelBelowLine ? static_cast<int>(y) + kAxisInset : static_cast<int>(y) - kAxisLabelHeight - kAxisInset;
			g.drawText(label, plotArea.getX() + kAxisInset, labelY, 40, kAxisLabelHeight, juce::Justification::centredLeft);
		}

		// Frequency gridlines and labels (log scale, 20 Hz - 20 kHz), drawn inside the plot area.
		const float freqTicks[] = { 100.0f, 1000.0f, 10000.0f };
		for (float freq : freqTicks)
		{
			float u = std::log(freq / MIN_FREQUENCY) / std::log(MAX_FREQUENCY / MIN_FREQUENCY);
			float x = plotArea.getX() + u * plotArea.getWidth();
			g.setColour(Palette::Track);
			g.drawLine(x, static_cast<float>(plotArea.getY() + kPaddingTop), x, static_cast<float>(plotArea.getBottom()), 1.0f);
			g.setColour(Palette::CaptionText.withAlpha(0.7f));
			juce::String label = freq >= 1000.0f ? juce::String(freq / 1000.0f, 0) + "k" : juce::String(static_cast<int>(freq));
			int labelWidth = 28;
			bool isLastTick = freq == freqTicks[juce::numElementsInArray(freqTicks) - 1];
			int labelX = isLastTick ? static_cast<int>(x) - labelWidth - kAxisInset : static_cast<int>(x) + kAxisInset;
			g.drawText(label, labelX, plotArea.getBottom() - kAxisLabelHeight - kAxisInset, labelWidth, kAxisLabelHeight,
				isLastTick ? juce::Justification::centredRight : juce::Justification::centredLeft);
		}

		juce::Path path;
		int width = plotArea.getWidth();
		int startX = plotArea.getX();
		for (int x = 0; x < width; x++)
		{
			float u = x / (float)width;
			float frequency = MIN_FREQUENCY * std::pow(MAX_FREQUENCY / MIN_FREQUENCY, u);
			float response = EvaluateTransferFunction(frequency, type);
			float y = juce::jmap(response, MIN_MAG_DB, MAX_MAG_DB, static_cast<float>(plotArea.getBottom()), static_cast<float>(plotArea.getY()));
			(x == 0) ? path.startNewSubPath(startX, y) : path.lineTo(startX + static_cast<float>(x), y);
		}
		g.setColour(Palette::Accent);
		g.strokePath(path, juce::PathStrokeType(2.0f));
	}

	void ModulationVisualiser::PaintLFOResponse(juce::Graphics& g, juce::Rectangle<int> area)
	{
		float midY = area.getCentreY();
		EffectType type = m_Processor.GetActiveEffectType();
		float depth = SampleDepth(type) * 0.5f;
		float amplitude = depth * (area.getHeight() / 2.0f - kGap);

		juce::Path path;
		int width = area.getWidth();
		int startX = area.getX();
		for (int x = 0; x < width; x++)
		{
			float u = x / (float)width;
			float phase = m_Phase + u * kVisCyclesShown;
			float y = midY - amplitude * SampleShape(phase, type);
			(x == 0) ? path.startNewSubPath(startX, y) : path.lineTo(startX + static_cast<float>(x), y);
		}
		g.setColour(Palette::Track);
		g.drawLine(startX, midY, area.getRight(), midY, 1.0f);
		g.setColour(Palette::Accent);
		g.strokePath(path, juce::PathStrokeType(2.0f));
	}

	float ModulationVisualiser::SampleShape(float phase, EffectType type)
	{
		switch (type)
		{
		case EffectType::Chorus:
		case EffectType::Flanger:
		case EffectType::Phaser:
			return std::sin(juce::MathConstants<float>::twoPi * phase);
		case EffectType::Vibe:
			return VibeEffect::GetAsymmetricShape(phase);
		default:
			return 0.0f;
		}
	}

	float ModulationVisualiser::SampleDepth(EffectType type)
	{
		float depth = 0.0f;
		switch (type)
		{
		case EffectType::Chorus:
			depth = m_APVTS.getRawParameterValue(ParameterIDs::ChorusDepth)->load();
			break;
		case EffectType::Flanger:
			depth = m_APVTS.getRawParameterValue(ParameterIDs::FlangerDepth)->load();
			break;
		case EffectType::Phaser:
			depth = m_APVTS.getRawParameterValue(ParameterIDs::PhaserDepth)->load();
			break;
		case EffectType::Vibe:
			depth = m_APVTS.getRawParameterValue(ParameterIDs::VibeDepth)->load();
			break;
		}
		return depth / 100.0f;
	}

	void ModulationVisualiser::EvaluateCutoffFrequency(EffectType type)
	{
		float logCutoff = 0.5f * (std::log(PhaserEffect::MIN_FC_HZ) + std::log(PhaserEffect::MAX_FC_HZ));
		float logHalf = 0.5f * (std::log(PhaserEffect::MAX_FC_HZ) - std::log(PhaserEffect::MIN_FC_HZ));
		switch (type)
		{
		case EffectType::Phaser:
		{
			m_Cutoffs.resize(1);
			m_Cutoffs[0] = std::clamp(std::exp(logCutoff + (logHalf * SampleDepth(type) * std::sin(juce::MathConstants<float>::twoPi * m_Phase))), PhaserEffect::MIN_FC_HZ, PhaserEffect::MAX_FC_HZ);
			break;
		}
		case EffectType::Vibe:
		{
			m_Cutoffs.resize(VibeEffect::NUM_STAGES);
			float shapeSample = SampleShape(m_Phase, type);
			float center = logCutoff + (logHalf * SampleDepth(type) * shapeSample);
			for (int i = 0; i < VibeEffect::NUM_STAGES; i++)
				m_Cutoffs[i] = std::clamp(std::exp(center + VibeEffect::STAGE_OFFSET[i] * std::log(2.0f)), VibeEffect::MIN_FC_HZ, VibeEffect::MAX_FC_HZ);
			break;
		}
		}
	}

	float ModulationVisualiser::EvaluateTransferFunction(float frequency, EffectType type)
	{
		std::complex<float> aPrime;
		switch (type)
		{
		case EffectType::Phaser:
		{
			float stages = m_APVTS.getRawParameterValue(ParameterIDs::PhaserStages)->load();
			float feedback = m_APVTS.getRawParameterValue(ParameterIDs::PhaserFeedback)->load() / 100.0f;
			float phi = stages * (-2.0f * std::atan(frequency / m_Cutoffs[0]));
			std::complex<float> a = { std::cos(phi), std::sin(phi) };
			aPrime = a / (1.0f - feedback * a);
			break;
		}
		case EffectType::Vibe:
		{
			float phiTotal = 0.0f;
			for (int i = 0; i < VibeEffect::NUM_STAGES; i++)
				phiTotal += -2.0f * std::atan(frequency / m_Cutoffs[i]);
			aPrime = std::complex<float>(std::cos(phiTotal), std::sin(phiTotal));
			break;
		}
		}
		float mix = m_APVTS.getRawParameterValue(ParameterIDs::Mix)->load() / 100.0f;
		std::complex<float> H = mix * aPrime + (1.0f - mix);
		return 20.0f * std::log10(std::max(std::abs(H), 1e-4f));
	}
}