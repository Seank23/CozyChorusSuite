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

## 2026-08-09 — Session 15: pluginval wired up

Not a milestone — tooling. The `pluginval` validation step that had sat under "Optional/later" in
`CLAUDE.md` since M0 is now a first-class, one-command gate. No DSP, editor or parameter code was
touched; the plugin binary is byte-for-byte what Session 14 left behind.

**Done:**
- **`scripts/Run-Pluginval.ps1`** (new) — downloads Tracktion **pluginval v1.0.4** into `tools/pluginval/`
  on first use, runs it against the VST3 in the build tree, echoes the log and exits with pluginval's own
  code (0 = pass, 1 = fail). Params: `-Config Debug|Release` (default Release), `-Strictness 1..10`
  (default 5), `-TimeoutMs` (default 300000), `-SampleRates`, `-BlockSizes`, `-Repeat`, `-Randomise`,
  `-SkipGuiTests`, `-PluginPath`, `-ForceDownload`, and the common `-Verbose` mapped onto `--verbose`.
  Written to Windows PowerShell **5.1**-compatible syntax so it runs under either `pwsh` or the in-box
  shell.
- **`CMakeLists.txt`** — a `Validate` custom target (`pwsh -File Run-Pluginval.ps1 -Config $<CONFIG>`),
  deliberately **not** in `ALL`, `USES_TERMINAL` so output streams live, with
  `add_dependencies(Validate CozyChorusSuite_VST3)` so it can never validate a stale binary. Guarded by
  `if(WIN32)` and a `find_program(COZY_POWERSHELL NAMES pwsh powershell)`. Sits at the solution root
  next to `Docs`.
- **`.gitignore`** — `/tools/`, so the downloaded binary is never committed.
- **`CLAUDE.md`** — new *Validation (pluginval)* subsection under Build commands; the Tech-stack line no
  longer lists pluginval as optional.

**Results — the plugin passes `pluginval` clean at BOTH strictness 5 and strictness 10 (Release VST3).**
Strictness 5: 18 test groups, `SUCCESS`. **Strictness 10: 24 test groups, `SUCCESS`** — the extra six are
*Non-releasing audio processing*, *Plugin state restoration*, *Parameters*, *Background thread state*,
**Parameter thread safety** and **Fuzz parameters**. Audio processing and Automation ran the full default
matrix (44.1/48/96 kHz × 64/128/256/512/1024 = 15 pairs each; Automation adds a 32-sample sub-block).

Worth calling out, because each maps onto a known hazard in this codebase:
- **No NaN/Inf/subnormal findings.** The Session 14 Chorus `wetSum / voices` NaN would have been caught
  here — this is now a standing regression guard for it.
- **Fuzz parameters + Parameter thread safety passed.** That machinery hammers `effectType`, `vibeMode`
  and `phaserStages` from off-thread, i.e. precisely the mode-switch paths still on the open list. They
  don't crash or produce bad samples — the remaining concern there is *audible clicks*, which pluginval
  does not and cannot measure.
- **Editor + Open editor whilst processing + Editor Automation passed**, so the `CCSLookAndFeel` lifetime
  rule (declared-first / `setLookAndFeel(nullptr)`) and the `VBlankAttachment` teardown hold up under
  repeated open/close while audio runs.
- **Bus tests enumerated exactly the Mono/Stereo pair** `isBusesLayoutSupported` advertises, and restored
  the default 2-in/2-out layout cleanly. This is what keeps the unguarded `MAX_CHANNELS = 2` in
  Phaser/Vibe out of reach — the constraint is enforced at the layout-negotiation boundary, not in the
  effects, and that boundary is now verified.
- **Plugin state restoration passed**, so the APVTS save/load round-trip survives repeated restores.

**Decisions:**
- **Two Windows quirks are handled in the script, not left to the caller.** `pluginval.exe` is a JUCE
  **GUI-subsystem** binary, so PowerShell does *not* block on it — a bare `& pluginval.exe …` returns
  instantly and looks like a no-op. **Piping the output forces the wait** (the stdout handle only closes
  at process exit). For the same reason `--output-dir` is always passed and the log is echoed as a
  fallback if nothing reaches stdout. In practice this build *does* write to the console, but the
  fallback costs nothing and survives a future pluginval repackaging.
- **Release is the default target, not Debug.** Debug is slow enough to trip timeouts and a JUCE
  `jassert` there opens a **modal dialog that hangs an unattended run**. Debug validation is for when
  you're attached to a debugger, chasing a specific failure.
- **`Validate` stays out of `ALL`.** A strictness-10 sweep runs for minutes across 15 rate/block pairs —
  a gate you invoke, not a tax on every build.
- **`$<CONFIG>`, not `CMAKE_BUILD_TYPE`.** Visual Studio is a multi-config generator, so the
  configuration isn't known at configure time; the generator expression resolves at build time. It is
  kept out of `COMMENT`, where generator-expression support varies across the CMake version range.

**Next up:**
- Adopt `-Strictness 10 -Repeat 3 -Randomise` as the pre-release ritual — repeats with a shuffled test
  order are what shake out order-dependent state bugs that a single ordered pass can miss.
- pluginval says nothing about *sound*, so the by-ear polish passes still open (Vibe stagger/`ASYM_K`
  against reference Uni-Vibe material, Character weights, Phaser tuning) are unaffected by this green run.

**Open questions / blockers:**
- **`Reported latency: 0` in the Plugin info test.** Expected rather than alarming — that test runs
  before `prepareToPlay`, and `setLatencySamples(m_CharacterStage.GetLatencySamples())` only fires there,
  so pluginval is reading the pre-prepare value. Worth confirming against a host that re-queries after
  preparation before calling the PDC path proven.
- The **vst3 validator** step is skipped (`--vst3validator` path unset). Steinberg's own `validator.exe`
  ships with the VST3 SDK; wiring it in would add another layer, but it isn't vendored here.
- Still unaddressed from Session 14, and **not** something pluginval flags: mode-switch clicks (Vibe
  `effectiveMix` 0.5→1.0 on the Vibrato toggle; Phaser `m_Stages` changing instantly with stale state
  reused on regrown stages), the spectrum-vs-Vibrato blend gap, and the ~34 narrowing warnings
  (C4244/C4267) plus 5 unreferenced-parameter warnings in `drawComboBox`.

---

## 2026-08-08 — Session 14: Code review, fix pass, and doc reconciliation

Not a milestone — a review of the finished plugin, the fixes that came out of it, and a pass over
`CLAUDE.md` to make it match the code again. Claude reviewed and advised; **the code changes in this entry
were written by hand** (Claude edited only `CLAUDE.md` and this file).

**Done — review:**
- **Read all 26 files in `Source/`** against the JUCE 8.0.14 sources in the submodule (`SmoothedValue`,
  `ProcessSpec`, `Oversampling`, `Path::addCentredArc`, `JUCE_SNAP_TO_ZERO`), and rebuilt the
  `CozyChorusSuite` shared-code target in Debug to inventory compiler warnings.

**Done — bugs found and fixed:**
1. **Chorus emitted NaN for ~294 samples (6.7 ms @ 44.1 k) whenever it became the active effect.**
   `Prepare` called `smoothedVal->reset(...)` **before** `SetParameters(ChorusParameters{})`. JUCE's
   `reset(numSteps)` does `setCurrentAndTargetValue(this->target)` and a default
   `SmoothedValue<float, Linear>` has `target == 0` (`juce_SmoothedValue.h:244,274`), so `m_Voices` ramped
   **from 0**; `static_cast<int>` floored it to 0 until the ramp crossed 1.0, the voice loop never ran, and
   `wetSum / voices` was `0.0f / 0` → **NaN** into the output block and on into `CharacterStage`.
   `JUCE_SNAP_TO_ZERO` flushes NaN out of the downstream filter states, so it was a burst rather than a
   permanent kill — but it fired on load (Chorus is the default effect) and again on every re-selection,
   and would have failed `pluginval`. **Fixed** by moving `SetParameters` above the reset loop in all four
   effects (with `stepsToTarget` still 0, `setTargetValue` snaps current to target, and the following
   `reset` keeps it there), plus `voices = std::max(static_cast<int>(...), 1)` as a second line of defence.
   Recorded as a **settled design decision** in `CLAUDE.md` — the ordering is load-bearing.
2. **The rotary value arc never rendered.** `drawRotarySlider` passed `rx`/`ry` — the arc's bounding-box
   **corner** — where `Path::addCentredArc` expects **radiusX/radiusY**. For the 60×66 rotary area that is
   radii (4.5, 7.5) instead of (25.5, 25.5): a tiny ellipse centred on the knob, which the `fillEllipse`
   below it then painted over. Stacked on that, the `- π` applied to `angle` (cancelled by the pointer
   maths, so the pointer was right) was fed to the value arc as its **end angle**. **Fixed**: arcs now use
   `radius, radius`, `angle` lost the `- π`, and the pointer switched to `angle + halfPi` — algebraically
   the same direction as before (`(a - π) - π/2 ≡ a + π/2`), so the pointer is unchanged and the arcs are
   now correct.
3. **`m_ProcessSpec` was uninitialised.** `juce::dsp::ProcessSpec` has no default member initialisers
   (`juce_ProcessContext.h:44-54`), so it held indeterminate values until `prepareToPlay`;
   `ModulationVisualiser` read it in its constructor and every vblank and divided by it — `0.0f / 0.0f`
   → NaN → `Path::lineTo` with NaN coords, which asserts in debug JUCE. **Fixed**: `m_ProcessSpec{}`,
   `GetProcessSpec()` now returns a `const&`, and `UpdateVisualisation` bails on `sampleRate <= 0`.
4. **The editor relaid out and re-showed every knob 30×/s.** `if (idx != m_LastEffectIndex) m_LastEffectIndex = idx;`
   was a no-op and `RenderComponents()` ran unconditionally, so each visible knob went
   `true → false → true` per tick with a `repaintParent()` on each transition (and mid-drag visibility
   toggling is its own hazard). **Fixed**: re-render only on change, with `m_LastEffectIndex` seeded from
   the parameter in the constructor so the first `resized()` already lays out the right effect.
5. **`size_t` underflow on the first layout** — `n == 0` made `(n - 1) * kGap` wrap. **Fixed** by `int n`
   (and made unreachable anyway by the seeding in 4).
6. **The visualiser published the wrong delay tap** — see the dedicated section below.
7. Smaller, all **fixed**: `useIntegerLatency = true` on the `Oversampling` ctor so the PDC figure is an
   exact integer instead of a rounded fractional one; `m_SampleRate` promoted `float → double` in
   `CharacterStage` and `ModulationVisualiser`; `CharacterStage::Process` now honours
   `context.isBypassed` like the four effects do.
8. **Dead code removed:** `fwidth`/`fheight`, the unused `depth` in `EvaluateTransferFunction`, the three
   identical sine cases in `SampleShape` (collapsed to fallthrough), `kCellPadX/Y`, `kMaxColumns` and the
   duplicate `kBackground/kTitleText/kCaptionText` in `EditorConstants.h` (which also moved out of an
   anonymous namespace into `namespace CozyChorus` and dropped its JUCE include), and the empty
   hand-written ctors/dtors in all five DSP classes.

**Done — the delay tap the visualiser publishes (finding 6):**
- M6b assigned `m_DelayInSamples` from inside the voice **and** channel loops, so it ended up holding the
  **last voice on the last channel** — up to ±4 ms of voice spread and ~240° of LFO phase away from the
  phase published beside it.
- **Fix shipped:** write the member only under `if (ch == 0 && v == 0)` (Chorus) / `if (ch == 0)`
  (Flanger). Rationale: **voice 0 on channel 0 is the only tap read at LFO phase offset `0.0f`** — the
  same phase `GetCurrentLFOPhase()` publishes — so the Signal and LFO views stay in lock-step. The per-tap
  local that M6b had replaced with the member is back and is what `popSample` receives, so the exported
  value is literally (post-clamp) the number handed to the delay line, not a re-derivation. Renamed to
  `m_ReferenceDelayInSamples`; kept a plain `float` because `GetDelayInSamples()` is only ever called from
  `processBlock` (`PluginProcessor.cpp:174,176`), so the atomic boundary is already in the right place.
- Accepted trade-off: voice 0's base delay is 20/18/16 ms at 1/2/3 voices, so the displayed offset steps
  with the Voices knob.

**Done — visual tuning (by ear, unrelated to the review):**
- `kVisCyclesShown` 6 → **4**, `kVisPxPerMs` 2.0 → **1.0**.
- Knob taper skews: Flanger Base Delay 0.1 → **0.4**; Phaser and Vibe Rate 0.35 → **0.4** (Chorus and
  Flanger Rate left at 0.35).

**Done — `CLAUDE.md` reconciled** (4 corrections, plus the Session 14 state):
- **Vibrato defaults to ON** — `VibeParameters::Vibrato = true` feeds the APVTS default, so the Vibe boots
  as a vibrato with Mix greyed out. The doc said "default Off" in three places. **The code is the intent;
  the doc was wrong.** Fixed in the M4 param table, the M4 topology decision and the Vibe DSP reference.
- **Wow/flutter drift is per *sample*, not per block** — `CharacterStage.cpp` draws a `juce::Random` value
  inside the per-sample loop; the doc claimed per-block in three places. Code is again the right version:
  the `1/√(coeff/2)` makeup is derived for white noise at sample rate, and a per-block draw would make the
  drift block-size dependent. That reasoning is now written down so it does not get "corrected" back.
- **`Palette::Screw` does not exist** — removed from the M6a palette list and from the open-cleanups list,
  along with the claim that `Palette::Trace` duplicates its value.
- **`m_ScreenZone` is already gone** from the editor header — removed from the open-cleanups list.
- Open-cleanups and PDC/visualiser passages updated to the post-fix state; the new `Prepare` ordering rule
  added under Settled design decisions.

**Decisions:**
- **Docs follow the code on both mismatches** (Vibrato default, per-sample drift) — the shipped behaviour
  is what we want in both cases, so nothing in `Source/` changed for them.
- **`docs/Milestone-*.md` guides are left untouched.** They are point-in-time implementation guides, not
  state documents; `Milestone-4-Vibe-Guide.md:174` still sketches `Vibrato = false` and that is fine as
  history. `CLAUDE.md` is the single source of truth for current state.

**Next up:**
- **Audition the fixes** — the `Prepare` reorder changes first-block behaviour for every effect (controls
  now start *at* their values instead of ramping up from 0 over 20 ms), and the knob now draws a real
  value arc. Neither has been heard/seen yet.
- Then the still-open items below, then `pluginval`, Catch2 DSP tests, and the by-ear tuning passes (Vibe
  stagger / `ASYM_K`, M5b weights).

**Open questions / blockers:**
- **Still open from the review:** mode-switch clicks (Vibe `effectiveMix` jumps 0.5→1.0 on the Vibrato
  toggle; Phaser `m_Stages` changes instantly and stale state on removed stages is reused if the count
  goes back up); smoothing the integer `Voices` count still only moves the click rather than removing it;
  `MAX_CHANNELS = 2` unguarded in Phaser/Vibe (safe only because `isBusesLayoutSupported` rejects >2);
  `LFO::SetShape` and its Triangle/Saw/Square branches unreachable; `thinkness` typo; `EditorConstants.h`
  has no trailing newline.
- **Warnings after the fix pass: 34 sites, all narrowing** — C4267 `size_t → int` from
  `block.getNumChannels()/getNumSamples()` in every `Process`, C4244 `double → float` from `0.001 *
  m_SampleRate` expressions, C4244 `int ↔ float` in the editor/visualiser paint paths — plus 5 C4100
  unreferenced-parameter warnings in `drawComboBox`. The C4189 dead-variable ones are gone.
- **The knob cap covers the inner half of the new value arc** (cap diameter is `radius * 2`, the arc is
  stroked at `radius` with `radius * 0.18` thickness), so the arc reads as a thin outer ring. Worth a look
  to decide whether the cap should shrink to ~`radius * 0.7`.
- The spectrum plot still blends with `mix` even for the Vibe, where **Vibrato forces 100 % wet** — and
  Vibrato is now confirmed as the *default*, so the wrong curve is the first thing a user sees on the Vibe.
- Richer alternative for the delay export, deferred: publish **all** voice taps (array of atomics + count)
  and draw one wet trace per voice, so the Signal view shows the ensemble honestly rather than one
  representative tap.

---

## 2026-08-08 — Session 13: Milestone 6b — Modulation visualiser (the screen comes alive)

**Done:**
- **`ModulationVisualiser`** (`Source/Editor/ModulationVisualiser.h/.cpp`, new) — a plain `juce::Component`
  driven by a **`juce::VBlankAttachment`** (vsync-paced repaints, **no OpenGL**, no `Timer`), hosted in the
  centre zone M6a reserved. It paints its own recessed screen (`Palette::Screen` fill + inner shadow line),
  a corner caption naming the current view, and one of two modes.
- **Mode toggle:** left-click cycles `Mode::LFO ↔ Mode::Response` (`mouseDown`, pointing-hand cursor),
  default **Response**. **View state only** — no parameter, not persisted; caption reads "LFO" / "Signal"
  (delay family) / "Spectrum" (all-pass family).
- **LFO mode:** `kVisCyclesShown = 6` cycles across the width, drawn from the **live DSP phase** so it
  scrolls in lock-step with the audio (and freezes when the host stops processing). Amplitude ∝ that
  effect's Depth; shape per effect — sine for Chorus/Flanger/Phaser, `VibeEffect::GetAsymmetricShape` for
  the Vibe throb.
- **Response mode, delay family (Chorus/Flanger):** a dry sinusoid (`Palette::Trace`) and a wet copy
  (`Palette::Accent`) offset horizontally by the **live** delay at `kVisPxPerMs = 2 px/ms` — the modulation
  reads directly as a breathing time shift.
- **Response mode, all-pass family (Phaser/Vibe):** the **analytic** magnitude response of the wet/dry sum
  on a log axis (50 Hz–20 kHz, −40…+20 dB; gridlines at 0/−40 dB and 100 Hz/1 k/10 k). Each first-order
  all-pass contributes `−2·atan(f/fc)`; Phaser wraps `A = e^{jφ}` (`φ = stages·−2 atan(f/fc)`) in feedback
  as `A' = A/(1 − fb·A)`, Vibe sums its 4 staggered stage phases (no feedback); then
  `H = mix·A' + (1 − mix)`, plotted as `20·log10|H|`. `EvaluateCutoffFrequency()` repeats the DSP's own
  log-domain sweep so the notches track the real modulation. **No FFT, no audio buffer crosses to the GUI.**
- **Audio → GUI hand-off (lock-free):** `PluginProcessor` publishes `m_VisualPhase` and
  `m_VisualDelayInSamples` (both `std::atomic<float>`, `memory_order_relaxed`) at the **end** of
  `processBlock`, with `GetVisualPhase()` / `GetVisualDelayInSamples()` / `GetActiveEffectType()` /
  `GetProcessSpec()` accessors. The `juce::dsp::ProcessSpec` was promoted from a local in `prepareToPlay`
  to the member `m_ProcessSpec` so the view can read the live sample rate. No locks, no allocation on the
  audio thread, no latency change.
- **Read-only DSP exports:** `ModulationEffect::GetCurrentLFOPhase()` on the **base** (one line covers all
  four effects); `GetDelayInSamples()` on Chorus + Flanger, backed by a new `m_DelayInSamples` member that
  replaces the old local `delaySample`; `PhaserEffect::MIN_FC_HZ/MAX_FC_HZ` and
  `VibeEffect::NUM_STAGES/MIN_FC_HZ/MAX_FC_HZ/STAGE_OFFSET` promoted to **public**, plus
  `VibeEffect::GetAsymmetricShape` made **public static**. Vibe's `Prepare` now builds `m_StageLogOffset`
  from that shared `STAGE_OFFSET` table instead of a local copy.
- **A real (small) DSP change — 6b is not view-only:** `m_LFO.SetFrequency(m_RateHz.getNextValue())` moved
  **into** the per-sample loop in all four effects. It had been called once per block, so the Rate smoother
  advanced a single step per block and its 20 ms ramp was effectively multiplied by the block size. Now the
  Rate smoother runs at sample rate like Depth/Mix/Width; steady-state sound is unchanged.
- **Editor wiring:** `m_ModulationVisualiser` constructed with the `PluginProcessor&` and given the centre
  remainder directly in `RenderComponents()`; the screen-placeholder block deleted from `paint()`.
  `CCSAudioProcessorEditor.h` now **forward-declares** `LabeledKnob` + `ModulationVisualiser` (headers moved
  to the `.cpp`). New shared metrics `kVisCyclesShown` / `kVisPxPerMs` in `EditorConstants.h`; new
  `Palette::Trace` colour for the dry curve.
- **`CMakeLists.txt` untouched** (globbed sources), **`Parameters.h` untouched** (no new parameter).
- Verified: clean incremental `cmake --build build --config Debug` — VST3 + Standalone both link.
- Updated `CLAUDE.md` (Current status, build-order — 6b marked done, `Source/` tree, a full M6b
  settled-design-decision block, the shared-LFO + Vibe DSP-reference notes) and added this entry.

**Decisions:**
- **`VBlankAttachment` over a `Timer`** for the animation — repaints are paced by the display's vsync, so
  the curve is smooth and can't outpace the screen. The 30 Hz editor `Timer` still handles per-effect
  control visibility; the two drivers stay separate.
- **Plain `juce::Graphics`, no OpenGL** — the M6a "possibly OpenGL" note is resolved. Path strokes at this
  size are cheap; an OpenGL context would have added lifetime and driver surface for no visible gain.
- **Publish scalars, never buffers.** The GUI gets a phase and a delay — two atomics — and re-derives every
  curve from the APVTS + the DSP's own constants. No FIFO, no audio data crossing threads, nothing for the
  audio thread to block on.
- **Share the DSP's constants rather than mirror them.** Making the Phaser/Vibe frequency bounds, stage
  table and asymmetric shape public was preferred over re-typing magic numbers in the view — the plot can't
  drift from the sound. The cost is a slightly wider public surface on two effect classes.
- **Response as the default mode** (not LFO): it's the view that shows what the effect is *doing* to the
  signal; the LFO curve is the diagnostic one.
- **Mode is not a parameter.** It's per-editor view state — deliberately not persisted or automatable.

**Next up:**
- **The milestone list is finished** — DSP (M0–M5b) and GUI (M6a–M6b) are all shipped, plugin is
  feature-complete and loadable as VST3 + Standalone.
- Remaining work is all polish, in rough priority order: by-ear tuning passes still open from Sessions
  10–11 (M5/M5b endpoints, Vibe stagger + `ASYM_K`, per-effect defaults), **`pluginval`** validation, and
  **Catch2** DSP tests. Presets and through-zero flanging remain optional stretch ideas.

**Open questions / blockers:**
- **Dead code still not swept:** `EditorConstants.h` keeps the pre-M6a grid constants (`kMaxColumns`,
  `kCellPadX/Y`) and the duplicate `kBackground/kTitleText/kCaptionText` colours superseded by `Palette`;
  `m_ScreenZone` is still **declared** in `CCSAudioProcessorEditor.h` but never assigned now that the
  visualiser owns those bounds. All harmless, all one small cleanup commit.
- `timerCallback()` still calls `RenderComponents()` every tick (full relayout at 30 Hz) instead of only on
  effect/mode change — unchanged from 6a, still a cheap win if wanted.
- `Palette::Trace` is currently the same value as `Palette::Screw` (`0xff5a4c42`) — fine visually, but the
  duplicate is worth a deliberate look on a colour pass.
- The visualiser's spectrum mode reads `mix` even for the Vibe, where **Vibrato mode forces 100 % wet** in
  the DSP — so with Vibrato on the plotted curve is flatter than what's actually heard. Small fidelity gap,
  not a crash.
- `docs/` (the per-milestone implementation guides, now including
  `Milestone-6b-Visualiser-Guide.md`) is still **untracked** — decide whether it joins the repo.

---

## 2026-08-03 — Session 12: Milestone 6a — Pedal UI (custom LookAndFeel + zone layout)

**Done:**
- First **pure-GUI** milestone — **no DSP change**. `PluginProcessor`, `Parameters.h`, the `dsp/` tree and
  `CMakeLists.txt` are all untouched; the three new editor files compile via the existing source glob.
- **`CCSLookAndFeel`** (`Source/Editor/CCSLookAndFeel.h/.cpp`) — a custom `juce::LookAndFeel_V4` skin drawn
  **entirely in code, no image assets**. Overrides `drawRotarySlider` (unfilled track arc + amber value
  arc + flat knob cap + pointer line), `drawComboBox`, `drawPopupMenuBackground`, and `drawToggleButton`
  (an amber **LED** with a glow halo + caption when lit, reserving the same value-box band so it centres
  with the knob dials). Label / text-box / combo / popup **colours** are set in the constructor via
  `setColour(...)` instead of overriding `drawLabel`. A shared **`Palette`** namespace
  (`namespace CozyChorus::Palette`, in the header) holds the cozy amber-on-brown colours. One
  `setLookAndFeel(&m_LookAndFeel)` on the editor **cascades** to every child (JUCE walks the parent chain
  to resolve a control's L&F), so the sliders **inside** each `LabeledKnob` are skinned too.
- **`LabeledKnob`** (`Source/Editor/LabeledKnob.h/.cpp`) — a reusable `juce::Component` composite: a
  caption `Label` stacked over a `RotaryHorizontalVerticalDrag` `Slider` with a `TextBoxBelow` read-out.
  `resized()` slices the caption off the top; `getSlider()` exposes the inner slider for APVTS
  attachment. This **retires** the old paint-time `captionFor` map + caption-band loop — captions are now
  real child components. Every knob is a `std::unique_ptr<LabeledKnob>`; Vibrato stays a bare
  `juce::ToggleButton`.
- **`EditorConstants.h`** — new header holding the shared layout metrics (margins, header height, zone
  widths, `kKnobWidth/Height`, corner radii, screw sizes) read by **both** `resized()` and `paint()`.
- **`CCSAudioProcessorEditor` rewrite** — retired the wrap-at-4 uniform grid for a fixed **560×440
  guitar-pedal faceplate**, sliced once per `resized()` with `removeFromTop/Left/Right/Bottom`:
  **header** (brand plate painted + effect-selector `ComboBox` on the right) · **Mix** top-left, centred
  in its zone via `withSizeKeepingCentre(kKnobWidth, kKnobHeight)` · framed **CHARACTER** box top-right,
  interior halved for **Warmth + Age** · a recessed **"screen"** reserved in the centre (stored as
  `m_ScreenZone`, painted as a placeholder — the future M6b visualiser bounds) · the **active effect's**
  controls **centred** in a bottom row. `paint()` draws the chassis plate, brand plate, CHARACTER frame +
  title, and the screen recess; the 30 Hz `Timer` still swaps per-effect visibility.
- **Mix greyed in Vibrato:** the timer reads `vibeMode`; Vibe active + Vibrato on ⇒
  `m_MixKnob->getSlider().setEnabled(false)` (disabled draw path, Mix is ignored by the Vibe DSP there),
  without dropping it from the layout.
- Updated `CLAUDE.md` (Phase/Current status, editor bullet, build-order — Milestone 6a marked done + a new
  Milestone 6b line, `Source/Editor/` tree entries, and a full M6a settled-design-decision block) and
  added this DEVLOG entry.

**Decisions:**
- **Flat-vector "cozy" aesthetic, no assets** (locked in the M6a planning forks, alongside: animated LFO
  viz deferred to **6b**, and splitting the GUI work **6a → 6b**). Everything is `juce::Graphics`
  primitives + a shared `Palette` — nothing to bundle, resolution-independent, easy to retint.
- **One `LookAndFeel` for the whole editor, cascaded** rather than per-control skins — the idiomatic JUCE
  approach; the single `setLookAndFeel` reaches composite children automatically.
- **Lifetime rule enforced:** `m_LookAndFeel` declared **first** (destructs last) + `setLookAndFeel(nullptr)`
  in the destructor — both required so no child dangles a L&F pointer during teardown.
- **Colours via `setColour` over `drawLabel`:** simpler than a full label-draw override for what 6a needs
  (just text colour); the draw overrides are reserved for the shapes that actually change (knob, combo,
  popup, toggle LED).
- **Metrics centralised in `EditorConstants.h`** so `paint()` (frames/titles) and `resized()` (control
  positions) read the same numbers and can't drift — the discipline the pre-M6a caption band already relied on.

**Next up (M6b):**
- The **animated LFO visualiser** rendered into the reserved `m_ScreenZone` — a `ModulationVisualiser`
  view (possibly OpenGL) reading the active effect's LFO. Only remaining GUI work.

**Open questions / blockers:**
- **Dead code to sweep when 6b lands:** `EditorConstants.h` still carries the old grid constants
  (`kMaxColumns`, `kCellPadX/Y`) and a duplicate `kBackground/kTitleText/kCaptionText` colour set now
  superseded by `Palette`.
- `timerCallback()` calls `RenderComponents()` **every tick** (full relayout at 30 Hz) rather than only on
  effect/mode change — harmless on the message thread, but a cheap win to gate on change if wanted.
- `Palette::Screw` + screw metrics are defined but **not yet drawn** in `paint()` — a small decorative
  extra left for a polish pass.
- By-ear DSP tuning from Sessions 10–11 (M5/M5b endpoints, per-effect defaults) and the missing
  `pluginval` / automated DSP test remain open, unchanged by this GUI milestone.

---

## 2026-07-31 — Session 11: Milestone 5b — Character tape age (wow/flutter)

**Done:**
- Added **wow/flutter tape-age** to the existing `CharacterStage` (user-implemented; I verified + guided).
  It runs **first in `Process`, before the oversampled saturation**: a modulated fractional delay
  (`juce::dsp::DelayLine<Lagrange3rd>`, 2 ms centre ± 1.5 ms span, `+4` headroom samples) whose delay is
  `(CENTER_DELAY_MS + HALF_SPAN_DELAY_MS·mod·Age)·0.001·fs`.
- `mod = WOW_WEIGHT·wow + FLUTTER_WEIGHT·flutter + NOISE_WEIGHT·drift` (weights **0.8 / 0.04 / 0.02**):
  two `LFO` sines (**wow 0.556 Hz**, **flutter 12 Hz**, advanced once per sample) plus **band-limited
  random drift** — one `juce::Random` sample **per block**, made bipolar and run through a one-pole LPF
  (`NOISE_LPF_HZ = 2 Hz`, coeff precomputed in `Prepare`) with a `1/√(coeff/2)` **makeup gain** to
  restore amplitude after the heavy lowpass. **One shared `mod`/delay across channels** (mono-correlated,
  physically correct for a single transport). **No feedback.**
- New global **`age`** param (`AudioParameterFloat`, 0–100 %, default **30**, cached atomic in the
  processor, smoothed 20 ms, read **per sample**); scales `mod`, so Age 0 % ⇒ static 2 ms delay
  (inaudible) and 100 % ⇒ full wander. Editor gains an **Age** rotary knob, always visible like Warmth,
  placed right after it. `Reset()` now also flushes the delay line, both LFOs, the noise state, and
  the Age smoother.
- **Also retuned Warmth (M5) by ear during this pass:** `DRIVE_MAX` **4 → 2.5**, high-cut floor
  `MIN_WARMTH_CUTOFF_HZ` **6.5 kHz → 4 kHz** (so `drive = 1 + Warmth·2.5`, cut 18 kHz → 4 kHz).
- Updated `CLAUDE.md` (Phase/Current status, params bullet, latency bullet, build-order — added
  Milestone 5b / GUI bumped to 8, `Source/` tree comment, a Character tape-age settled-design decision,
  Warmth-number reconciliation, and the DSP-reference Character section + Age param row) and added this
  DEVLOG entry.

**Verify/fix during the pass:**
- **Age was inaudible (0 % vs 100 % identical):** `SetParameters` set only `m_Warmth` and never
  `m_Age`, so the smoother stayed pinned at 0 and zeroed the whole `mod·Age` term — the delay sat static
  at 2 ms. Fixed by adding `m_Age.setTargetValue(params.Age)`.
- **Raw random gargled:** the first cut sampled `m_Random.nextFloat()` per sample (full-bandwidth,
  unipolar → DC-biased) → harsh noise, not drift. Replaced with the bipolar per-block sample + 2 Hz
  one-pole LPF + makeup path above.
- Flutter deliberately carries a **much smaller weight** than wow (0.04 vs 0.8): equal weight would make
  12 Hz swing pitch ~20× harder than 0.556 Hz for the same delay amplitude → seasick.

**Decisions:**
- **Wow/flutter only — vinyl/tape noise dropped** (user call). A single **Age** macro over the whole
  modulation, mirroring Warmth: one knob, brand-consistent "Cozy" simplicity.
- **Pitch modulation via time-varying fractional delay** (Lagrange3rd) rather than a resampler — reuses
  the delay-line family's kernel and is allocation-free once prepared.
- **~2 ms wow centre delay is NOT PDC-reported** (`GetLatencySamples()` still returns only the
  oversampler) — it's a time-varying modulation delay, treated as part of the effect, not fixed latency.
- **Mono-correlated modulation** (one `mod` for both channels) — a real tape has one transport; a
  per-channel offset would smear the image and isn't physical here.

**Next up:**
- By-ear tuning of the M5b endpoints (weights, wow/flutter frequencies, drift LPF, delay span) and the
  M5 Warmth voicing, alongside the still-open per-effect default tuning (Vibe stagger/`ASYM_K`, Phaser,
  Flanger).
- Then the deferred **GUI polish pass**: custom `LookAndFeel`, per-effect panels, LFO visualiser; grey
  the Mix knob in Vibe's Vibrato mode.

**Open questions / blockers:**
- Warmth coefficients still update **once per block** (Age is per-sample). Fine at static settings; if
  faint stepping is audible while *automating* Warmth, move the drive/cutoff recompute per-sample.
- `drift` is sampled once per block, so its effective input rate is block-rate (then LPF'd to ~2 Hz) —
  audibly fine, but its texture is very slightly block-size-dependent. Not worth changing unless heard.
- Unchanged from Session 10: still no `pluginval` / automated DSP test in-repo.

---

## 2026-07-28 — Session 10: Milestone 5 — Character (tape-warmth) stage

**Done:**
- Implemented the **Character stage** (`Source/dsp/CharacterStage.{h,cpp}`) — the first non-effect DSP
  and the first **global** processor. It is **not** a `ModulationEffect` and **not** in the `effectType`
  switch: `PluginProcessor` owns one `m_CharacterStage` and runs it **unconditionally on the output
  after** `GetActiveEffect().Process(context)`, so it colours whichever effect is active (and the dry
  signal, since it sits post-Mix). Reuses the effects' `Prepare`/`Process`/`Reset` + POD-`SetParameters`
  shape for consistency.
- DSP: **2× oversampled** (`juce::dsp::Oversampling`, `filterHalfBandPolyphaseIIR` = minimum-phase for
  low latency) **asymmetric-tanh saturation** —
  `y = (tanh(drive·(x+BIAS)) − tanh(drive·BIAS))·invS0`, `drive = 1 + Warmth·4`, `BIAS = 0.15` (even
  harmonics), `invS0` = slope-normalised makeup (unity small-signal gain, so Warmth compresses peaks
  rather than raising level) — then a one-pole **high-cut** (`FirstOrderTPTFilter`) swept 18 kHz → 6.5 kHz.
  A single **Warmth** macro drives both.
- Wiring: global `warmth` param (`AudioParameterFloat`, 0–100 %, default **30**, cached atomic in the
  processor); `CharacterStageParameters` POD built each block from it; `m_CharacterStage.Prepare(spec)`
  in `prepareToPlay` followed by **`setLatencySamples(m_CharacterStage.GetLatencySamples())`** — the
  suite's **first PDC / host delay compensation**. Editor gains a **Warmth** rotary knob, always visible
  like Mix (independent of `effectType`), placed right after Mix.
- **Crackle fix (user-reported):** at high Warmth the saturation loop iterated the *pre*-oversampling
  sample/channel counts (`< numSamples - 1`, `< numChannels - 1`), so the back half of every 2× block
  passed through unsaturated → a per-block discontinuity heard as low-frequency crackle (bit-crusher-like),
  and the last channel was skipped (right channel unprocessed in stereo). Fixed to iterate the
  oversampled block's own `getNumSamples()`/`getNumChannels()`; dropped the now-unused `numChannels`
  local. Release VST3 rebuilds clean (only pre-existing Phaser/Vibe `size_t→int` warnings remain).
- Updated `CLAUDE.md` (Current status, build-order list — M5 added / GUI bumped to 7, `Source/` tree,
  a Character settled-design decision, and a Character DSP-reference section + param table) and added
  this DEVLOG entry.

**Decisions:**
- **Global post-effect placement, one Warmth macro** — brand-consistent "Cozy" voice with a single
  control; the consequence (colours even a fully-dry signal, since it's post-Mix) was accepted.
- **Slope-normalised makeup** (`invS0`) over endpoint normalisation — keeps small-signal gain at unity
  so Warmth reads as *tone/compression*, not loudness; avoids the "louder = better" pitfall.
- **2× minimum-phase IIR oversampling** — enough headroom against tanh aliasing while keeping latency to
  a few samples (matters for live guitar), at the cost of some phase non-linearity (inaudible here).
- **Tape voicing, core only** — saturation + high-cut this pass; wow/flutter + vinyl noise deferred to
  a later **5b**.

**Next up:**
- Optional **5b**: wow/flutter (slow pitch drift) + subtle vinyl/tape noise for more character.
- By-ear tuning of the Character endpoints (`DRIVE_MAX`, `BIAS`, cutoff range) alongside the still-open
  per-effect default-value tuning (Vibe stagger/`ASYM_K`, Phaser, Flanger).
- Then the deferred **GUI polish pass**: custom `LookAndFeel`, per-effect panels, LFO visualiser; grey
  the Mix knob in Vibe's Vibrato mode.

**Open questions / blockers:**
- Warmth coefficients update **once per block** (not per sample) — fine at static settings; if faint
  stepping is audible while *automating* Warmth, move the drive/cutoff recompute per-sample.
- Heavy asymmetric compression at max Warmth (drive 5 ⇒ ~18 dB peak reduction on full-scale input) — a
  voicing choice; lower `DRIVE_MAX` if it feels too quiet/dark, don't touch the normalisation.
- Unchanged from Session 9: by-ear tuning still pending; still no `pluginval` / automated DSP test in-repo.

---

## 2026-07-22 — Session 9: Per-effect Rate/Depth/Width params + doc reconciliation

**Done:**
- Recorded a design change that had drifted ahead of the docs: **Rate / Depth / Stereo Width** are now
  **per-effect** APVTS params (`chorusRate`/`chorusDepth`/`chorusWidth`, and likewise `flanger*` /
  `phaser*` / `vibe*`), each defaulting from that effect's own `XxxParameters` POD in
  `CreateParameterLayout()`. **`mix` is the only remaining shared param.** Already wired end-to-end —
  `PluginProcessor` caches the per-effect atomics and loads each in its `processBlock` case, and
  `CCSAudioProcessorEditor` holds a per-effect slider + attachment set toggled by the existing 30 Hz
  visibility `Timer`. Effect DSP unchanged (only the source param IDs moved).
- Reconciled `CLAUDE.md` to match: **Current status** (new per-effect-params bullet); Phaser/Vibe
  **milestone bullets**; the **Design-principle** line; the Flanger/Phaser/Vibe **topology decisions**
  and the **Editor decision**; a new **per-effect-params settled-design bullet**; and the
  Flanger/Phaser/Vibe **parameter tables** (Rate/Depth/Width re-tagged per-effect, Mix stays shared).
  Added this DEVLOG entry.

**Decisions:**
- **Per-effect params over one shared set** — the motivation is per-effect defaults; a single shared
  param can't carry four different sensible defaults. `mix` stays shared for one consistent dry/wet feel
  across the suite.

**Next up:**
- The payoff: dial in the actual per-effect default values in the `XxxParameters` PODs — this is where
  the deferred by-ear tuning (Vibe stagger/`ASYM_K`, Phaser, Flanger) now naturally lands.
- Then the deferred **GUI polish pass**: custom `LookAndFeel`, per-effect panels, LFO visualiser; grey
  the Mix knob in Vibe's Vibrato mode.

**Open questions / blockers:**
- Unchanged from Session 8: by-ear tuning still pending; still no `pluginval` / automated DSP test in-repo.

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
