# CozyChorus Suite

A cross-platform audio plugin (**VST3 + AU + Standalone**): a suite of four classic
guitar-pedal modulation effects — **Chorus, Flanger, Phaser, Vibe** (Uni-Vibe style).
The user selects the active effect; each exposes its own controls. Guitar-oriented,
works in mono and stereo. C++20 / JUCE 8 / CMake.

---

## Current status

- **Phase:** Milestone 3 complete — Chorus + Flanger (delay-line family) **and Phaser** (all-pass
  family) are implemented and audible. Milestone 4 (Vibe, all-pass family) is next — the last effect.
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
   cascade for resonance. Rate/Depth/Mix/Width are the shared controls; **Stages + Feedback** are
   Phaser-only. All params smoothed. No delay buffer.
5. **Milestone 4 — Vibe.** Phaser variant, staggered stages + asymmetric LFO (hardest, last).
6. **GUI — in progress (functional).** A parameter-driven `CCSAudioProcessorEditor` now ships
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
  PluginProcessor.h / .cpp   // AudioProcessor: owns APVTS + effect instances; routes processBlock to active effect
  Parameters.h               // parameter IDs + APVTS layout in one place
  Editor/
    CCSAudioProcessorEditor.h / .cpp  // custom editor: effect selector + rotary knobs, per-effect control visibility (30 Hz Timer), wrapping-grid layout; createEditor() returns this
  dsp/
    ModulationEffect.h       // abstract base: Prepare(spec) / Process(context) / Reset()
    NullEffect.h             // pass-through; still the fallback for the one effect not yet built (Vibe)
    LFO.h / .cpp             // shared LFO: continuous phase, per-channel phase-offset reads, Hz rate; sine/triangle/saw/square implemented (Chorus + Flanger + Phaser use sine)
    ChorusEffect.h / .cpp    // Chorus (delay-line family) — Milestone 1, done
    FlangerEffect.h / .cpp   // Flanger (delay-line family, feedback comb) — Milestone 2, done
    PhaserEffect.h / .cpp    // Phaser (all-pass family, TPT all-pass cascade + feedback) — Milestone 3, done
    // VibeEffect added in Milestone 4
```

### Design principle: two DSP families, one shared skeleton

- **Delay-line family** — Chorus, Flanger: a modulated fractional delay line
  (flanger = chorus + feedback + shorter base delay).
- **All-pass family** — Phaser, Vibe: a cascade of first-order all-pass filters
  (vibe = phaser with staggered stages + asymmetric LFO).

All four are driven by a shared **LFO** and share **Rate / Depth / Mix / Stereo** controls.

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
  Rate/Depth/Mix/Width are the **same APVTS params** as Chorus (shared controls); Feedback + Base
  Delay are Flanger-only.
- **Editor (M2, extended M3):** replaced `GenericAudioProcessorEditor` with a hand-written
  `CCSAudioProcessorEditor` (`Source/Editor/`). One shared set of rotary sliders + an effect
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
  (±0.95). Stereo width reuses the Chorus/Flanger per-channel LFO phase-offset trick. Rate/Depth/Mix/
  Width are the shared APVTS params; **Stages** (2–12, default 6) and **Feedback** (default 0) are
  Phaser-only. No delay line — this is the first effect that allocates no delay buffer.

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
| Rate | 0.05–5 Hz | LFO speed (shared with Chorus) |
| Depth | 0–100% | delay-sweep amount, up to +5 ms (shared) |
| Mix | 0–100% | dry/wet (shared) |
| Feedback | −95…95% | comb feedback, mapped to ±0.95 coefficient (skew 0.4, default 45%) |
| Base Delay | 0.2–5 ms | shortest delay / sweep floor (default 1 ms) |
| Stereo Width | 0–100% | L/R LFO phase offset (shared) |

**Shipped M2 (+ tuning):** functionally correct and RT-safe — verified by offline impulse-response
measurement (feedback lifts the resonant peak 0 → +14.6 dB at fb 0.9; base delay moves the comb by
1/D). Two of the three M2 tuning caveats were then addressed in the shipped params: feedback now
**defaults to 45%** (audible resonance out of the box) with a **skewed taper** (`skew 0.4`, so the
knob's travel is no longer bunched into the top quarter), and the **base delay default dropped to
1 ms** (range widened to 0.2–5 ms). Remaining caveat, not a bug: the sweep is still **upward-only from
base**, so the comb's top-end reach is limited and there's no separate Flanger Depth scaling (it shares
Chorus's). See DEVLOG Session 6.

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
| Rate | 0.05–5 Hz | LFO speed (shared) |
| Depth | 0–100% | cutoff-sweep amount (shared) |
| Mix | 0–100% | dry/wet (shared) |
| Stages | 2–12 | number of all-pass stages (default 6) |
| Feedback | −95…95% | cascade feedback, mapped to ±0.95 coefficient (default 0) |
| Stereo Width | 0–100% | L/R LFO phase offset (shared) |

**Shipped M3:** functionally correct and RT-safe — all state (`m_AllPassState`, `m_FeedbackState`)
allocated in `Prepare`, params smoothed over 20 ms, LFO phase advanced **once per sample** (a
block-rate `Advance()` bug that froze the sweep was caught and fixed before sign-off). `Stages` is any
int 2–12 (not restricted to even). Sweep audibly present across the range; feedback lifts the resonant
peaks. Tuning by ear against reference phaser material still open for a later polish pass.

### Vibe (last — all-pass family, hardest)
Phaser variant: **4 all-pass stages with staggered coefficients**, an **asymmetric LFO**
(photocell/lamp emulation), and a **Chorus / Vibrato mode** switch (vibrato = 100% wet).
Tune by ear against reference Uni-Vibe material.

---

## Definition of done (first handoff)

- Builds as VST3 + Standalone via CMake with JUCE 8 as a submodule.
- Loads in the standalone host and a DAW, passes audio.
- Chorus works with Rate/Depth/Mix/Width (Voices optional), automatable, no RT-thread
  allocation, no zipper noise.
- Clean commit history, one commit per working milestone.
