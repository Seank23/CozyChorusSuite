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

## 2026-07-21 — Session 8: Milestone 4 — Vibe (Uni-Vibe, all-pass family) — suite DSP complete

**Done:**
- Implemented the Vibe (`Source/dsp/VibeEffect.{h,cpp}`) — the **last effect**, closing the all-pass
  family and the whole DSP milestone track. Reuses the Phaser's TPT all-pass kernel with three deltas:
  (1) **fixed 4 staggered stages** — one LFO-swept centre + a constant per-stage log offset
  (`{−0.75, −0.25, +0.25, +0.75}` octaves, precomputed in `Prepare`), so `G = g/(1+g)` is recomputed
  **per stage** (4 `tan()`/sample/channel); (2) an **asymmetric LFO owned by `VibeEffect`**
  (`GetAsymmetricShape`: piecewise-linear phase warp, `ASYM_K = 0.35`, then a sine — a smooth, skewed
  throb); (3) a **Chorus / Vibrato mode** (`effectiveMix = m_Vibrato ? 1.0f : mix`). **No feedback, no
  delay buffer** — only fixed `std::array` all-pass state. `fc` swept 200 Hz–2 kHz in the log domain,
  clamped to that range. Rate/Depth/Mix/Width all `SmoothedValue` (20 ms); LFO advances once per sample.
- Added one Vibe-specific param `vibeMode` (`AudioParameterBool` "Vibrato", default Off) to the APVTS
  (`Parameters.h`); wired `EffectType::Vibe → m_VibeEffect` in `PluginProcessor` (caches the `vibeMode`
  atomic, builds `VibeParameters` per block, `Vibrato = load() > 0.5f`). With this, **`GetActiveEffect()`
  routes every real selection to its own effect — `NullEffect` is now only the unreachable `default`
  guard.**
- Extended the shared `LFO` with a generic `GetPhase()` accessor (the only `LFO` change — the
  asymmetric *shape* stays in `VibeEffect`, so the other three effects still use the default sine).
- Extended the editor: a **"Vibrato" `ToggleButton`** (`m_VibeModeButton` + `ButtonAttachment`), shown
  only when Vibe is selected via the same 30 Hz `Timer` visibility mechanism; it slots into the wrapping
  grid generically (Vibe shows 4 shared knobs + the toggle = 5 controls). No layout-math change.
- Updated `CLAUDE.md` (status → **M4 done, suite DSP complete**; milestones; architecture tree +
  `NullEffect`/`LFO` notes; Vibe topology decision; Vibe DSP section + param table).

**Decisions:**
- **`VibeEffect` is a new sibling class**, not a subclass of `PhaserEffect` — the ~6-line TPT kernel is
  copied; Vibe *removes* the variable stage count and feedback, so inheritance buys nothing.
- **Asymmetric shape owned by the effect**, `LFO` stays shape-agnostic (just gains `GetPhase()`).
- **Mode is a bool + toggle button**; Vibrato forces 100 % wet, so the shared Mix knob is a no-op in
  Vibrato mode (documented, not greyed — greying is deferred editor polish).
- Stagger spread and `ASYM_K` are **tuned by ear, not measured** — a clean musical approximation of the
  hardware's mismatched-cap stage frequencies.

**Next up:**
- Build + audition M4 in the standalone host / DAW (Chorus-mode swirl vs. Vibrato-mode pitch wobble;
  confirm the throb and stereo width), then commit (one commit for the milestone).
- With all four effects done, the DSP track is complete — remaining work is the deferred **GUI polish
  pass**: custom `LookAndFeel`, per-effect panels, LFO visualiser (possibly OpenGL).

**Open questions / blockers:**
- Vibe tuning is by-ear-pending: stagger spread (`±0.75` octave), `ASYM_K` throb amount, and the
  200 Hz–2 kHz `fc` range want an audition against reference Uni-Vibe material. Fold into the GUI/tuning
  polish pass.
- Optional future params (deferred): expose `ASYM_K` / stagger spread; grey the Mix slider in Vibrato
  mode once the custom `LookAndFeel` lands.
- Still no `pluginval` / automated DSP test in-repo; verification remains a manual audition on this box.

---

## 2026-07-20 — Session 7: Milestone 3 — Phaser (all-pass family)

**Done:**
- Implemented the Phaser (`Source/dsp/PhaserEffect.{h,cpp}`) — the first **all-pass family** effect and
  the first with **no delay buffer**. Per channel: a cascade of N (2–12) hand-rolled **one-pole TPT
  all-pass** stages (`g = tan(π·fc/fs)`, `G = g/(1+g)`; each stage returns `2·lowpass − input`, one
  state var). The shared LFO modulates the all-pass cutoff `fc` in the **log domain** — centre/half-span
  precomputed in `Prepare` from `MIN_FC_HZ=200`/`MAX_FC_HZ=2000`, then `fc = exp(logCenter +
  logHalfSpan·depth·lfo)` clamped to range. **Feedback** wraps the whole cascade (`input +=
  feedbackState·feedback` before, `feedbackState = cascadeOutput` after; ±0.95). Stereo width reuses the
  per-channel LFO phase-offset trick. All state allocated in `Prepare`; Rate/Depth/Mix/Width/Feedback
  all `SmoothedValue` (20 ms).
- **Fixed a sweep-freezing bug:** `m_LFO.Advance()` was called once per *block* (outside the sample
  loop) instead of once per *sample*, so the LFO crawled ~block-size too slowly and the filter sounded
  static. Moved it inside the per-sample loop, matching `ChorusEffect`/`FlangerEffect`. Sweep now
  audible across the range.
- Added `phaserStages` (int 2–12, default 6) and `phaserFeedback` (−95…95 %, skew 0.4, default 0) to the
  APVTS (`Parameters.h`); wired `EffectType::Phaser → m_PhaserEffect` in `PluginProcessor` (caches the
  two atomic pointers, builds `PhaserParameters` per block, feedback → ±0.95).
- Extended the editor: `Stages` + `Feedback` rotary sliders, shown only when Phaser is selected (30 Hz
  `Timer` visibility, same mechanism as Chorus/Flanger). Only Vibe still falls through to `NullEffect`.
- Updated `CLAUDE.md` (status → M3 done; milestones; architecture tree; Phaser topology decision;
  Phaser DSP section + param table; reconciled the stale Flanger caveats against the shipped tuning).

**Decisions:**
- **Phaser = the shared all-pass skeleton** for Milestone 4 (Vibe): Vibe will reuse this cascade with
  staggered per-stage coefficients + an asymmetric LFO, rather than a fresh core.
- `Stages` is any int 2–12 (not restricted to even, despite the original brief note) — kept simple.

**Next up:**
- Build + audition M3 in the standalone host / DAW, then commit (one commit for the milestone).
- Milestone 4 — Vibe (last effect): staggered all-pass stages, asymmetric LFO, Chorus/Vibrato mode.

**Open questions / blockers:**
- Phaser tuning is by-ear-pending: default `fc` range (200 Hz–2 kHz), stage count feel, and whether the
  feedback taper/default want the same treatment the Flanger got. Defer to a polish pass.

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
