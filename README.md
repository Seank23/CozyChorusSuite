# CozyChorus Suite

**Four classic guitar-pedal modulation effects — Chorus, Flanger, Phaser and Vibe — behind one
switch, with a global tape "Character" stage on the output and a live modulation visualiser.**

Built with C++20 / JUCE 8 / CMake. Ships as **VST3**, **Standalone**, and **AU** (macOS only).
Guitar-oriented, works in mono and stereo, fully automatable, real-time safe.

<p align="center">
  <img width="500" height="418" alt="image" src="https://github.com/user-attachments/assets/809aa198-d1ac-45a7-806e-cd6b61aa5291" />
</p>

---

## Contents

- [What it is](#what-it-is)
- [Feature highlights](#feature-highlights)
- [Signal flow](#signal-flow)
- [Installing](#installing)
- [Building from source](#building-from-source)
- [The interface](#the-interface)
- [The modulation visualiser](#the-modulation-visualiser)
- [Parameter reference](#parameter-reference)
- [Getting a sound — starting points](#getting-a-sound--starting-points)
- [Host integration notes](#host-integration-notes)
- [Known limitations](#known-limitations)
- [Project layout](#project-layout)
- [Documentation](#documentation)
- [Licensing](#licensing)

---

## What it is

CozyChorus Suite is a single modulation plugin that contains four distinct effects rather than four
plugins. You pick the active effect from the header selector; the faceplate re-lays itself to show
only that effect's controls, so the panel never shows a knob that does nothing.

The four effects are built from **two shared DSP families**:

| Family | Effects | Core |
|---|---|---|
| **Delay line** | Chorus, Flanger | A modulated fractional delay line (Lagrange 3rd-order interpolation). The Flanger is the Chorus plus feedback and a much shorter base delay. |
| **All-pass** | Phaser, Vibe | A cascade of hand-rolled one-pole TPT all-pass filters swept in the log-frequency domain. The Vibe is the Phaser with fixed, staggered stages and an asymmetric LFO. |

All four share one **LFO** design (continuous phase across blocks, per-channel phase offset for
stereo width) and one control vocabulary — **Rate / Depth / Stereo Width** — plus the shared **Mix**.

Whatever effect is active, the output then always passes through the global **Character** stage:
tape-style pitch instability (**Age**) followed by saturation and a high-cut (**Warmth**).

---

## Feature highlights

- **Chorus** — 1–3-voice ensemble. One LFO, per-voice phase offsets and a ±4 ms base-delay spread
  around a 20 ms centre, taps summed and normalised. Bipolar sine sweep, up to ±7 ms at full Depth.
- **Flanger** — feedback comb (read-before-write, so the minimum effective delay is 1 sample).
  Base delay 0.2–5 ms with an upward-only sweep of up to +5 ms; feedback to ±0.95 for the resonant
  jet whoosh.
- **Phaser** — 2–12 all-pass stages swept 200 Hz–2 kHz in the log domain, with feedback (±0.95)
  wrapped around the whole cascade for resonant peaks between the notches. Allocates no delay buffer.
- **Vibe** — Uni-Vibe style. Exactly 4 all-pass stages, each **staggered** to its own break frequency
  (±0.25 / ±0.75 octaves around one swept centre), driven by an **asymmetric** LFO for the
  lamp-and-photocell throb. Chorus/Vibrato mode switch; no feedback.
- **Character (global)** — always on the output, after the effect and after the Mix blend:
  - **Age** — wow (0.556 Hz) + flutter (12 Hz) + band-limited random drift modulating a ~2 ms
    fractional delay by up to ±1.5 ms. Mono-correlated across channels, as a single tape transport
    would be.
  - **Warmth** — 2× oversampled asymmetric-tanh saturation (DC-biased for even harmonics, with
    slope-normalised makeup so it compresses rather than acting as a volume knob), then a one-pole
    high-cut swept 18 kHz → 4 kHz.
- **Live visualiser** — a vsync-driven view in the centre screen showing either the effect's
  modulation waveform running on the *real* DSP phase, or its response (a breathing time offset for
  the delay family, an analytic swept-notch magnitude spectrum for the all-pass family).
- **Real-time safe throughout** — no allocation and no locking on the audio thread; every buffer,
  filter and oversampler is built in `prepare`. All user-facing parameters are smoothed (20 ms) to
  avoid zipper noise, and LFO phase is continuous across blocks.
- **Full host integration** — every control is an APVTS parameter, so automation, preset save/load
  and host state recall all work; the plugin reports its oversampler latency for delay compensation.

---

## Signal flow

```
                     ┌─── one of ───────────────────────────────────┐
                     │  Chorus  │  Flanger  │  Phaser  │  Vibe      │
 input ──────────────┤  (dry/wet blended by Mix inside the effect)  ├─┐
                     └──────────────────────────────────────────────┘ │
                                                                      ▼
                        ┌──────────────── Character (always on) ────────────────────────────────┐
                        │  wow/flutter delay  →  2× up  →  asym. tanh  →  2× down  →  high-cut  │
                        │        (Age)                    (Warmth)               (Warmth)       │
                        └───────────────────────────────────────────────────────────────────────┘
                                                                      │
                                                                      ▼
                                                                   output
```

The Character stage sits **after** the Mix blend, so it colours the dry signal too — it is a global
output stage, not part of any one effect.

---

## Installing

See latest release, or build from source (below), then:

**VST3 (Windows)** — copy
`build/CozyChorusSuite_artefacts/<config>/VST3/CozyChorus Suite.vst3`
into `C:\Program Files\Common Files\VST3\`, or simply add the build folder to your DAW's VST3
search paths. (`COPY_PLUGIN_AFTER_BUILD` is deliberately **off** — the automatic install needs an
elevated shell.)

**Standalone** — run
`build/CozyChorusSuite_artefacts/<config>/Standalone/CozyChorus Suite.exe`.
Pick your audio device and input in *Options → Audio/MIDI Settings*.

**AU (macOS)** — the `AU` format is added to the build automatically on Apple platforms; copy the
resulting component into `~/Library/Audio/Plug-Ins/Components/`. Note that AU has not been built or
tested on the project's Windows development machine.

---

## Building from source

### Prerequisites

- **CMake ≥ 3.25** (4.2+ if you want the `Visual Studio 18 2026` generator)
- A **C++20** toolchain — developed against Visual Studio 2026 (MSVC v145)
- **JUCE 8.0.14**, included as a git submodule (no Projucer required)

### Clone

```powershell
git clone --recurse-submodules https://github.com/Seank23/CozyChorusSuite.git
cd CozyChorusSuite
```

Already cloned without submodules?

```powershell
git submodule update --init --recursive
```

### Configure and build

```powershell
cmake -B build -G "Visual Studio 18 2026"
cmake --build build --config Debug     # or Release
```

Any other generator works too (`-G Ninja`, Xcode, Makefiles); the generator above is just what the
project is developed with.

Artefacts land in:

```
build/CozyChorusSuite_artefacts/<config>/Standalone/CozyChorus Suite.exe
build/CozyChorusSuite_artefacts/<config>/VST3/CozyChorus Suite.vst3
```

`Source/` is globbed with `CONFIGURE_DEPENDS`, so new source files are picked up on the next build
without editing CMake. In Visual Studio, `CozyChorusSuite_Standalone` is set as the startup project,
so F5 launches the standalone app.

---

## The interface

A fixed **560 × 440** guitar-pedal faceplate, drawn entirely in vector code — no image assets — in a
cozy amber-on-brown palette.

```
┌──────────────────────────────────────────────────────────────┐
│  CozyChorus Suite                            [ Chorus   ▾ ]  │  ← brand plate + effect selector
├────────┬──────────────────────────────┬──────────────────────┤
│        │                              │ ┌─ Character ──────┐ │
│  Mix   │      modulation screen       │ │  Warmth    Age   │ │  ← always visible, global
│  (○)   │      (click to toggle)       │ │   (○)      (○)   │ │
│        │                              │ └──────────────────┘ │
├────────┴──────────────────────────────┴──────────────────────┤
│           Rate     Depth     Width     Voices                │  ← active effect's controls only
└──────────────────────────────────────────────────────────────┘
```

- **Effect selector** (header, right) — switches the active effect. The bottom row re-lays itself
  immediately to show only that effect's controls.
- **Mix** (top left) — the one shared effect control. Greyed out when the Vibe is in Vibrato mode,
  where the DSP forces 100 % wet and ignores it.
- **Character** (top right, framed) — **Warmth** and **Age**, always visible; they apply to whichever
  effect is selected.
- **Screen** (centre) — the live visualiser; click it to swap modes.
- **Bottom row** — the active effect's own Rate / Depth / Width plus its effect-specific controls,
  centred as a group (4 controls for Chorus and Vibe, 5 for Flanger and Phaser).

Knobs are rotary with vertical/horizontal drag and a value read-out underneath. Vibe's **Vibrato**
control is an LED-style toggle rather than a knob.

---

## The modulation visualiser

The centre screen is a live view driven by the display's vsync, fed by two lock-free atomics that the
audio thread publishes once per block. It shows the **real** running DSP phase — which means it
scrolls exactly in step with what you hear, and **freezes when the host stops processing** (a handy
tell that transport is stopped or the plugin is bypassed).

**Click anywhere on the screen to toggle between the two modes.** The current mode is named in the
top-left corner. The mode is view state only — it isn't a parameter and isn't saved with the preset.

### LFO mode — caption "LFO"

Draws the active effect's modulation waveform, 4 cycles across the width, amplitude proportional to
that effect's **Depth**. Sine for Chorus, Flanger and Phaser; the skewed throb shape for the Vibe —
plotted from the DSP's own shape function, not a lookalike.

Use it to *see* what Rate and Depth are doing, and to confirm the Vibe's asymmetry.

### Response mode — caption "Signal" or "Spectrum"

Adapts to the family of the active effect:

- **Chorus / Flanger → "Signal".** A dry sinusoid (pale trace) and a wet copy (amber) shifted
  horizontally by the **live** delay time at 1 px/ms. The modulation reads directly as a breathing
  time offset — you can watch the Flanger's sweep collapse toward zero as Base Delay drops.
- **Phaser / Vibe → "Spectrum".** The analytic magnitude response of the wet/dry sum on a log
  frequency axis (50 Hz – 20 kHz, −40 … +20 dB, gridlines at 0/−40 dB and 100 Hz/1 k/10 k). The
  notches move because the published LFO phase moves. Raise **Stages** to add notches; raise
  **Feedback** to grow the peaks between them.

This is computed analytically from the DSP's own constants — there is **no FFT** and no audio buffer
is ever handed to the GUI.

---

## Parameter reference

Percentages are shown 0–100 in the UI and normalised internally. Every value below is the shipped
default.

### Global

| Control | Range | Default | Notes |
|---|---|---|---|
| **Effect** | Chorus / Flanger / Phaser / Vibe | Chorus | Selects the active effect. |
| **Mix** | 0–100 % | 50 % | Dry/wet for the active effect. Shared across all four. Ignored (and greyed) in Vibe → Vibrato. |
| **Warmth** | 0–100 % | 30 % | Character: drives saturation amount **and** the high-cut together (18 kHz at 0 % → 4 kHz at 100 %). |
| **Age** | 0–100 % | 30 % | Character: wow/flutter depth. 0 % = a static, inaudible 2 ms delay; 100 % = full tape wander. |

### Chorus

| Control | Range | Default | Notes |
|---|---|---|---|
| Rate | 0.05–5 Hz | 0.6 Hz | LFO speed (skewed taper for resolution at the slow end). |
| Depth | 0–100 % | 70 % | Delay modulation, up to ±7 ms around a 20 ms base. |
| Width | 0–100 % | 75 % | Right-channel LFO phase offset, up to 90° at 100 %. 0 % = mono-correlated. |
| Voices | 1–3 | 3 | Parallel delay taps, phase-offset and spread ±4 ms around the base delay. |

### Flanger

| Control | Range | Default | Notes |
|---|---|---|---|
| Rate | 0.05–5 Hz | 0.5 Hz | |
| Depth | 0–100 % | 80 % | Sweeps the delay **upward** from base, up to +5 ms. |
| Width | 0–100 % | 50 % | |
| Feedback | −95…95 % | 60 % | Comb feedback. Negative inverts, thinning the comb; high positive values ring. |
| Base Delay | 0.2–5 ms | 0.65 ms | The sweep floor — the shortest delay reached. Low = metallic; high = chorus-like. |

### Phaser

| Control | Range | Default | Notes |
|---|---|---|---|
| Rate | 0.05–5 Hz | 0.8 Hz | |
| Depth | 0–100 % | 80 % | How much of the 200 Hz–2 kHz range the sweep covers. |
| Width | 0–100 % | 50 % | |
| Stages | 2–12 | 6 | All-pass stages; roughly half as many notches. Any integer, odd values included. |
| Feedback | −95…95 % | 30 % | Wraps the whole cascade — resonant peaks between the notches. |

### Vibe

| Control | Range | Default | Notes |
|---|---|---|---|
| Rate | 0.05–5 Hz | 2.5 Hz | Faster than the other effects by default — Uni-Vibe territory. |
| Depth | 0–100 % | 80 % | Span of the swept centre frequency. |
| Width | 0–100 % | 50 % | |
| Vibrato | Off / On | **On** | Off = Chorus mode (blended via Mix, swirly). On = Vibrato (100 % wet, so the swept group delay reads as pitch wobble). |

---

## Getting a sound — starting points

The defaults are deliberately usable, but here's where to go from them:

**Lush 80s chorus** — Chorus, Rate ~0.4 Hz, Depth 50–70 %, Voices 3, Width 100 %, Mix 40 %.
Add Warmth ~40 % to take the edge off. Small Age (20 %) makes it breathe.

**Subtle thickening** — Chorus, Voices 1, Depth 25 %, Mix 25 %, Width 40 %. Almost inaudible as an
effect, but the guitar sits wider.

**Jet flanger** — Flanger, Feedback 80–90 %, Depth 100 %, Base Delay 0.2–0.5 ms, Mix 50 %. Watch the
"Signal" view: the wet trace should swing hard against the dry one.

**Hollow metallic comb** — Flanger with **negative** feedback (−60 %), Rate slow (0.15 Hz), Base
Delay ~1 ms.

**Classic 4-stage phaser** — Phaser, Stages 4, Feedback 0–20 %, Rate 0.5 Hz, Depth 80 %, Mix 50 %.
Push Stages to 10–12 with Feedback 60 % for a much denser, more vocal sweep.

**Uni-Vibe chop** — Vibe, Vibrato **off** (Chorus mode), Rate 4–5 Hz, Depth 100 %, Mix 50 %.
Turning Vibrato **on** with the same settings gives the pitch-wobble Leslie-ish voice instead.

**Worn tape deck** — any effect, then Age 70–90 % and Warmth 60 %. Age alone at high settings on a
slow Chorus is a very convincing "this cassette has been played too often".

**Tips**

- **Width 0 %** makes both channels read the LFO at the same phase — use it when you need a strictly
  mono-compatible result.
- **Warmth is not a volume knob.** The saturation is slope-normalised for unity small-signal gain, so
  turning it up compresses peaks and adds harmonics rather than getting louder.
- **Age at 0 % is genuinely off** (a fixed 2 ms delay), so you can use Warmth alone.
- Automating **Rate** is smooth — since the Rate smoother runs at sample rate, sweeping it from a
  host lane doesn't step or zipper.

---

## Host integration notes

- **Channel layouts** — mono → mono and stereo → stereo only. The input layout must match the output;
  the plugin rejects mismatched or wider layouts. Stereo Width has no effect in mono (there is no
  second channel to offset).
- **Latency / PDC** — the Character stage's 2× oversampler adds a few samples of latency, reported
  once via `setLatencySamples` so the host can compensate. It's built with integer latency, so the
  reported figure is exact. The Age wow/flutter delay (~2 ms nominal) is **not** reported: it is a
  time-varying pitch-modulation delay and is part of the effect, exactly as on a real tape machine.
- **State** — the full APVTS tree is saved and restored with the host session and with presets,
  including all four effects' parameter sets (not just the active one), so switching effects after a
  recall gives you back the settings you left.
- **Automation** — every control is a host-visible parameter. Note that per-effect Rate/Depth/Width
  are *separate* parameters (`chorusRate`, `flangerRate`, …), so automation you write for one effect
  doesn't disturb the others. **Mix**, **Warmth** and **Age** are shared/global.
- **Bypass** — the effect and Character stages both honour the host's bypass context.

---

## Known limitations

Honest list of what's still rough — none of these are crashes or dropouts:

- **Mode-switch clicks.** Toggling Vibe's Vibrato jumps the effective mix 0.5 → 1.0 instantly, and
  changing the Phaser's Stages count takes effect immediately (stale state on removed stages is
  reused if the count goes back up). Both can produce a click at the moment of the change.
- **Spectrum view vs. Vibrato.** The Phaser/Vibe spectrum plot blends by the `mix` parameter even for
  the Vibe, where Vibrato mode forces 100 % wet in the DSP — so with Vibrato on (the default) the
  drawn curve is flatter than what you actually hear. Cosmetic only.
- **Flanger sweeps upward only** from Base Delay, so its top-end reach is limited compared with a
  bipolar sweep. Through-zero flanging is not implemented.
- **Phaser/Vibe assume at most 2 channels** — consistent with the supported mono/stereo layouts, but
  the limit is a fixed array bound rather than a guarded one.
- **Chorus voice-count stepping.** Voice 0's base delay is 20/18/16 ms at 1/2/3 voices, so the
  visualiser's displayed offset steps when you turn the Voices knob.
- **Tuning is by ear.** The Vibe's stagger spread and asymmetry constant, the Character stage's drive/
  bias/cutoff endpoints, and the wow/flutter weights were all tuned by listening, not measured
  against reference hardware.
- No `pluginval` run or automated DSP test suite yet.

---

## Project layout

```
Source/
  PluginProcessor.h/.cpp        AudioProcessor: owns the APVTS, one instance of each effect and the
                                Character stage; builds a per-effect POD each block and dispatches;
                                publishes the visualiser atomics
  Parameters.h                  All parameter IDs and the APVTS layout in one place
  Editor/
    CCSAudioProcessorEditor     The faceplate: selector, knobs, zone layout, per-effect visibility
    CCSLookAndFeel              Custom LookAndFeel_V4 skin + the shared Palette (flat vector, no assets)
    LabeledKnob                 Reusable caption-over-slider composite
    ModulationVisualiser        The centre screen: LFO and Response modes, VBlank-driven
    EditorConstants.h           Layout metrics shared by resized() and paint()
  dsp/
    ModulationEffect.h          Abstract base: Prepare / Process / Reset (+ LFO phase export)
    NullEffect.h                Pass-through; now only the unreachable default guard
    LFO.h/.cpp                  Shared LFO: continuous phase, phase-offset reads, four shapes
    ChorusEffect                Delay-line family
    FlangerEffect               Delay-line family + feedback comb
    PhaserEffect                All-pass family, N-stage cascade + feedback
    VibeEffect                  All-pass family, 4 staggered stages + asymmetric LFO
    CharacterStage              Global tape stage (not a ModulationEffect)
```

Parameters reach the DSP as plain per-effect structs (`ChorusParameters`, `FlangerParameters`, …)
built once per block from cached APVTS atomic pointers — effects never read the APVTS themselves.

Code style is documented in **`CPP-STYLE-GUIDE.md`** and enforced by `.clang-format` (tabs, Allman
braces, one `CozyChorus` namespace, `PascalCase` methods, `m_`-prefixed members). JUCE's own override
names and signatures are kept as JUCE spells them; the JUCE submodule is never restyled.

---

## Documentation

- **`CLAUDE.md`** — the living project brief: architecture, settled design decisions, milestone list,
  and the DSP reference for every effect.
- **`DEVLOG.md`** — session-by-session log (newest first): what was done, what was decided, what's next.
- **`CPP-STYLE-GUIDE.md`** — house C++ style.

---

## Licensing

This repository does not currently carry a licence file. Note that **JUCE** (included as a submodule)
is distributed under its own terms — JUCE 8 is dual-licensed under the AGPLv3 and a commercial
licence — so any redistribution of built binaries needs to satisfy JUCE's licensing as well.
