# CozyChorus Suite — Development Log

Append-only session log; **newest entry at the top**. This is the portable "pick up where we
left off" record — it's committed to git, so it travels to any PC that clones the repo.

Companion files: **`CLAUDE.md`** = current project state · **`DEVLOG.md`** (this file) =
chronological narrative · **`git log`** = granular changes. Local memory files and Claude Code
conversation transcripts do **not** travel with the repo, so anything worth resuming elsewhere
belongs here.

**Entry template:**

```markdown
## YYYY-MM-DD — Session N: <short title>
**Done:** …
**Decisions:** …
**Next up:** …
**Open questions / blockers:** …
```

---

## 2026-07-17 — Session 6: Milestone 2 — Flanger (+ custom editor)

**Done:**
- Implemented the Flanger (`Source/dsp/FlangerEffect.{h,cpp}`): the delay-line skeleton reused as a
  **feedback comb** — per sample `popSample` (modulated delay) **then**
  `pushSample(input + feedback·wet)` (read-before-write, so min delay = 1 sample). Base delay
  0.5–5 ms; LFO sweeps upward from base by up to +5 ms (unipolar `0.5 + 0.5·sin`); feedback ±0.95;
  stereo width via the Chorus per-channel phase-offset trick. Rate/Depth/Mix/Width/Feedback/BaseDelay
  all `SmoothedValue`.
- Added `flangerFeedback` (−95…95 %, default 0) and `flangerBaseDelay` (0.5–5 ms, default 2) to the
  APVTS (`Parameters.h`); wired `EffectType::Flanger → m_FlangerEffect` in `PluginProcessor` (caches
  the two atomic pointers, builds `FlangerParameters` per block, feedback → ±0.95).
- Replaced `GenericAudioProcessorEditor` with a hand-written `CCSAudioProcessorEditor`
  (`Source/Editor/`): effect selector + rotary knobs; a 30 Hz `Timer` shows/hides per-effect controls
  (Voices for Chorus; Feedback + Base Delay for Flanger). Implemented `resized()` (wrapping grid over
  the *visible* controls) and `paint()` (background + title + a caption above each visible knob).
  Compiles clean on MSVC v145.
- **Verified the Flanger by measurement**, not just by ear: an offline C# reimplementation of the
  exact loop + impulse-response DFT showed the DSP is correct — feedback raises the resonant peak
  0 → +3.5 → +14.6 dB (fb 0 / 0.5 / 0.9), and base delay moves the comb 2000 / 500 / 200 Hz at
  0.5 / 2 / 5 ms (the 1/D law).

**Decisions:**
- Flanger reuses Chorus's Rate/Depth/Mix/Width APVTS params (shared controls); only Feedback +
  Base Delay are Flanger-specific. Phaser/Vibe still fall through to `NullEffect`.
- GUI is no longer fully deferred: a functional, parameter-driven editor ships now. Custom
  `LookAndFeel` / per-effect panels / LFO visualiser remain deferred.
- Accepted the current Flanger tuning as "M2 complete" despite the caveats below — they are
  parameter-curve / default choices, **not** correctness bugs.

**Next up:**
- Optional Flanger polish (skew the feedback taper, lower base-delay default to ~1 ms, decouple Depth
  from Chorus) — user's call whether to fold into M2 or a later pass.
- Milestone 3 (Phaser): new all-pass-cascade core (all-pass family), no delay buffer.

**Open questions / blockers:**
- Flanger **tuning caveats** (verified, not bugs): feedback default 0 → chorus-like out of the box;
  linear feedback taper → knob feels dead until ~75 % travel; upward-only sweep from a 2 ms base → no
  bright top-end. Decide whether to address before M3.
- `CLAUDE.md`'s M1 "Voices not wired" note was stale (Session 5 wired it) — corrected this session.
- Still no `pluginval` / automated DSP test in-repo; the C# harness was throwaway. Consider a Catch2
  DSP test as the suite grows.

---

## 2026-07-16 — Session 5: Chorus voices ensemble + stereo-width fix

**Done:**
- Wired the `Voices` (1–3) control end-to-end (user implemented, guided review): the processor now
  caches the `voices` APVTS pointer and fills `ChorusParameters::Voices`; `ChorusEffect` smooths it
  and reads the delay line as **N summed taps** — one `pushSample`, then N `popSample`s per
  sample/channel with `updateReadPointer = true` on **only the last tap** so read/write pointers
  stay in lockstep — normalised by `1/voices`.
- Made the ensemble actually audible: each voice gets its **own base delay**, spread ±4 ms around
  the 20 ms centre (`m_BaseDelayMs + (v − (voices−1)/2)·4 ms`), plus even LFO phase spread
  (`v/voices`). Previously all voices shared one base delay + rate, so extra voices were nearly
  inaudible.
- **Fixed the stereo-width bug:** the width phase offset was being added to *both* channels
  identically, so L/R stayed correlated and the Width knob did nothing. Now applied per-channel
  (right channel only, `+width*0.25` cycle) on top of the per-voice phase, restoring the widening.
- Cleanups: removed a duplicated/shadowed `voices` read; restored push-then-pop ordering to match
  the original single-tap path.

**Decisions:**
- Ensemble = single shared LFO with per-voice **phase offset + base-delay spread**, same rate.
  True per-voice detune (independent rates) would need one `LFO` instance per voice — deferred.
- `Voices` is smoothed as a float then truncated to int; an abrupt click when the voice count
  changes mid-audio is accepted for now (per-voice gain crossfade deferred).
- Confirmed `Voices` is now part of M1 rather than a later polish pass — supersedes Session 4's
  "shipped single-voice, Voices left unwired" note.

**Next up:**
- Audition the updated Chorus (voices 1→3; Width sweep in **stereo / on headphones** — width is a
  decorrelation effect and won't show in a mono sum), then commit M1.
- Milestone 2 (Flanger): reuse the delay line with feedback + a shorter 0.5–5 ms base delay.

**Open questions / blockers:**
- `CLAUDE.md` still describes M1 as single-voice with `Voices` unwired — now stale, refresh before
  the M1 commit.
- Selectable LFO shape still unwired; changing voice count mid-audio can click (no crossfade yet).
- No `pluginval` / automated DSP test; verification is still a manual audition on this box.

---

## 2026-07-16 — Session 4: Milestone 1 — Chorus

**Done:**
- Implemented the first real effect. New `Source/dsp/LFO.{h,cpp}` (continuous-phase oscillator:
  sine/triangle/saw/square, Hz rate, per-channel phase-offset reads) and
  `Source/dsp/ChorusEffect.{h,cpp}` (fractional `DelayLine<float, Lagrange3rd>`, 20 ms base
  delay, ±7 ms LFO modulation, per-channel width offset; Rate/Depth/Mix/Width all `SmoothedValue`).
- Wired it into `PluginProcessor`: caches the Rate/Depth/Mix/Width APVTS atomic pointers, builds a
  `ChorusParameters` POD per block (percentages → 0–1) via `SetParameters`, dispatches
  `EffectType::Chorus` → `m_ChorusEffect`. Added `rate`/`depth`/`width`/`voices` to the APVTS
  layout (`Parameters.h`).
- Switched `CMakeLists.txt` source collection to `file(GLOB_RECURSE … CONFIGURE_DEPENDS)` over
  `Source/`, so new files auto-include — no more hand-maintained `COZY_SOURCES` list.
- Debugged the "no output when Chorus is selected" report (user's implementation, user fixed):
  the wet/dry sample was read into an `int`, truncating all |x|<1 audio to 0 — total silence.
  Same pass moved to a true fractional delay (the modulated delay was cast to `int`, wasting the
  Lagrange interpolation) and fixed the `Voices` parameter's mislabelled name.

**Decisions:**
- Parameter passing (settled): the processor owns the APVTS atomic pointers and builds a per-effect
  POD each block; the effect smooths internally. Effects never touch the APVTS directly.
- Stereo width = LFO phase offset: right channel read at `+width*0.25` cycle (≤90°); 0 % ⇒ mono.
- Chorus ships as a single bipolar-sine voice; `Voices` (1–3 ensemble) and selectable LFO shape
  are present in code but intentionally left unwired for now.

**Next up:**
- Audition Chorus in the standalone host + a DAW (listen for zipper noise / RT-safety), then commit
  M1 as one commit — it folds in Session 3's CMake solution-tidy and this session's glob change.
- Milestone 2 (Flanger): reuse the delay line with feedback + a shorter 0.5–5 ms base delay.

**Open questions / blockers:**
- Ensemble (`Voices` > 1) and selectable LFO shape deferred — confirm they belong to a later
  Chorus polish pass rather than M1.
- No `pluginval` / automated DSP test yet; verification is still a manual audition on this box.

---

## 2026-07-15 — Session 3: Fresh-PC setup + Visual Studio solution tidy-up

**Done:**
- Brought the repo up on this box: populated the JUCE submodule (JUCE/ @ 8.0.14) and
  reconfigured `build/` with the `Visual Studio 18 2026` generator (CMake 4.4 emits a
  `.slnx`, the new XML solution format — not a `.sln`).
- Organised the generated Visual Studio solution entirely from `CMakeLists.txt` (hand-edits
  to the `.slnx` are clobbered on every configure / ZERO_CHECK run):
  - `USE_FOLDERS ON` + `PREDEFINED_TARGETS_FOLDER "Build (auto-generated)"` — CMake's own
    ALL_BUILD / ZERO_CHECK / INSTALL now collapse into one bucket.
  - Format wrappers → `Formats/`; the `_All` / `_rc_lib` / `_vst3_helper` plumbing →
    `Build (auto-generated)/`. The editable **`CozyChorusSuite`** code target is hoisted to
    the solution root (`FOLDER ""`) so it is the most prominent project.
  - `source_group(TREE Source ...)` mirrors the on-disk `Source/` tree in its own filter.
  - `set(JUCE_ENABLE_MODULE_SOURCE_GROUPS ON …)` — JUCE's own option groups all module
    sources under a collapsed **"JUCE Modules"** filter (headers marked header-only), so the
    project shows just two top-level filters: `Source` and `JUCE Modules`.
  - `VS_STARTUP_PROJECT` → `CozyChorusSuite_Standalone` (F5 launches the standalone app).
- Verified by inspecting the regenerated `.slnx` and `.vcxproj.filters`. Configure is green;
  a full compile on this PC has **not** been run yet.

**Decisions:**
- Solution layout lives in `CMakeLists.txt`, never hand-edited in the `.slnx`.
- Chose JUCE's `JUCE_ENABLE_MODULE_SOURCE_GROUPS` (shows all module files, tidily grouped)
  over the default (only ~22 unity `.cpp` files, but dumped in the generic "Source Files"
  filter). One-line toggle if the leaner view is preferred later.

**Next up:**
- Full build on this box (CLI or in VS), audition M0 pass-through, then Milestone 1 (Chorus).

**Open questions / blockers:**
- The CMake solution-tidy changes are **uncommitted** — fold into the M1 commit or commit
  separately per the "one commit per milestone" rule (user's call).

---

## 2026-07-14 — Session 2: Milestone 0 — scaffolding

**Done:**
- Added JUCE as a submodule pinned to **8.0.14**; wrote a hand-rolled `CMakeLists.txt`,
  `.gitignore`, and the `Source/` skeleton (`PluginProcessor`, `Parameters.h`,
  `dsp/ModulationEffect.h`, `dsp/NullEffect.h`).
- Configured with the `Visual Studio 18 2026` generator and built **VST3 + Standalone**
  (Debug) — green. MSVC v145 compiled JUCE 8.0.14 + our code with no warnings-as-errors issues.
- Ran clang-format (VS-bundled, v22) over `Source/`; updated `CLAUDE.md` + this log.
- Compile/link verified; audio audition (standalone + DAW) is the user's step.

**Decisions:**
- Namespace is `CozyChorus`; our methods are `PascalCase`
  (`ModulationEffect::Prepare/Process/Reset`); JUCE's own names kept when overriding/calling JUCE.
- `COPY_PLUGIN_AFTER_BUILD` set **FALSE** — auto-install into `C:\Program Files\Common Files\VST3`
  hit `Permission denied` (needs admin). Point the DAW at the build folder instead.
- No `PluginEditor.{h,cpp}` yet — `createEditor()` returns `GenericAudioProcessorEditor`.

**Next up:**
- Audition M0 pass-through, then start **Milestone 1 (Chorus)**: shared `LFO` class + a
  delay-line `ChorusEffect` (Rate/Depth/Mix/Width, smoothed params, per-channel LFO phase offset).

**Open questions / blockers:**
- None open. Resolved: namespace = `CozyChorus`; `.claude/settings.local.json` git-ignored.

---

## 2026-07-14 — Session 1: Planning & project setup

**Done:**
- Absorbed the build brief: a VST3 + AU + Standalone suite of four modulation effects, built
  in fixed order **chorus → flanger → phaser → vibe**, as two DSP families (delay-line,
  all-pass) on one shared skeleton. A loadable plugin at the end of every milestone.
- Initialized git on branch `main`; wired remote `origin` =
  https://github.com/Seank23/CozyChorusSuite.git (remote was empty — first push lands at M0).
- Confirmed the Windows toolchain: **VS Community 2026 v18.7.3** (MSVC v145, C++20) +
  **CMake 4.4** (supports the `Visual Studio 18 2026` generator). AU is macOS-only, so it
  can't be built/tested on this box.
- Wrote **`CLAUDE.md`** (project reference) and this **`DEVLOG.md`**. Both currently
  uncommitted — will fold into the Milestone 0 commit.

**Decisions:**
- **LFO** = a single shared class, **instanced by each effect** (not one processor-owned instance).
- **`process()`** uses `juce::dsp::ProcessContextReplacing<float>`.
- **Latest JUCE** added as a **pinned git submodule**; **hand-rolled minimal `CMakeLists.txt`**;
  no Projucer.
- Plugin identifiers: target `CozyChorusSuite`, `PRODUCT_NAME` "CozyChorus Suite",
  `COMPANY_NAME` `Seank23`, bundle prefix `com.seank23`, `PLUGIN_MANUFACTURER_CODE` `Sk23`,
  `PLUGIN_CODE` `Cczs`, category Fx/Modulation.
- Docs kept current by hand: `CLAUDE.md` + `DEVLOG.md` updated each milestone/session; one
  commit per working milestone.
- Adopted the house C++ style: all hand-written `Source/` code follows `CPP-STYLE-GUIDE.md`,
  enforced by `.clang-format` / `.editorconfig` (all added to the repo this session).
  JUCE override names and the JUCE submodule are exempt.

**Next up:**
- On the user's "start Milestone 0" go-ahead: add the JUCE submodule + pin, write
  `CMakeLists.txt` and `.gitignore`, and build the pass-through `Source/` skeleton
  (`PluginProcessor` pass-through, APVTS with `mix` + `effectType`, `ModulationEffect` base +
  `NullEffect`, `GenericAudioProcessorEditor`). Build VST3 + Standalone, verify audio passes
  unchanged, then make the first commit.

**Open questions / blockers:**
- Whether to git-ignore `.claude/` (specifically `settings.local.json`) or track it —
  default is to ignore local settings; to confirm at M0.
- Project namespace name to satisfy the style guide's "single project namespace" rule
  (candidate: `CozyChorus`) — to pick at M0.
