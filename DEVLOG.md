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
