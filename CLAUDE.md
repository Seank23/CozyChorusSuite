# CozyChorus Suite

A cross-platform audio plugin (**VST3 + AU + Standalone**): a suite of four classic
guitar-pedal modulation effects — **Chorus, Flanger, Phaser, Vibe** (Uni-Vibe style) —
plus a global **Character** (tape-warmth) stage on the output. The user selects the active
effect; each exposes its own controls. Guitar-oriented, works in mono and stereo.
C++20 / JUCE 8 / CMake.

---

## Current status

- **Phase:** Milestone 6b complete — **the plugin is feature-complete: all planned DSP and all planned GUI
  are shipped.** The centre "screen" reserved by M6a now hosts a live **`ModulationVisualiser`** with two
  click-toggled modes: **LFO** (the active effect's modulation waveform, scrolling in lock-step with the
  real DSP phase, amplitude ∝ Depth) and **Response** (family-adaptive — dry/wet sinusoids offset by the
  live delay for Chorus/Flanger, an analytic swept-notch **magnitude spectrum** for Phaser/Vibe). The
  audio→GUI hand-off is two lock-free `std::atomic<float>`s published once per block. M6a had given the
  plugin its flat-vector "guitar-pedal" face: a custom `CCSLookAndFeel` (cozy amber-on-brown skin, no
  image assets), a reusable `LabeledKnob` composite, and a fixed **560×440 zone layout** — Mix top-left, a
  framed **CHARACTER** box (Warmth + Age) top-right, the active effect's controls centred in a bottom row.
  Chorus + Flanger (delay-line family) and Phaser + **Vibe** (all-pass family) finished the DSP-effect
  track at M4; **M5 added a global `CharacterStage`** (oversampled asymmetric-tanh saturation + high-cut
  tone, one **Warmth** macro) applied to the output after the active effect; **M5b added tape-age
  wow/flutter** to that same stage — a modulated fractional delay (slow **wow** + fast **flutter** +
  band-limited random drift) driven by a second **Age** macro, running *before* the saturation. Nothing on
  the original milestone list remains; further work is polish (by-ear tuning, `pluginval`, DSP tests).
- **M6b was *not* purely view-only** (unlike 6a): besides the read-only visualiser exports, the LFO rate
  update `m_LFO.SetFrequency(m_RateHz.getNextValue())` moved from **once per block to once per sample** in
  all four effects — the Rate smoother now actually runs at sample rate (previously it advanced one step
  per block, stretching the 20 ms ramp by the block size). Audible only as smoother/faster Rate changes;
  the steady-state sound is unchanged.
- **Params are per-effect (post-M4):** Rate / Depth / Stereo Width were split from shared params into
  **per-effect** APVTS params (`chorusRate`, `flangerRate`, …), so each effect carries its own defaults.
  **`mix` is the only shared *effect* param**; **`warmth`** and **`age`** are separate **global stage**
  params (the Character stage, not effect controls).
- **Character stage reports latency (PDC):** the 2× oversampler adds a few samples of latency, reported
  once via `setLatencySamples(m_CharacterStage.GetLatencySamples())` in `prepareToPlay` — the suite's
  first use of host delay compensation. The M5b wow/flutter delay line adds a ~2 ms nominal delay that
  is **deliberately not** PDC-reported (it is a time-varying pitch-modulation delay, part of the effect,
  not a fixed processing latency).
- A hand-written **`CCSAudioProcessorEditor`** (rotary knobs + effect selector, per-effect control
  visibility) has replaced the generic editor, skinned by a custom **`CCSLookAndFeel`**, laid out as a
  guitar-pedal faceplate (M6a) and animated by the **`ModulationVisualiser`** in the centre screen (M6b).
  No GUI work from the milestone list remains.
- Builds as **VST3 + Standalone** via CMake + JUCE **8.0.14** with the `Visual Studio 18 2026`
  generator (MSVC v145). Artefacts land in
  `build/CozyChorusSuite_artefacts/<config>/{Standalone,VST3}/`.
- `COPY_PLUGIN_AFTER_BUILD` is **FALSE**: auto-installing the VST3 into
  `C:\Program Files\Common Files\VST3` needs admin. Point your DAW at the build folder, or
  copy the `.vst3` there once from an elevated shell.
- Git repo on `main`, remote `origin` = https://github.com/Seank23/CozyChorusSuite.git.
- Update this section at the end of each milestone.

---

## Maintaining this file

`CLAUDE.md` does not update automatically — keep it current by hand as part of each
milestone's work (the edits go in that milestone's proposed file list and its single commit).
At the end of every milestone, refresh:

- **Current status** — new phase and what's now loadable.
- **Build order & milestones** — mark the finished milestone done.
- **Architecture** — add new files to the `Source/` tree as they appear.
- **Settled design decisions** — record any decision made mid-milestone.
- **DSP reference / parameter tables** — reconcile with the params actually shipped.

Also append a **`DEVLOG.md`** entry every session and at each milestone (Done / Decisions /
Next up / Open questions; newest entry at the top). `DEVLOG.md` is the portable, committed
session log — it's how work is resumed in a new session or on a different PC, since Claude's
local memory files and conversation transcripts do **not** travel with the repo.

(Separate mechanism: Claude also keeps its own memory files in `~/.claude/…/memory/` current.)

---

## Working agreement (how to collaborate on this repo)

These are hard rules, not preferences:

- **One milestone at a time.** Stop at the end of each milestone so the user can build
  and audition before continuing. Never run ahead to later milestones.
- **Propose before coding.** Before writing any code for a milestone, state the intended
  approach and the exact file changes, then wait for explicit go-ahead.
- **Do not scaffold/create files/run commands until the user says** "start Milestone N"
  (or similar) for that milestone.
- When the user says **"let me implement this / the next step"**, do **not** write code —
  explain how you would approach the implementation to guide them.
- If the brief is ambiguous or you'd deviate from it, **raise it and ask** rather than guessing.
- **One commit per working milestone**; keep a loadable plugin at the end of every milestone.

The user is an experienced C++ / real-time-DSP / graphics developer but **new to JUCE** —
explain JUCE-specific idioms, APIs, and conventions; don't explain C++ or DSP basics.

---

## Real-time safety (HARD constraints, apply throughout)

- **No allocation and no locking on the audio thread.** All allocation happens in
  `prepare` / `prepareToPlay`, never in `processBlock` / `process`.
- Read parameters via **APVTS atomic pointers**; never lock on the audio thread.
- Wrap user-facing params in **`juce::SmoothedValue`** to avoid zipper noise.
- Keep **LFO phase continuous across blocks**; never reset it per block.

---

## Build order & milestones

Fixed order (delay-line family first, then all-pass family):

1. **Milestone 0 — Scaffolding. ✅ Done.** Empty plugin that loads and passes audio
   unchanged. APVTS with `mix` + `effectType`; `ModulationEffect` base + `NullEffect`
   pass-through; `GenericAudioProcessorEditor`.
2. **Milestone 1 — Chorus. ✅ Done.** First real effect (delay-line family): shared `LFO` +
   `ChorusEffect` (fractional `DelayLine`, Rate/Depth/Mix/Width + a 1–3-voice ensemble, per-channel
   LFO phase offset for stereo width, all params smoothed). `Voices` is wired and exposed (added
   post-M1 in Session 5); selectable LFO shape still exists in code but is not user-selectable.
3. **Milestone 2 — Flanger. ✅ Done.** Reuses the delay-line skeleton with **feedback** + a shorter
   **0.5–5 ms base delay**: `FlangerEffect` (pop-before-push feedback comb;
   Rate/Depth/Mix/Width/Feedback/BaseDelay, all smoothed). A custom `CCSAudioProcessorEditor`
   (`Source/Editor/`) landed alongside — effect selector + rotary knobs, per-effect control
   visibility. See the Flanger notes below for shipped tuning caveats.
4. **Milestone 3 — Phaser. ✅ Done.** All-pass cascade (new core, first non-delay-line effect):
   `PhaserEffect` — a cascade of N (2–12) hand-rolled one-pole **TPT all-pass** stages; the shared LFO
   modulates the all-pass cutoff, log-spaced **200 Hz–2 kHz**; **feedback** (±0.95) wraps the whole
   cascade for resonance. Rate/Depth/Width are per-effect controls (Mix is the only shared param); **Stages + Feedback** are
   Phaser-only. All params smoothed. No delay buffer.
5. **Milestone 4 — Vibe. ✅ Done.** Uni-Vibe (all-pass family, the last effect): `VibeEffect` — a
   cascade of **exactly 4** hand-rolled TPT all-pass stages, each **staggered** to its own break
   frequency (fixed per-stage log offset around one LFO-swept centre), driven by an **asymmetric LFO**
   (lamp/photocell throb, shaped inside `VibeEffect`), with a **Chorus / Vibrato mode** switch (Vibrato
   = 100 % wet). **No feedback, no delay buffer.** Rate/Depth/Width are per-effect controls (Mix is the only shared param); the
   only Vibe-specific param is the `vibeMode` bool. All effects now exist — the effect list is complete.
6. **Milestone 5 — Character (tape warmth). ✅ Done.** First non-effect DSP: a global `CharacterStage`
   (not a `ModulationEffect`) applied to the output **after** the active effect. **2× oversampled**
   (`juce::dsp::Oversampling`, min-phase half-band IIR) **asymmetric-tanh saturation** with a built-in
   slope-normalised makeup (unity small-signal gain) for even-harmonic "tape" colour, followed by a
   one-pole **high-cut** (`FirstOrderTPTFilter`). A single **Warmth** macro drives both drive and
   cutoff. Global `warmth` param (default 30 %). Reports oversampler latency via `setLatencySamples`
   (first PDC use).
7. **Milestone 5b — Character (tape age / wow+flutter). ✅ Done.** Adds slow pitch instability to the
   same `CharacterStage`, **before** the saturation: a modulated fractional delay
   (`juce::dsp::DelayLine<Lagrange3rd>`, 2 ms centre ±1.5 ms) whose delay is swept by **wow** (0.556 Hz
   sine), **flutter** (12 Hz sine) and **band-limited random drift** (a per-block `juce::Random` sample
   through a one-pole ~2 Hz LPF with makeup gain). One shared modulation value across channels
   (mono-correlated). A single **Age** macro (global `age` param, default 30 %) scales the whole sweep.
   Wow/flutter only — vinyl/tape *noise* was dropped from scope. Same POD/`SetParameters` shape; an Age
   knob joins Warmth in the editor (always visible).
8. **Milestone 6a — Pedal UI. ✅ Done.** First pure-GUI milestone (no DSP). A custom
   **`CCSLookAndFeel`** (`juce::LookAndFeel_V4`: flat-vector rotary, combo box + popup, LED toggle;
   cozy amber-on-brown `Palette`, no image assets) skins the whole editor via one
   `setLookAndFeel(&m_LookAndFeel)` that cascades to every child. A reusable **`LabeledKnob`**
   composite (caption `Label` over a `Slider` with a value read-out) replaces the old paint-time
   caption band. The editor is re-laid as a fixed **560×440 guitar-pedal faceplate** — header
   (brand plate + effect selector), **Mix** top-left, a framed **CHARACTER** box (Warmth + Age)
   top-right, a recessed **"screen"** reserved in the centre (kept as `m_ScreenZone` for M6b), and
   the active effect's controls **centred** in a bottom row (replacing the wrap-at-4 grid). Mix is
   greyed (`setEnabled(false)`) in Vibe's Vibrato mode. Lifetime-safe: `m_LookAndFeel` declared
   first, `setLookAndFeel(nullptr)` in the destructor.
9. **Milestone 6b — Modulation visualiser. ✅ Done.** The live animated view in the centre screen zone: a
   `ModulationVisualiser` (plain `juce::Component` + `juce::VBlankAttachment`, **no OpenGL**) with two
   click-toggled modes — **LFO** (the active effect's modulation waveform scrolling on the real DSP phase)
   and **Response** (dry/wet sinusoid pair offset by the live delay for Chorus/Flanger; an analytic
   swept-notch magnitude spectrum for Phaser/Vibe). Fed by two lock-free atomics published from
   `processBlock` (`m_VisualPhase`, `m_VisualDelayInSamples`). Last item on the milestone list —
   **the plugin is feature-complete**; anything further is polish.

---

## Tech stack

- **JUCE 8.x**, added as a **git submodule** (latest, pinned to a commit), **no Projucer**.
- **C++20**.
- **CMake ≥ 3.25** via `juce_add_plugin` (hand-rolled minimal `CMakeLists.txt`).
- Formats: **VST3, Standalone** (+ **AU on macOS only**). LV2 optional.
- Prefer **`juce::dsp`** modules where they exist: `DelayLine`, `Oscillator`,
  `FirstOrderTPTFilter`, `SmoothedValue`.
- Parameters via **`AudioProcessorValueTreeState` (APVTS)** — host automation, preset
  save/load, thread-safe reads.
- Optional/later: **pluginval** validation step, **Catch2** DSP tests.

### Toolchain (Windows dev box, confirmed 2026-07-14)

- **Visual Studio Community 2026 v18.7.3** — MSVC toolset **v145**, C++20 OK.
- **CMake 4.4** — supports the `Visual Studio 18 2026` generator (added in CMake 4.2).
- No Ninja installed (not needed).
- **AU cannot be built or tested here** — Audio Unit is macOS-only.
- Risk: v145 is new and JUCE compiles with warnings-as-errors, so a fresh MSVC warning could
  trip the build. Latest JUCE mitigates; fallback is to bump JUCE or relax warnings-as-errors
  on the JUCE target. Flag it if hit; don't loosen pre-emptively.

### Build commands

```powershell
cmake -B build -G "Visual Studio 18 2026"
cmake --build build --config Debug     # or Release
```

JUCE writes binaries under `build/<target>_artefacts/<config>/<Format>/`
(e.g. `build/CozyChorusSuite_artefacts/Debug/Standalone/CozyChorus Suite.exe`).

---

## Code style

All hand-written C++ in `Source/` follows the project house style in **`CPP-STYLE-GUIDE.md`**
(tabs, Allman braces, single project namespace, `PascalCase` types/methods, `m_`-prefixed
private members, `#pragma once`, `struct` = passive data / `class` = behavior,
`unique_ptr`/`shared_ptr` ownership). Formatting is enforced by **`.clang-format`** and editor
defaults by **`.editorconfig`** — run clang-format before committing. Two carve-outs:
JUCE-mandated override names and signatures (e.g. `prepareToPlay`, `processBlock`) keep JUCE's
own naming, and the **JUCE submodule is never restyled**.

---

## Architecture

```
Source/
  PluginProcessor.h / .cpp   // AudioProcessor: owns APVTS + effect instances + CharacterStage; routes processBlock to active effect, then runs the Character stage on the output; reports oversampler latency via setLatencySamples; publishes the M6b visualiser atomics (phase + delay) and caches the ProcessSpec for the GUI
  Parameters.h               // parameter IDs + APVTS layout in one place
  Editor/
    CCSAudioProcessorEditor.h / .cpp  // custom editor: effect selector + LabeledKnobs, per-effect control visibility (30 Hz Timer), pedal zone layout (header / Mix / CHARACTER box / visualiser / centred bottom row); owns the CCSLookAndFeel + the ModulationVisualiser; createEditor() returns this
    CCSLookAndFeel.h / .cpp  // custom juce::LookAndFeel_V4 skin (M6a): drawRotarySlider / drawComboBox / drawPopupMenuBackground / drawToggleButton + the shared Palette namespace (flat-vector amber-on-brown, no assets; Palette::Trace added in M6b for the dry curve)
    LabeledKnob.h / .cpp     // reusable composite view (M6a): a caption Label stacked over a rotary Slider with a value read-out; getSlider() exposes the inner slider for APVTS attachment
    ModulationVisualiser.h / .cpp  // the centre-screen animated view (M6b): juce::Component + VBlankAttachment; Mode::LFO (modulation waveform on the live DSP phase) / Mode::Response (dry-vs-delayed sinusoids for Chorus+Flanger, analytic magnitude spectrum for Phaser+Vibe); click toggles the mode
    EditorConstants.h        // shared layout metrics (kMargin, kHeaderHeight, zone widths, kKnobWidth/Height, corner radii, kVisCyclesShown/kVisPxPerMs) read by BOTH resized() and paint()
  dsp/
    ModulationEffect.h       // abstract base: Prepare(spec) / Process(context) / Reset(); + GetCurrentLFOPhase() (M6b, read-only export for the visualiser)
    NullEffect.h             // pass-through; now ONLY the `default` guard in GetActiveEffect() — every built effect routes to itself
    LFO.h / .cpp             // shared LFO: continuous phase, per-channel phase-offset reads, Hz rate; sine/triangle/saw/square (Chorus/Flanger/Phaser use sine); GetPhase() accessor lets Vibe apply its own asymmetric shape
    ChorusEffect.h / .cpp    // Chorus (delay-line family) — Milestone 1, done
    FlangerEffect.h / .cpp   // Flanger (delay-line family, feedback comb) — Milestone 2, done
    PhaserEffect.h / .cpp    // Phaser (all-pass family, TPT all-pass cascade + feedback) — Milestone 3, done
    VibeEffect.h / .cpp      // Vibe (all-pass family, 4 staggered TPT stages + asymmetric LFO + Chorus/Vibrato mode, no feedback) — Milestone 4, done
    CharacterStage.h / .cpp  // Character (global tape stage, NOT a ModulationEffect): M5b wow/flutter fractional-delay pitch modulation (Age macro) → M5 2x oversampled asymmetric-tanh saturation + one-pole high-cut (Warmth macro) — Milestones 5 + 5b, done
```

### Design principle: two DSP families, one shared skeleton

- **Delay-line family** — Chorus, Flanger: a modulated fractional delay line
  (flanger = chorus + feedback + shorter base delay).
- **All-pass family** — Phaser, Vibe: a cascade of first-order all-pass filters
  (vibe = phaser with staggered stages + asymmetric LFO).

All four are driven by a shared **LFO** and expose the same control set — **Rate / Depth / Stereo Width**,
now **per-effect** APVTS params (each with its own default), plus a shared **Mix**.

### Settled design decisions

- **Project namespace:** all hand-written code lives in `namespace CozyChorus`
  (`createPluginFilter()` stays global, as JUCE requires).
- **Method naming:** our own methods are `PascalCase`; JUCE's own names are kept when
  overriding or calling into JUCE (`prepareToPlay`, `processBlock`, `delayLine.prepare(...)`).
- **LFO** is a single shared class **instanced by each effect** (not one processor-owned
  instance) — allows per-channel / per-voice phase offsets and vibe's asymmetric shape.
- **`ModulationEffect` base interface:** `Prepare(const juce::dsp::ProcessSpec&)`,
  `Process(const juce::dsp::ProcessContextReplacing<float>&)`, `Reset()`.
- **Parameter passing (settled M1):** the processor caches APVTS atomic pointers and, each block,
  builds a plain per-effect POD (`ChorusParameters`, percentages converted to 0–1) and hands it to
  `ChorusEffect::SetParameters`, which feeds per-control `SmoothedValue`s. Effects expose their own
  `SetParameters(const XxxParameters&)` rather than reading the APVTS directly.
- **Per-effect Rate/Depth/Width params (post-M4):** Rate, Depth and Stereo Width were split from shared
  APVTS params into **per-effect** params — `chorusRate`/`chorusDepth`/`chorusWidth`, and likewise
  `flanger*`/`phaser*`/`vibe*` — each defaulting from that effect's `XxxParameters` POD in
  `CreateParameterLayout()`. **`mix` is now the only shared *effect* param** (the M5 `warmth` param is
  global to the Character stage, not an effect control). Rationale: a single shared knob
  can't carry four different sensible defaults, so per-effect params let every effect boot at its own
  sweet spot and automate independently. The POD-per-block dispatch is unchanged (only the source
  param IDs moved); the editor holds a matching per-effect slider + attachment set, shown/hidden by
  selection.
- **Stereo width = LFO phase offset (M1):** both channels read one continuous LFO; the right
  channel is read at `+width*0.25` cycle (up to 90° at 100%), so Width 0 % ⇒ mono-correlated.
  Chorus is a bipolar sine around a 20 ms base delay, ±7 ms at full depth.
- **Sources are globbed:** `CMakeLists.txt` uses `file(GLOB_RECURSE … CONFIGURE_DEPENDS)` over
  `Source/*.cpp|*.h`, so new files are collected on the next build with no CMake edit.
- **`PluginProcessor`** owns the APVTS and one instance of each effect; an `effectType`
  choice parameter selects which effect `processBlock` dispatches to (Vibe → `NullEffect`
  until built).
- **Flanger topology (M2):** the delay-line skeleton reused as a **feedback comb** — per sample
  `popSample` (read the modulated delay) **then** `pushSample(input + feedback·wet)`, i.e. read
  before write, so the minimum effective delay is 1 sample (hence `MIN_DELAY_SAMPLES = 1`). Base
  delay 0.5–5 ms; the LFO sweeps the delay **upward** from base by up to +5 ms (`0.5 + 0.5·sin`,
  unipolar); feedback ±0.95; stereo width reuses the Chorus per-channel phase-offset trick.
  Rate/Depth/Width are **per-effect params** (`flangerRate`/`flangerDepth`/`flangerWidth`); `mix` is the
  only shared param; Feedback + Base Delay are Flanger-only.
- **Editor (M2, extended M3):** replaced `GenericAudioProcessorEditor` with a hand-written
  `CCSAudioProcessorEditor` (`Source/Editor/`). A **per-effect** set of rotary sliders (each effect owns
  its own Rate/Depth/Width knobs) + a shared **Mix** knob and an effect
  selector; a 30 Hz `Timer` watches the `effectType` param and shows/hides the per-effect controls
  (Voices for Chorus; Feedback + Base Delay for Flanger; **Stages + Feedback for Phaser**). `resized()`
  lays the *visible* controls out in a wrapping grid; `paint()` fills the background and draws the
  title + a caption above each visible knob. No custom `LookAndFeel` yet — a later polish pass.
- **Phaser topology (M3):** the all-pass family's shared skeleton — a per-channel cascade of N
  hand-rolled **one-pole TPT all-pass** stages (`g = tan(π·fc/fs)`, `G = g/(1+g)`; each stage returns
  `2·lowpass − input` and carries one state variable). The shared LFO modulates the cutoff `fc` in the
  **log domain** (centre/half-span precomputed in `Prepare` from `MIN_FC_HZ=200`/`MAX_FC_HZ=2000`), so
  `fc = exp(logCenter + logHalfSpan·depth·lfo)`, clamped to the range. **Feedback** wraps the whole
  cascade: `input += feedbackState·feedback` before the stages, `feedbackState = cascadeOutput` after
  (±0.95). Stereo width reuses the Chorus/Flanger per-channel LFO phase-offset trick. Rate/Depth/
  Width are **per-effect params** (`phaserRate`/…); `mix` is the only shared param; **Stages** (2–12,
  default 6) and **Feedback** (default 0) are Phaser-only. No delay line — this is the first effect that allocates no delay buffer.
- **Vibe topology (M4):** the Phaser's all-pass skeleton, reworked into a Uni-Vibe by three deltas —
  everything else (TPT kernel, exponential log sweep, per-channel width offset, POD-per-block dispatch,
  show/hide editor) is reused. (1) **Fixed 4 stages, each staggered** to its own break frequency: one
  LFO-swept centre plus a constant per-stage log offset (`{−0.75, −0.25, +0.25, +0.75}` octaves,
  precomputed in `Prepare`), so `G = g/(1+g)` is **recomputed per stage** (4 `tan()`/sample/channel).
  (2) **Asymmetric LFO owned by `VibeEffect`:** the shared `LFO` stays shape-agnostic and only exposes
  `GetPhase()`; `VibeEffect::GetAsymmetricShape` warps the phase (piecewise-linear, `ASYM_K = 0.35`)
  then takes a sine — a smooth, skewed throb, no `SetShape` call. (3) **Chorus / Vibrato mode**
  (`vibeMode` **bool** + `ToggleButton`): Vibrato forces `effectiveMix = 1.0` (100 % wet, so the swept
  group delay reads as pitch wobble); Chorus uses the shared Mix blend. **No feedback** (no
  `m_FeedbackState`), **no delay buffer** — only fixed `std::array` all-pass state. `fc` clamped to the
  200 Hz–2 kHz sweep range. Rate/Depth/Width are **per-effect params** (`vibeRate`/…); `mix` is the only
  shared param; the mode bool is the only Vibe-specific param. Stagger spread + `ASYM_K` are **tuned by ear, not measured**.
- **`NullEffect` is now only the guard:** with all four effects built, every real `EffectType`
  selection routes to its own effect in `GetActiveEffect()`; `NullEffect` remains solely the
  unreachable `default:` safety net.
- **Character stage (M5):** a **global** tape-warmth stage, deliberately **not** a `ModulationEffect`
  and **not** in the `effectType` switch — `PluginProcessor` owns one `CharacterStage m_CharacterStage`
  and runs it **unconditionally on the output after** `GetActiveEffect().Process(context)`, so it
  colours whichever effect is active (and the dry signal too, since it sits post-Mix). It reuses the
  effects' `Prepare/Process/Reset` + POD-`SetParameters` shape for consistency but stands alone. DSP:
  **2× oversampled** (`juce::dsp::Oversampling`, `filterHalfBandPolyphaseIIR` = minimum-phase, chosen
  for low latency) **asymmetric-tanh saturation** —
  `y = (tanh(drive·(x + DC_BIAS)) − tanh(drive·DC_BIAS)) · invS0`, with `drive = 1 + w·DRIVE_MAX`
  (`DRIVE_MAX = 2.5`), `DC_BIAS = 0.15` giving even harmonics, and `invS0 = 1/(drive·(1 − tanh²(drive·DC_BIAS)))`
  a **slope-normalised makeup** (unity small-signal gain, so Warmth compresses peaks rather than acting
  as a volume knob) — followed by a one-pole **high-cut** (`FirstOrderTPTFilter`, lowpass) swept
  `fc = MAX_WARMTH_CUTOFF_HZ·(MIN/MAX)^w` from **18 kHz → 4 kHz**. One **Warmth** macro (0–1) drives
  both drive and cutoff; `warmth` is smoothed 20 ms but read **once per block** (coefficients constant
  per block). All state (oversampler, filter) allocated in `Prepare`; `Process` is allocation-free
  (`processSamplesUp`/`Down` reuse the oversampler's internal buffers). **Latency:** the oversampler's
  IIR adds a few samples, reported once via `setLatencySamples(m_CharacterStage.GetLatencySamples())`
  in `prepareToPlay` (the suite's first PDC). **`warmth` is a global stage param** (default 30 %), not
  an effect control and independent of `effectType`. (`DRIVE_MAX`/cutoff endpoints were retuned by ear
  during M5b — was 4 / 6.5 kHz at M5.) Drive/bias/cutoff endpoints are **tuned by ear**.
  - **Editor:** a **Warmth** rotary knob, always visible (like Mix, independent of `effectType`), added
    right after the Mix knob in `GetAllComponents()`; caption "Warmth".
  - **Shipped-M5 fix:** the saturation loop originally iterated the *pre*-oversampling sample/channel
    counts (`< numSamples - 1`, `< numChannels - 1`), leaving the back half of every 2× block
    unsaturated → a per-block discontinuity heard as low-frequency crackle, and skipping the last
    channel. Fixed to iterate the oversampled block's own `getNumSamples()`/`getNumChannels()`.
- **Character tape-age / wow+flutter (M5b):** the same `CharacterStage` gained a pitch-instability stage
  that runs **first in `Process`, before the oversampled saturation**, sharing the stage's `Prepare/
  Process/Reset` + POD-`SetParameters` shape. DSP: a modulated fractional delay
  (`juce::dsp::DelayLine<Lagrange3rd>`, max `(CENTER+HALF_SPAN)` ms + 4 headroom samples) whose delay is
  `(CENTER_DELAY_MS + HALF_SPAN_DELAY_MS·mod·age)·0.001·fs` with `CENTER = 2 ms`, `HALF_SPAN = 1.5 ms`.
  `mod = WOW_WEIGHT·wowSine + FLUTTER_WEIGHT·flutterSine + NOISE_WEIGHT·driftMakeup` (weights
  `0.8 / 0.04 / 0.02`) — two `LFO` instances (`WOW_FREQUENCY = 0.556 Hz`, `FLUTTER_FREQUENCY = 12 Hz`,
  advanced once per sample) plus **band-limited random drift**: one `juce::Random` sample **per block**,
  bipolar, run through a one-pole LPF (`NOISE_LPF_HZ = 2 Hz`, coeff precomputed in `Prepare`) with a
  `1/√(coeff/2)` **makeup gain** to restore amplitude after the heavy lowpass. **One shared `mod`/delay
  across channels** (mono-correlated, physically correct for one transport). A single **Age** macro
  (global `age` param, default 30 %, smoothed 20 ms, read **per sample**) scales `mod`, so Age 0 % ⇒ a
  static 2 ms delay (inaudible) and 100 % ⇒ full wander. **No feedback.** State (delay line, LFOs,
  noise) allocated/prepared in `Prepare`; `Reset` flushes all of it. The ~2 ms centre delay is
  **not** PDC-reported (`GetLatencySamples()` still returns only the oversampler) — it's a time-varying
  modulation delay, treated as part of the effect. Flutter carries a deliberately **much smaller weight**
  than wow (equal weight would make 12 Hz swing pitch ~20× harder than 0.556 Hz → seasick). Scope was
  **wow/flutter only** — vinyl/tape *noise* was dropped. Weights/frequencies/span are **tuned by ear**.
  - **Fix during M5b:** `SetParameters` initially set only `m_Warmth` and never `m_Age`, so `m_Age`
    stayed pinned at 0 and Age was inaudible across its whole range; adding `m_Age.setTargetValue(...)`
    fixed it. The raw per-sample random was also replaced by the band-limited-drift path above (raw
    white noise gargled).
  - **Editor:** an **Age** rotary knob, always visible (like Warmth), added right after Warmth in
    `GetAllComponents()`; caption "Age".
- **Pedal UI (M6a) — first pure-GUI milestone, no DSP change.** Three new/rewritten view pieces, all on
  the message thread; `PluginProcessor`, `Parameters.h`, the `dsp/` tree and `CMakeLists.txt` are
  untouched (sources are globbed, so the three new files compile with no CMake edit). No new parameter —
  every knob still binds to an existing APVTS id.
  - **`CCSLookAndFeel` (custom `juce::LookAndFeel_V4`):** a stateless flat-vector skin drawn entirely in
    code — **no image assets**. Overrides `drawRotarySlider` (track arc + amber value arc + flat knob cap
    + pointer line), `drawComboBox`, `drawPopupMenuBackground`, and `drawToggleButton` (an amber **LED**,
    lit when on, with a glow halo + caption; reserves the same value-box band as a knob so its LED
    centres on the knob dials). Label/text-box/combo/popup **colours** are set in the constructor via
    `setColour(...)` rather than by overriding `drawLabel`. A shared **`Palette`** namespace
    (`namespace CozyChorus::Palette` in the header) holds the cozy amber-on-brown colours
    (`Background/Plate/Screen/TitleText/CaptionText/KnobBody/Track/Accent/Screw`). **Cascade:** the editor
    calls `setLookAndFeel(&m_LookAndFeel)` **once** — JUCE resolves each control's L&F by walking up the
    parent chain, so the one call skins every child, including the sliders **inside** each `LabeledKnob`.
  - **Lifetime rule (the L&F footgun):** a `LookAndFeel` must outlive every component drawing with it.
    `m_LookAndFeel` is declared **first** in the editor (destructs last) and the destructor calls
    `setLookAndFeel(nullptr)` before members tear down. Both are required; omitting either dangles a
    pointer during destruction.
  - **`LabeledKnob` composite (`juce::Component`):** a caption `juce::Label` stacked over a
    `RotaryHorizontalVerticalDrag` `juce::Slider` (with a `TextBoxBelow` value read-out). Owns both as
    value members (`addAndMakeVisible`, no ownership transfer); `resized()` slices the caption off the
    top and gives the rest to the slider. `getSlider()` exposes the inner slider so APVTS
    `SliderAttachment`s bind to it. This **retires** the old paint-time `captionFor` map + caption-band
    loop — captions are now real child components. Every knob (Mix, Warmth, Age, and all per-effect
    controls) is a `std::unique_ptr<LabeledKnob>`; the Vibrato control stays a bare `juce::ToggleButton`.
  - **Zone layout (replaces the wrapping grid):** a fixed **560×440** faceplate sliced once per
    `resized()` with `removeFromTop/Left/Right/Bottom` into: **header** (brand plate drawn in `paint()` +
    effect-selector `ComboBox` on the right), **Mix** (`m_MixZoneWidth` on the left, knob centred via
    `withSizeKeepingCentre(kKnobWidth, kKnobHeight)`), a framed **CHARACTER** box (`kCharacterZoneWidth`
    on the right; interior halved for Warmth + Age), a reserved recessed **"screen"** (the centre
    remainder, stored as `m_ScreenZone` and painted as a placeholder — the future M6b visualiser bounds),
    and a **bottom row** holding only the **active effect's** controls, **centred** (total width computed,
    started at `bottomRow.getCentreX() - total/2`). Metrics live in **`EditorConstants.h`** and are read
    by **both** `resized()` and `paint()` so the drawn frames never drift from the control positions.
  - **Per-effect visibility unchanged in spirit:** the 30 Hz `Timer` still tracks `effectType`;
    `RenderComponents()` hides all effect controls then shows/positions only the active set
    (`GetActiveComponents()` returns the right `LabeledKnob*` list per `EffectType`).
  - **Mix greyed in Vibrato:** the timer also reads `vibeMode`; when the Vibe is active **and** Vibrato is
    on, `m_MixKnob->getSlider().setEnabled(false)` renders Mix via the disabled path (it is ignored in the
    DSP there) without removing it from the layout.
  - **Deferred to M6b (now delivered):** the animated curve inside the screen rect. 6a painted that
    rectangle as a static recessed placeholder; M6b replaced it with the `ModulationVisualiser`, which
    draws its own recess.
  - **Open cleanups (not blockers, still open after M6b):** `EditorConstants.h` still carries the pre-M6a
    grid constants (`kMaxColumns`, `kCellPad*`) and a duplicate `kBackground/kTitleText/kCaptionText`
    colour set superseded by `Palette`; `m_ScreenZone` is still **declared** in the editor header but no
    longer assigned (the visualiser owns those bounds now). `timerCallback()` calls `RenderComponents()`
    every tick (full relayout at 30 Hz) rather than only on change; harmless on the message thread.
    `Palette::Screw` + screw metrics are defined but not yet drawn.

- **Modulation visualiser (M6b) — the centre screen comes alive.** One new view
  (`Source/Editor/ModulationVisualiser.h/.cpp`) plus a small read-only export surface on the DSP side.
  `Parameters.h` is untouched — **no new parameter**; the visualiser is pure view state.
  - **Audio → GUI hand-off (lock-free, one store per block):** `PluginProcessor` keeps two
    `std::atomic<float>`s — `m_VisualPhase` and `m_VisualDelayInSamples` — stored at the **end** of
    `processBlock` with `std::memory_order_relaxed`, and exposes `GetVisualPhase()`,
    `GetVisualDelayInSamples()`, `GetActiveEffectType()` and `GetProcessSpec()`. The `juce::dsp::ProcessSpec`
    was promoted from a local in `prepareToPlay` to the member `m_ProcessSpec` so the GUI can read the live
    sample rate. No locks, no allocation, no new latency; the GUI never touches DSP objects.
  - **Read-only DSP exports:** `ModulationEffect::GetCurrentLFOPhase()` on the **base** covers all four
    effects in one line (`m_LFO` is a base member); `ChorusEffect`/`FlangerEffect` gained
    `GetDelayInSamples()`, backed by a new `m_DelayInSamples` member that replaces what used to be a local
    `delaySample` inside the per-sample loop. `PhaserEffect::MIN_FC_HZ/MAX_FC_HZ`,
    `VibeEffect::NUM_STAGES/MIN_FC_HZ/MAX_FC_HZ/STAGE_OFFSET` and `VibeEffect::GetAsymmetricShape` were
    promoted from private to **public** (the last one to `static`) so the view derives its curves from the
    **same constants and shape function as the DSP** rather than duplicating magic numbers. Vibe's
    `Prepare` now builds `m_StageLogOffset` from the shared `STAGE_OFFSET` table.
  - **Real DSP change (6b is *not* view-only, unlike 6a):** `m_LFO.SetFrequency(m_RateHz.getNextValue())`
    moved **inside** the per-sample loop in Chorus, Flanger, Phaser and Vibe. It had been called once per
    block, so the Rate smoother advanced one step per block and its 20 ms ramp was effectively stretched by
    the block size; it now runs at sample rate like every other smoothed control.
  - **Animation driver: `juce::VBlankAttachment`, not a `Timer`** — the callback fires on the display's
    vsync, so the curve is smooth and repaints can't outpace the screen. Each frame it re-reads the sample
    rate, phase and delay, then `repaint()`s. **No OpenGL** (the M6a note that it "possibly" would be is
    resolved: plain `juce::Graphics` paths are fast enough at this size).
  - **Two modes, click to toggle:** `Mode::LFO` and `Mode::Response` (default **Response**), swapped in
    `mouseDown` with a pointing-hand cursor; a corner caption names the current view ("LFO" / "Signal" /
    "Spectrum"). Mode is **view state only** — not a parameter, not persisted.
  - **LFO mode:** `kVisCyclesShown = 6` cycles across the width, drawn from the published phase so it
    scrolls in lock-step with the DSP (and **freezes when the host stops processing** — a useful tell).
    Amplitude ∝ that effect's Depth param; shape per effect — sine for Chorus/Flanger/Phaser,
    `VibeEffect::GetAsymmetricShape` for the Vibe throb.
  - **Response mode is family-adaptive**, mirroring the two DSP families:
    - **Delay family (Chorus, Flanger)** — a dry sinusoid (`Palette::Trace`) and a wet copy
      (`Palette::Accent`) shifted horizontally by the **live** delay at `kVisPxPerMs = 2 px/ms`, so the
      modulation is read directly as a breathing time offset. No filtering maths involved.
    - **All-pass family (Phaser, Vibe)** — the **analytic** magnitude response of the wet/dry sum on a log
      frequency axis (50 Hz–20 kHz, −40…+20 dB, gridlines at 0/−40 dB and 100 Hz/1 k/10 k). Each first-order
      all-pass contributes phase `−2·atan(f/fc)`; Phaser uses `A = e^{jφ}` with `φ = stages·(−2 atan(f/fc))`
      wrapped by feedback as `A' = A/(1 − fb·A)`, Vibe sums the four staggered stage phases (no feedback);
      then `H = mix·A' + (1 − mix)` and `20·log10|H|`. Cutoffs come from
      `EvaluateCutoffFrequency`, which repeats the DSP's own log-domain sweep
      (`exp(logCenter + logHalfSpan·depth·shape)`, clamped) — **no FFT, no audio buffer is ever sent to the
      GUI**; the notches move because the published phase moves.
  - **Editor wiring:** the visualiser is a `std::unique_ptr` member constructed with the `PluginProcessor&`;
    `RenderComponents()` hands it the centre remainder directly (`setBounds(area)`) and `paint()` no longer
    draws the screen placeholder — the component paints its own recessed screen. `CCSAudioProcessorEditor.h`
    now **forward-declares** `LabeledKnob` and `ModulationVisualiser` and includes their headers only in the
    `.cpp`. New shared metrics `kVisCyclesShown` / `kVisPxPerMs` live in `EditorConstants.h`; the dry trace
    colour `Palette::Trace` was added to `CCSLookAndFeel.h`.
  - **Known gap (cosmetic):** the spectrum plot blends with the `mix` param even for the Vibe, where
    **Vibrato mode forces 100 % wet** in the DSP — so with Vibrato on the drawn curve is flatter than what
    is actually heard. `Palette::Trace` also currently duplicates `Palette::Screw`'s value.

---

## Plugin identifiers (`juce_add_plugin`)

| Setting | Value |
|---|---|
| CMake target | `CozyChorusSuite` |
| `PRODUCT_NAME` | `CozyChorus Suite` |
| `COMPANY_NAME` | `Seank23` |
| Bundle prefix | `com.seank23` |
| `PLUGIN_MANUFACTURER_CODE` | `Sk23` |
| `PLUGIN_CODE` | `Cczs` |
| Formats | VST3, Standalone (+ AU on macOS) |
| Category | Fx / Modulation |

---

## DSP reference (per effect)

**Shared LFO:** sine/triangle, rate range **0.05–5 Hz**, per-channel phase offset for stereo
width, continuous phase across blocks. `GetPhase()` exposes the running phase; `ModulationEffect` re-exports
it as `GetCurrentLFOPhase()` for the M6b visualiser. **Since M6b, `SetFrequency(m_RateHz.getNextValue())` is
called per sample** in all four effects (it was once per block, which stretched the Rate smoother's 20 ms
ramp by the block size).

### Chorus (Milestone 1)
- `input --> modulated fractional delay line --> wet`; `output = dry*(1-mix) + wet*mix`.
- `juce::dsp::DelayLine<float, Lagrange3rd>` (or Thiran), max delay ~50 ms.
- Base delay ~15–25 ms; LFO modulates ±depth (up to ~±7 ms at full depth).
- Stereo: offset L/R LFO phase (~90°); Width scales the offset.
- **No feedback** (feedback is what makes it a flanger).
- Optional after single-voice works: 2–3 parallel taps (ensemble).

| Param | Range | Notes |
|---|---|---|
| Rate | 0.05–5 Hz | LFO speed |
| Depth | 0–100% | delay-time modulation amount |
| Mix | 0–100% | dry/wet |
| Voices | 1–3 | start at 1 |
| Stereo Width | 0–100% | L/R LFO phase offset |

Smooth Rate, Depth, Mix, Width with `SmoothedValue`.

**Shipped M1 (+ Session 5):** 1–3-voice ensemble wired and exposed — one shared LFO with a per-voice
phase offset + base-delay spread (±4 ms around the 20 ms centre), N summed delay taps normalised by
`1/voices`. LFO is sine (triangle/saw/square exist in `LFO` but aren't user-selectable yet). Base
delay 20 ms, ±7 ms modulation, params smoothed over 20 ms.

### Flanger (Milestone 2)
- `input --> modulated fractional delay line (with feedback) --> wet`; `output = dry*(1-mix) + wet*mix`.
- Same `juce::dsp::DelayLine<float, Lagrange3rd>`, max delay ~15 ms.
- Base delay 0.5–5 ms; the LFO sweeps the delay **upward** from base by up to +5 ms at full depth.
- **Feedback** (−0.95…+0.95) closes the comb → the resonant "jet" sweep (the thing that makes it a
  flanger and not a chorus). pop-before-push feedback loop.
- Stereo: reuses the Chorus per-channel LFO phase offset; Width scales it.

| Param | Range | Notes |
|---|---|---|
| Rate | 0.05–5 Hz | LFO speed (per-effect) |
| Depth | 0–100% | delay-sweep amount, up to +5 ms (per-effect) |
| Mix | 0–100% | dry/wet (shared) |
| Feedback | −95…95% | comb feedback, mapped to ±0.95 coefficient (skew 0.4, default 45%) |
| Base Delay | 0.2–5 ms | shortest delay / sweep floor (default 1 ms) |
| Stereo Width | 0–100% | L/R LFO phase offset (per-effect) |

**Shipped M2 (+ tuning):** functionally correct and RT-safe — verified by offline impulse-response
measurement (feedback lifts the resonant peak 0 → +14.6 dB at fb 0.9; base delay moves the comb by
1/D). Two of the three M2 tuning caveats were then addressed in the shipped params: feedback now
**defaults to 45%** (audible resonance out of the box) with a **skewed taper** (`skew 0.4`, so the
knob's travel is no longer bunched into the top quarter), and the **base delay default dropped to
1 ms** (range widened to 0.2–5 ms). Remaining caveat, not a bug: the sweep is still **upward-only from
base**, so the comb's top-end reach is limited, and its own `flangerDepth` param reuses Chorus's
depth-scaling behaviour rather than defining a bespoke range. See DEVLOG Session 6.

Through-zero flanging is an optional later refinement.

### Phaser (Milestone 3 — all-pass family)
- `input --> feedback --> N-stage all-pass cascade --> wet`; `output = dry*(1-mix) + wet*mix`.
- Cascade of N **first-order all-pass filters**, hand-rolled in **TPT** form (`g = tan(π·fc/fs)`,
  `G = g/(1+g)`; each stage outputs `2·lowpass − input`, one state var per stage).
- LFO modulates the all-pass cutoff `fc` in the **log domain**, swept over **200 Hz–2 kHz**; Depth
  scales the sweep, Rate sets LFO speed.
- **Feedback** (−0.95…+0.95) wraps the whole cascade → resonant peaks between the notches.
- Stereo: reuses the per-channel LFO phase offset; Width scales it.
- **No delay buffer** — the first effect that allocates none.

| Param | Range | Notes |
|---|---|---|
| Rate | 0.05–5 Hz | LFO speed (per-effect) |
| Depth | 0–100% | cutoff-sweep amount (per-effect) |
| Mix | 0–100% | dry/wet (shared) |
| Stages | 2–12 | number of all-pass stages (default 6) |
| Feedback | −95…95% | cascade feedback, mapped to ±0.95 coefficient (default 0) |
| Stereo Width | 0–100% | L/R LFO phase offset (per-effect) |

**Shipped M3:** functionally correct and RT-safe — all state (`m_AllPassState`, `m_FeedbackState`)
allocated in `Prepare`, params smoothed over 20 ms, LFO phase advanced **once per sample** (a
block-rate `Advance()` bug that froze the sweep was caught and fixed before sign-off). `Stages` is any
int 2–12 (not restricted to even). Sweep audibly present across the range; feedback lifts the resonant
peaks. Tuning by ear against reference phaser material still open for a later polish pass.

### Vibe (Milestone 4 — all-pass family, Uni-Vibe)
- `input --> 4 staggered all-pass stages --> wet`; `output = dry*(1-mix) + wet*mix` (Chorus mode) or
  `output = wet` (Vibrato mode, forced 100 % wet). **No feedback, no delay buffer.**
- Cascade of **exactly 4** first-order TPT all-pass stages (same kernel as the Phaser). Each stage sits
  at its **own** break frequency: one LFO-swept centre + a fixed per-stage **log offset**
  (`{−0.75, −0.25, +0.25, +0.75}` octaves), so `g`/`G` are recomputed per stage.
- The LFO modulates the swept centre in the **log domain** over **200 Hz–2 kHz**; Depth scales the
  span, Rate sets speed — but the LFO waveform is **asymmetric** (owned by `VibeEffect`, not the shared
  `LFO`): warp-then-sine with `ASYM_K = 0.35`, giving the lamp/photocell *throb*.
- **Chorus / Vibrato mode** (`vibeMode` bool): Chorus = dry+wet via Mix (swirl); Vibrato = 100 % wet
  (the swept group delay reads as pitch wobble). Vibrato overrides the Mix knob to 1.0 in the DSP.
- Stereo: reuses the per-channel LFO phase offset; Width scales it.

| Param | Range | Notes |
|---|---|---|
| Rate | 0.05–5 Hz | LFO speed (per-effect) |
| Depth | 0–100% | sweep span (per-effect) |
| Mix | 0–100% | dry/wet (shared); **ignored in Vibrato mode** (forced 100 % wet) |
| Vibrato | Off/On | mode switch: Off = Chorus (blend), On = Vibrato (100 % wet). `AudioParameterBool`, default Off |
| Stereo Width | 0–100% | L/R LFO phase offset (per-effect) |

**Shipped M4:** functionally correct and RT-safe — all state (`m_AllPassState`) allocated in `Prepare`,
Rate/Depth/Mix/Width smoothed over 20 ms, LFO phase advanced **once per sample**, no allocation in
`Process`. Stages fixed at 4 (no `Stages` param); **no feedback** path at all — the Vibe can't ring, so
`Reset()` only flushes the all-pass state. The asymmetric shape lives in `VibeEffect::GetAsymmetricShape`
(the shared `LFO` gained only a generic `GetPhase()` accessor) — **public + `static` since M6b**, alongside
`NUM_STAGES`, `MIN_FC_HZ`, `MAX_FC_HZ` and the `STAGE_OFFSET` table (and `PhaserEffect::MIN_FC_HZ/MAX_FC_HZ`),
so the visualiser plots the sweep from the DSP's own constants. Editor adds a **"Vibrato" toggle button**
shown only for the Vibe (5 controls: 4 knobs — its Rate/Depth/Width + the shared Mix — plus the toggle). Stagger spread (`±0.75` octave)
and `ASYM_K` are **tuned by ear, not measured** — a by-ear polish pass against reference Uni-Vibe
material is still open.

### Character (Milestones 5 + 5b — global tape stage, not an effect)

- Runs on the output **after** the active effect (post-Mix), not selected by `effectType`. Full signal
  path: `input → wow/flutter fractional delay (M5b) → 2× upsample → asymmetric-tanh saturation →
  downsample → one-pole high-cut → output`.
- **Wow/flutter (M5b, runs first):** a modulated fractional delay (`juce::dsp::DelayLine<Lagrange3rd>`)
  whose delay is `(2 ms + 1.5 ms·mod·Age)`, where
  `mod = 0.8·wow(0.556 Hz) + 0.04·flutter(12 Hz) + 0.02·drift`. Wow/flutter are two `LFO` sines
  (advanced per sample); `drift` is one `juce::Random` value per block, bipolar, one-pole-LPF'd at 2 Hz
  with `1/√(coeff/2)` makeup. One shared `mod`/delay across channels (mono-correlated). Age 0 % ⇒ static
  2 ms delay (silent); 100 % ⇒ full wander. No feedback.
- **Saturation:** `y = (tanh(drive·(x + BIAS)) − tanh(drive·BIAS)) · invS0` at 2× oversampling
  (`juce::dsp::Oversampling`, minimum-phase half-band IIR for low latency). `drive = 1 + Warmth·2.5`;
  `BIAS = 0.15` shifts the operating point off-centre for **even-harmonic** colour; the `− tanh(drive·BIAS)`
  term re-centres DC; `invS0 = 1/(drive·(1 − tanh²(drive·BIAS)))` normalises the **slope at 0** so
  small signals pass at unity gain (built-in makeup — Warmth compresses peaks, it is *not* a volume knob).
- **Tone:** one-pole **high-cut** (`FirstOrderTPTFilter`, lowpass) after downsampling, swept by the same
  macro `fc = 18 kHz·(4/18)^Warmth` → 18 kHz at 0 %, 4 kHz at 100 %.
- **Latency:** the oversampler adds a few samples; reported via `setLatencySamples` (PDC). The M5b
  wow/flutter delay (~2 ms) is **not** PDC-reported (time-varying modulation delay, part of the effect).

| Param | Range | Notes |
|---|---|---|
| Warmth | 0–100% | **global** macro (default 30 %); drives saturation amount **and** high-cut together |
| Age | 0–100% | **global** macro (default 30 %); scales wow/flutter pitch-modulation depth (0 % = off) |

**Shipped M5:** functionally correct and RT-safe — oversampler + filter allocated in `Prepare`,
`Process` allocation-free, Warmth smoothed 20 ms (read once per block). One post-ship fix: the
saturation loop was iterating the pre-oversampling sample/channel counts, leaving half of every 2×
block unsaturated (low-frequency crackle) and skipping the last channel — corrected to iterate the
oversampled block's own extents. Endpoints (`DRIVE_MAX`, `BIAS`, cutoff range) are **tuned by ear**.

**Shipped M5b:** wow/flutter only (vinyl/tape *noise* dropped from scope). RT-safe — delay line, LFOs
and noise-LPF coefficient all set up in `Prepare`, `Process` allocation-free, Age smoothed 20 ms (read
per sample). One fix: `SetParameters` initially never called `m_Age.setTargetValue`, pinning Age at 0
(inaudible across its range); and the raw per-sample random was replaced by the band-limited-drift path
(raw white noise gargled). Weights (`0.8 / 0.04 / 0.02`), frequencies (0.556 / 12 Hz), the 2 Hz drift
LPF, and the 2 ms ± 1.5 ms delay span are **tuned by ear**; a by-ear polish pass is still open.

---

## Definition of done (first handoff)

- Builds as VST3 + Standalone via CMake with JUCE 8 as a submodule.
- Loads in the standalone host and a DAW, passes audio.
- Chorus works with Rate/Depth/Mix/Width (Voices optional), automatable, no RT-thread
  allocation, no zipper noise.
- Clean commit history, one commit per working milestone.
