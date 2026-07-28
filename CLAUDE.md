# CozyChorus Suite

A cross-platform audio plugin (**VST3 + AU + Standalone**): a suite of four classic
guitar-pedal modulation effects — **Chorus, Flanger, Phaser, Vibe** (Uni-Vibe style) —
plus a global **Character** (tape-warmth) stage on the output. The user selects the active
effect; each exposes its own controls. Guitar-oriented, works in mono and stereo.
C++20 / JUCE 8 / CMake.

---

## Current status

- **Phase:** Milestone 5 complete — **all four effects plus the global Character (tape-warmth) stage
  are implemented and audible.** Chorus + Flanger (delay-line family) and Phaser + **Vibe** (all-pass
  family) finished the DSP-effect track at M4; **M5 adds a global `CharacterStage`** (oversampled
  asymmetric-tanh saturation + high-cut tone, one **Warmth** macro) applied to the output after the
  active effect. Only GUI polish (custom `LookAndFeel`, per-effect panels, LFO visualiser) remains — a
  separate, deferred pass, not a milestone.
- **Params are per-effect (post-M4):** Rate / Depth / Stereo Width were split from shared params into
  **per-effect** APVTS params (`chorusRate`, `flangerRate`, …), so each effect carries its own defaults.
  **`mix` is the only shared *effect* param**; **`warmth`** is a separate **global stage** param (the
  Character stage, not an effect control).
- **Character stage reports latency (PDC):** the 2× oversampler adds a few samples of latency, reported
  once via `setLatencySamples(m_CharacterStage.GetLatencySamples())` in `prepareToPlay` — the suite's
  first use of host delay compensation.
- A hand-written **`CCSAudioProcessorEditor`** (rotary knobs + effect selector, per-effect control
  visibility) has replaced the generic editor. Custom `LookAndFeel` / LFO visualiser still deferred.
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
   (first PDC use). Wow/flutter + vinyl noise deferred to a later 5b.
7. **GUI — in progress (functional).** A parameter-driven `CCSAudioProcessorEditor` now ships
   (rotary knobs, effect selector, per-effect control visibility). Still deferred: custom
   `LookAndFeel`, polished per-effect panels, and an LFO visualiser (possibly OpenGL).

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
  PluginProcessor.h / .cpp   // AudioProcessor: owns APVTS + effect instances + CharacterStage; routes processBlock to active effect, then runs the Character stage on the output; reports oversampler latency via setLatencySamples
  Parameters.h               // parameter IDs + APVTS layout in one place
  Editor/
    CCSAudioProcessorEditor.h / .cpp  // custom editor: effect selector + rotary knobs, per-effect control visibility (30 Hz Timer), wrapping-grid layout; createEditor() returns this
  dsp/
    ModulationEffect.h       // abstract base: Prepare(spec) / Process(context) / Reset()
    NullEffect.h             // pass-through; now ONLY the `default` guard in GetActiveEffect() — every built effect routes to itself
    LFO.h / .cpp             // shared LFO: continuous phase, per-channel phase-offset reads, Hz rate; sine/triangle/saw/square (Chorus/Flanger/Phaser use sine); GetPhase() accessor lets Vibe apply its own asymmetric shape
    ChorusEffect.h / .cpp    // Chorus (delay-line family) — Milestone 1, done
    FlangerEffect.h / .cpp   // Flanger (delay-line family, feedback comb) — Milestone 2, done
    PhaserEffect.h / .cpp    // Phaser (all-pass family, TPT all-pass cascade + feedback) — Milestone 3, done
    VibeEffect.h / .cpp      // Vibe (all-pass family, 4 staggered TPT stages + asymmetric LFO + Chorus/Vibrato mode, no feedback) — Milestone 4, done
    CharacterStage.h / .cpp  // Character (global tape-warmth stage, NOT a ModulationEffect): 2x oversampled asymmetric-tanh saturation + one-pole high-cut, single Warmth macro — Milestone 5, done
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
  (`DRIVE_MAX = 4`), `DC_BIAS = 0.15` giving even harmonics, and `invS0 = 1/(drive·(1 − tanh²(drive·DC_BIAS)))`
  a **slope-normalised makeup** (unity small-signal gain, so Warmth compresses peaks rather than acting
  as a volume knob) — followed by a one-pole **high-cut** (`FirstOrderTPTFilter`, lowpass) swept
  `fc = MAX_WARMTH_CUTOFF_HZ·(MIN/MAX)^w` from **18 kHz → 6.5 kHz**. One **Warmth** macro (0–1) drives
  both drive and cutoff; `warmth` is smoothed 20 ms but read **once per block** (coefficients constant
  per block). All state (oversampler, filter) allocated in `Prepare`; `Process` is allocation-free
  (`processSamplesUp`/`Down` reuse the oversampler's internal buffers). **Latency:** the oversampler's
  IIR adds a few samples, reported once via `setLatencySamples(m_CharacterStage.GetLatencySamples())`
  in `prepareToPlay` (the suite's first PDC). **`warmth` is a global stage param** (default 30 %), not
  an effect control and independent of `effectType`. Wow/flutter + vinyl noise are deferred to a later
  **5b**. Drive/bias/cutoff endpoints are **tuned by ear**.
  - **Editor:** a **Warmth** rotary knob, always visible (like Mix, independent of `effectType`), added
    right after the Mix knob in `GetAllComponents()`; caption "Warmth".
  - **Shipped-M5 fix:** the saturation loop originally iterated the *pre*-oversampling sample/channel
    counts (`< numSamples - 1`, `< numChannels - 1`), leaving the back half of every 2× block
    unsaturated → a per-block discontinuity heard as low-frequency crackle, and skipping the last
    channel. Fixed to iterate the oversampled block's own `getNumSamples()`/`getNumChannels()`.

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
width, continuous phase across blocks.

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
(the shared `LFO` gained only a generic `GetPhase()` accessor). Editor adds a **"Vibrato" toggle button**
shown only for the Vibe (5 controls: 4 knobs — its Rate/Depth/Width + the shared Mix — plus the toggle). Stagger spread (`±0.75` octave)
and `ASYM_K` are **tuned by ear, not measured** — a by-ear polish pass against reference Uni-Vibe
material is still open.

### Character (Milestone 5 — global tape-warmth stage, not an effect)

- Runs on the output **after** the active effect (post-Mix), not selected by `effectType`. Signal path:
  `input → 2× upsample → asymmetric-tanh saturation → downsample → one-pole high-cut → output`.
- **Saturation:** `y = (tanh(drive·(x + BIAS)) − tanh(drive·BIAS)) · invS0` at 2× oversampling
  (`juce::dsp::Oversampling`, minimum-phase half-band IIR for low latency). `drive = 1 + Warmth·4`;
  `BIAS = 0.15` shifts the operating point off-centre for **even-harmonic** colour; the `− tanh(drive·BIAS)`
  term re-centres DC; `invS0 = 1/(drive·(1 − tanh²(drive·BIAS)))` normalises the **slope at 0** so
  small signals pass at unity gain (built-in makeup — Warmth compresses peaks, it is *not* a volume knob).
- **Tone:** one-pole **high-cut** (`FirstOrderTPTFilter`, lowpass) after downsampling, swept by the same
  macro `fc = 18 kHz·(6.5/18)^Warmth` → 18 kHz at 0 %, 6.5 kHz at 100 %.
- **Latency:** the oversampler adds a few samples; reported via `setLatencySamples` (PDC).

| Param | Range | Notes |
|---|---|---|
| Warmth | 0–100% | **global** macro (default 30 %); drives saturation amount **and** high-cut together |

**Shipped M5:** functionally correct and RT-safe — oversampler + filter allocated in `Prepare`,
`Process` allocation-free, Warmth smoothed 20 ms (read once per block). One post-ship fix: the
saturation loop was iterating the pre-oversampling sample/channel counts, leaving half of every 2×
block unsaturated (low-frequency crackle) and skipping the last channel — corrected to iterate the
oversampled block's own extents. Endpoints (`DRIVE_MAX`, `BIAS`, cutoff range) are **tuned by ear**.
Wow/flutter + vinyl noise deferred to a later **5b**.

---

## Definition of done (first handoff)

- Builds as VST3 + Standalone via CMake with JUCE 8 as a submodule.
- Loads in the standalone host and a DAW, passes audio.
- Chorus works with Rate/Depth/Mix/Width (Voices optional), automatable, no RT-thread
  allocation, no zipper noise.
- Clean commit history, one commit per working milestone.
