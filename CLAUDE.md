# CozyChorus Suite

A cross-platform audio plugin (**VST3 + AU + Standalone**): a suite of four classic
guitar-pedal modulation effects — **Chorus, Flanger, Phaser, Vibe** (Uni-Vibe style).
The user selects the active effect; each exposes its own controls. Guitar-oriented,
works in mono and stereo. C++20 / JUCE 8 / CMake.

---

## Current status

- **Phase:** Milestone 1 complete — Chorus is implemented and audible (delay-line family).
  Milestone 2 (Flanger) is next.
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
   `ChorusEffect` (fractional `DelayLine`, Rate/Depth/Mix/Width, per-channel LFO phase offset
   for stereo width, all params smoothed). `Voices` control and LFO shape exist in code but are
   not yet wired/exposed.
3. **Milestone 2 — Flanger.** Reuses the chorus delay line + feedback + shorter base delay.
4. **Milestone 3 — Phaser.** All-pass cascade (new core).
5. **Milestone 4 — Vibe.** Phaser variant, staggered stages + asymmetric LFO (hardest, last).
6. **GUI — deferred.** Custom `LookAndFeel`, per-effect panels, LFO visualiser (possibly OpenGL),
   only after the DSP is solid.

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
  dsp/
    ModulationEffect.h       // abstract base: Prepare(spec) / Process(context) / Reset()
    NullEffect.h             // pass-through; still the fallback for effects not yet built (Flanger/Phaser/Vibe)
    LFO.h / .cpp             // shared LFO: continuous phase, per-channel phase-offset reads, Hz rate; sine/triangle/saw/square implemented (Chorus uses sine)
    ChorusEffect.h / .cpp    // Chorus (delay-line family) — Milestone 1, done
    // FlangerEffect, PhaserEffect, VibeEffect added in later milestones
  // PluginEditor.h/.cpp — deferred to the GUI phase; createEditor() returns GenericAudioProcessorEditor
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
  choice parameter selects which effect `processBlock` dispatches to (all → `NullEffect` in M0).
- Start with **`GenericAudioProcessorEditor`**; custom GUI is a later phase.

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

**Shipped M1:** single voice (`Voices` control present but the ensemble is not yet wired); LFO is
sine (triangle/saw/square exist in `LFO` but aren't user-selectable yet). Base delay 20 ms, ±7 ms
modulation, params smoothed over 20 ms.

### Flanger (later — delay family)
Reuse chorus delay line. Base delay **0.5–5 ms**; add **Feedback** (−0.95…+0.95) and
**Manual** (base-delay) control. Through-zero flanging is an optional later refinement.

### Phaser (later — all-pass family)
Cascade of N **first-order all-pass filters** (`FirstOrderTPTFilter` in all-pass mode or
hand-rolled one-pole all-pass). LFO modulates all-pass cutoff (~200 Hz–2 kHz). Controls:
**Stages** (2–12, even), **Feedback**, **Rate**, **Depth**, **Mix**. No delay buffer.

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
