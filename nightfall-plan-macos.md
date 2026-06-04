# Nightfall — After Dark Revival for macOS (v4, scoped)

## Scope statement (read this first)

This is the macOS-only scope. Earlier drafts targeted macOS + Linux + Windows; that scope was deliberately cut because the cross-platform machinery was the single largest source of maintenance burden and agent-execution risk, and contributed nothing to the actual goal (running real After Dark modules on the maintainer's primary OS).

What this means concretely:

- **macOS only.** No Linux. No Windows. Not "macOS first" — macOS only, possibly forever, and that is an acceptable permanent outcome.
- **Metal renderer directly.** No Vulkan, no MoltenVK. Native Metal.
- **Thin C ABI boundary preserved** — not for portability, but because the testable headless core is what makes the agentic development loop and observability pipeline work. The boundary is testability infrastructure that happens to also be a portability seam; the portability is irrelevant to the decision.
- **macOS-native everywhere above the boundary.** Swift, AppKit, SwiftUI, Metal. No abstraction layers for platforms that don't exist.
- **68k Mac modules** are the target. PowerPC support (Phase C) is kept as a documented someday-maybe, deprioritized. Windows module support is **cut entirely**.
- **macOS-only CI.** No cross-platform equivalence testing, no distro packaging, no Windows/Linux runners.

Everything else — the gating philosophy, clean-room discipline, observability pipeline, the 22 committed design decisions, whimsy, validation checkpoints — is preserved, because none of it depended on the cross-platform scope.

## Project philosophy

The surface goal is a working After Dark–compatible screensaver for modern macOS that runs original 68k After Dark module files. The deeper goal is establishing a repeatable pattern for agentic development on a long-running codebase: clean-room discipline, observability-first architecture, gated quality checks, and feedback loops tight enough that an agent (the maintainer uses one called "Pip") can be trusted with real work.

The screensaver is the excuse. The pattern is the product.

## Agent execution handbook

How agents work within this plan. Published as `docs/AGENT-GUIDE.md`; this section is canonical. New agents read it first.

### Task acceptance criteria

Every task has: a **timebox** (max elapsed days before completing or producing a blocker doc), a **binary-verifiable outcome** (a test that passes or fails, not "X works"), **required output artifacts** (code + test + fixture + source-citation doc, co-located), and a **definition of done** (all artifacts committed, CI green, phase-tracking file updated).

### Handoff protocol

Phases end with a merged PR that updates `docs/current-phase.md`, commits the phase's `docs/phase-N.M-validation.md` log (or blocker doc), regenerates `docs/toolbox-coverage.md` via `tbcover`, and updates `docs/decisions-log.md` if decisions were made. A phase "done except for the tracking file" is not done.

### Phase tracking

Live single-source-of-truth at `docs/current-phase.md`:

```markdown
# Nightfall — Current Phase
**Current phase:** 4.3
**Status:** In progress
**Started:** 2026-05-14
**Timebox:** 2 days (→ 2026-05-16)
**Next expected deliverable:** docs/phase-4.3-validation.md
**Agent(s) assigned:** <identifier>
**Last blocker:** none
**Recent completions:**
- Phase 4.2 completed 2026-05-12
- Phase 4.1 completed 2026-05-10
```

PR checklist bot fails any PR touching `core/` without a corresponding update. Stale tracking is a build failure.

### Trace pipeline usage

When a test fails, the first response is to query the trace buffer for the failing span, not to add print statements. CI attaches the trace window for failures as an artifact. Agent reads structured events, forms a hypothesis, fixes, re-runs. No `printf` debugging — add an `nf_trace_event` emission instead (subject to schema rules). Print statements in core code fail nflint. `nfdebug` consumes the same stream interactively for local work.

### Required code patterns

- Every Toolbox trap uses the `NF_TOOLBOX_TRAP(name, ...)` macro (registration + entry/exit trace + error conventions). Hand-rolled traps without it fail nflint.
- Every emulated-memory access in core code uses `EmuPtr<T>` (Q3). Raw pointer dereferences in `ToolboxShim/**` fail the build.
- Test, fixture, and citation files co-located with implementation, enforced by `tbcover`.
- Trace events conform to the published schema; changes require `SCHEMA-BREAK` commit tag.

### STOP gate blocker docs

When a STOP condition triggers, produce `docs/blockers/<phase>-<short-description>.md` per `docs/blockers/TEMPLATE.md` before halting. A STOP without a blocker doc is a stall, not a stop. Template:

```markdown
# Blocker: <Phase N.M> — <short description>
## What failed
<Factual. No speculation.>
## Evidence
- Trace window: <link>
- Commit: <hash>
- Reproduction: <exact steps, fixtures, seed>
## Hypotheses
Ranked, with confidence and evidence for/against.
## Suggested next actions
Three options with tradeoffs.
## Agent's recommendation
<Best option and why. Human not bound by it.>
```

### Decisions log

Decisions arising during execution go to `docs/decisions-log.md` immediately (date, context, decision, rationale, consequences). Agents commit decisions as made, never accumulate them for later.

### Meta-rule

When in doubt: the answer is in this plan, in `docs/AGENT-GUIDE.md`, or in a Q-decision. Agents follow conventions, don't invent them. Genuine uncovered edge case → blocker doc requesting guidance, not improvisation.

## Legal boundaries

- **OK:** Clean-room reimplementation of the After Dark plugin host API and Mac Toolbox from public sources (Inside Macintosh, old SDKs, observed behavior). APIs aren't copyrightable (*Google v. Oracle*, 2021). Precedent: Wine, DOSBox, ScummVM.
- **OK for personal use (Phase A):** Loading the maintainer's own legally-acquired `.AD` module files on the maintainer's own machines.
- **NOT OK ever:** Bundling Berkeley Systems module files, art, or code in any public distribution. Using "After Dark" as the product name. Reading leaked/decompiled Berkeley Systems sources, or GPL emulator internals (Basilisk II, SheepShaver, Wine, Executor, PearPC, qemu, Dolphin) for any subsystem being contributed.
- **License hygiene:** Engine targets MIT. No GPL/LGPL code copied or linked into the core. Musashi (MIT) is safe. `THIRD_PARTY.md` tracks every dependency and license.
- **Paper trail:** `docs/clean-room-sources.md` logs every documentation source consulted for each Toolbox call. Phase B's clean-room defense rests on this.

### Licensed module content

Berkeley Systems produced originals they owned (Flying Toasters, Fish!, Lawnmower Man, Bad Dog!, Starry Night, Warp!, Rat Race) and licensed editions (Star Trek, Simpsons, Looney Tunes, Disney, Far Side, etc.). Licensed editions have active rights holders who enforce aggressively.

Guardrails (Phase B relevant; Phase A is personal use and unaffected):
- No licensed-property names anywhere in public-facing surface (README, marketing, UI strings, module lists, commit messages on release branches).
- Module picker displays whatever name the `.AD` file's `ADif` resource contains — Nightfall echoes the user's own file, maintains no catalog.
- Marketing screenshots/video use only hand-authored fixtures or Berkeley originals. Never licensed content.
- No module-specific code paths. The engine never knows licensed modules exist as a category.
- BYO documentation references formats and media types, not titles.
- Positioning: Nightfall officially supports Berkeley originals; licensed editions "may work, untested, unendorsed."

nflint scans public-facing files for licensed-property names and fails the build on matches. Denylist at `scripts/ci/licensed-property-names.txt`, reviewed when rights-holder situations change.

## Module execution

After Dark 68k modules are compiled 68k machine code resources, not extractable assets. They must be *executed*, not decompiled. Decompile-and-rewrite would be O(modules × effort), legally a derivative work, and would lose the point ("relive the glory days" means running the actual thing). Emulation is O(1) once the emulator works, legally clean (executing what the user owns, like Wine), and faithful.

A `.AD` module is a Mac resource file containing:
- `ADgy`: 68k machine code (the module program)
- `ADif`: metadata (name, version, preferred tick rate)
- `PICT`, `snd `, `cicn`: art, sound, icons
- A selector-based entry point: host calls the module with `kInit`, `kBlank`, `kDraw`, `kClose`, `kSetUp`, `kAbout` and a parameter block containing a QuickDraw `GrafPort`

Execution requires:
1. **Resource fork reader** — parse resource forks, MacBinary, BinHex. macOS deprecated the Resource Manager; build standalone.
2. **68k CPU emulator** — vendor **Musashi** (C, MIT, mature, used in MAME).
3. **Mac Toolbox shim** — QuickDraw, Memory Manager, Resource Manager, Sound Manager, Random, TickCount, and After Dark host calls. ~40-80 routines cover most modules.
4. **QuickDraw-to-Metal bridge** — module draws into an offscreen `GrafPort` (emulated memory buffer with QuickDraw semantics); each frame uploads as a Metal texture.
5. **After Dark host API** — selector dispatch, sourced from observed behavior and the After Dark Developer's Kit if legally available.

Reference projects (study architecture, never copy code — all GPL/contaminating): Basilisk II, SheepShaver, Mini vMac, Executor, ScummVM (for the BYO-content distribution model).

## Architecture

```
┌─────────────────────────────────────────────────┐
│  macOS App Layer (Swift / AppKit / SwiftUI)     │
│  ┌─────────────────┐  ┌──────────────────────┐  │
│  │ Nightfall.saver │  │ Nightfall.app shell  │  │
│  │ (ScreenSaverView)│  │ (standalone window)  │  │
│  └────────┬────────┘  └──────────┬───────────┘  │
│           └──────────┬───────────┘               │
│              MetalRenderer (Swift)               │
│              PacingDriver (CVDisplayLink)        │
│              Input handling (NSEvent)            │
└──────────────────────┬───────────────────────────┘
                       │  thin C ABI (nightfall.h)
┌──────────────────────▼───────────────────────────┐
│  libnightfall — pure C++17, no AppKit, headless  │
│                                                  │
│  Public C ABI:                                   │
│   nf_context_create / destroy                    │
│   nf_module_load(ctx, path)                      │
│   nf_module_start(ctx, w, h)                     │
│   nf_advance(ctx, virtual_ticks)                 │
│   nf_module_framebuffer(ctx, &ptr,&w,&h,&stride) │
│   nf_module_stop(ctx)                            │
│   nf_context_set_random_seed(ctx, seed)          │
│   nf_config_* / nf_trace_* / nf_audio_*          │
│                                                  │
│  ModuleRuntime (lifecycle state machine)         │
│       │                                          │
│  ┌────▼─────┐   ┌──────────────┐                │
│  │ Musashi  │──▶│ ToolboxShim  │                │
│  │ 68k core │   │ QuickDraw,   │                │
│  │          │   │ MemMgr, etc. │                │
│  └──────────┘   └──────────────┘                │
│  Shared: ResourceFile, EmulatedHeap, PixMap,     │
│          AudioBuffer, ConfigStore, Trace         │
└──────────────────────────────────────────────────┘
```

**Why the C ABI boundary, restated for this scope:** not portability. The boundary is what lets `libnightfall` build and run *headless* — no AppKit, no window, no run loop. That headless core is what makes (a) the agent's test-fix-retry loop fast (milliseconds, not screensaver-boot), (b) the observability pipeline cleanly instrumentable, and (c) `nfdebug` able to attach to a pure emulator context. Tangle the emulator into AppKit and every test becomes an integration test and the agentic loop dies. The boundary is testability infrastructure. ~40 C functions, mostly already specified as Q-decisions.

**Above the boundary is fully macOS-native.** Swift, AppKit, SwiftUI, Metal. No platform abstraction. `MetalRenderer` is just a Metal renderer, not an abstract `Renderer` with one implementation. `PacingDriver` is `CVDisplayLink`, not an interface. If macOS-only ever changes (it won't), that's a refactor then, not speculative generality now.

## Delivery targets

Both a `.saver` bundle and a `.app` shell:

- **Nightfall.saver** — installs to `~/Library/Screen Savers/`, runs in System Settings → Screen Saver. The "real" screensaver experience.
- **Nightfall.app** — standalone app that runs a module fullscreen on demand. Useful for: testing without installing a saver, running Nightfall as a deliberate "show me toasters now" app, and as the natural home for the config UI and the About dialog.

Both link the same `libnightfall` through the same C ABI. The `.app` is the primary development target (faster iteration than installing a `.saver` each cycle); the `.saver` is validated before Phase A ship.

## macOS host specifics

- **macOS 14+ (Sonoma and later).** `ScreenSaver.framework` for the `.saver`; plain AppKit `NSWindow` for the `.app`.
- **Renderer:** Metal directly. `CAMetalLayer`-backed `NSView`. Framebuffer from the core uploads as an `MTLTexture`, drawn as a fullscreen quad with the scaling shader (Q6).
- **Frame pacing:** `CVDisplayLink`, synchronized with display refresh, stops when display sleeps (Q1).
- **Config UI:** SwiftUI. In the `.saver`, presented via `configureSheet`. In the `.app`, a normal settings window.
- **Sandboxing note:** macOS 14+ runs screensavers in a separate sandboxed `legacyScreenSaver` process. Modules load from `~/Library/Application Support/Nightfall/Modules/` (the `.app` uses the same path). Entitlements: no network, file access scoped to the modules directory. The `.app` is less restricted and is the easier debugging surface.
- **Multi-display:** each display gets its own `ScreenSaverView` / window and its own `nf_context` (Q14). Mirror mode default, Independent available.

## Testing and gating infrastructure (first-class, Phase 0)

Guidelines decay. These are executable checks that fail CI. All macOS-only — no cross-platform machinery.

**`nflint`** — clean-room and hygiene enforcement:
- Forbidden-source scan: denylist of distinctive identifiers/comments from Basilisk II, SheepShaver, Wine, Executor, PearPC, qemu, Dolphin. Match fails build.
- GPL-header scan in `core/`.
- Clean-room attribution: every Toolbox shim file needs a `// Source:` header referencing `docs/clean-room-sources.md`.
- Binary-content scan: no `.AD`/`.rsrc`/MacBinary files outside `core/tests/fixtures/`.
- License compatibility against `THIRD_PARTY.md`.
- Licensed-property-names scan in public-facing files.
- `NF_TOOLBOX_TRAP` macro usage enforcement; `printf`-in-core ban.

**`tbcover`** — coverage tracker:
- Generated from directory structure, not hand-maintained. Manual edits to `docs/toolbox-coverage.md` rejected.
- Implementation file without co-located `.test.cpp` → fail. Without `.fixture.s` → fail (unless declared multi-trap in `FIXTURE-MAP.md`). Without `.md` citation → fail.
- Test file with only framebuffer assertions and no state assertion → fail.
- Trace-event coverage tracked alongside implementation coverage.

**`abi-guard`** — `nightfall.h` stability via `abidiff`. Breaking change without `ABI-BREAK:` commit tag → fail.

**`trace-schema-guard`** — observability schema stability. Rename/remove without `SCHEMA-BREAK:` tag → fail.

**`depcheck` / `toolcheck`** — dependency and toolchain pinning validation against `third_party/VERSIONS.toml`.

**`fixture-builder`** — assembles hand-written 68k `.AD` fixtures via vasm. Our code, committed, CI-only (no real modules in CI ever).

**`golden-frame-diff`** — fixture rendered, framebuffer hashed, compared to committed golden. `GOLDEN-UPDATE:` tag + review to regenerate.

**Sanitizer matrix** — ASan, UBSan, TSan on the core. Required passes.

**Structure-aware fuzzing** — `fuzz-pict`, `fuzz-resource-fork`, `fuzz-heap`, `fuzz-macbinary`. Co-located with the code they fuzz. 1M iterations/PR, 100M nightly.

**`app-smoke`** — the `.app` launches with a fixture module, renders 5 seconds, exits cleanly on input, first-frame hash matches golden. (Replaces the old cross-platform `host-smoke`; one platform, one host-pair.)

**PR checklist bot** — enforces `docs/pr-checklist.md`, AGENT-GUIDE patterns, `docs/current-phase.md` freshness, clean-room items.

CI runs on macOS runners only. No Linux/Windows jobs. No cross-platform equivalence (there's one platform). This removes an entire class of CI complexity and runner cost.

## Validation checkpoints (project-wide pattern)

Every phase relying on a bet has a checkpoint with a time box, a decision table, named STOP conditions, and a written artifact (`docs/phase-N.M-validation.md`). STOP is a legitimate outcome — the agent recognizing the plan needs revision and surfacing it, not a failure to avoid. STOP requires a blocker doc.

Checkpoints embedded: Phase 4 (color-depth/QuickDraw, Q7 gate), Phase 5 (Flying Toasters end-to-end), Phase 8 (per-module + phase-level), Phase 11 (Sound Manager scope), Phase C if undertaken (staged PPC milestones).

## The 22 committed design decisions

These were decided through extended deliberation and remain binding. Summarized; full rationale preserved in `docs/decisions-log.md` (ported from prior planning). Cross-platform-specific portions are struck where they no longer apply.

**Q1. Timing model.** Virtual clock. Core exposes `nf_advance(ctx, virtual_ticks)`; no wall-clock awareness. `TickCount()` reads a virtual counter (64-bit internal, truncated to 32-bit for modules). Pacing lives in the macOS host via `CVDisplayLink`. Adaptive tick rate to module's `ADif` preferred rate. Low-power/reduce-motion honored. Zero CPU when display blanked. *(Cross-platform pacing drivers struck; CVDisplayLink only.)*

**Q2. RNG determinism.** Emulate the Mac LCG faithfully in low memory `$0156`. Host-entropy seed at context creation; `nf_context_set_random_seed` for tests. Module writes to `$0156` honored.

**Q3. Endianness.** `EmuPtr<T>` wrapper for all emulated-memory access. Byte-swap + bounds-check + alignment + ASan poisoning. Built on Musashi's read/write callbacks. nflint bans raw pointer deref in `ToolboxShim/**`. (Still required — 68k is big-endian, Apple Silicon is little-endian.)

**Q4. Low-memory globals.** Minimum viable set (~20 globals) seeded from Inside Macintosh, demand-driven expansion. `docs/lowmemory-coverage.md` tracks. Per-global IM citation.

**Q5. Resolution.** Fixed 640×480×32-bit ARGB default, upscaled at the Metal layer. Per-module advanced override. Performance instrumentation from Phase 4.

**Q6. Scaling filter.** Three modes — CRT (default, vendored MIT GLSL-CRT adapted to Metal shading language), Sharp (nearest-neighbor integer-scaled), Smooth (bilinear). *(GLSL-CRT now ported to Metal MSL instead of Vulkan SPIR-V; ~same effort, documented in `third_party/glsl-crt/nightfall-changes.md`.)*

**Q7. Color depth.** 32-bit direct color only in Phase 4, mandatory validation checkpoint, Phase 4.5 fallback (8-bit indexed) ready if the bet fails. Decision table governs proceed/insert-4.5/STOP.

**Q8. Aspect ratio.** Letterbox 4:3 centered. Black letterboxes (genuinely off on OLED). CRT mode letterbox eligible for thematic cabinet treatment post-Phase-A. OLED-safety elevated to design principle.

**Q9. Input.** Cursor position forwarded via `$0830` (event-driven from `NSEvent`, no polling). Buttons/keys never forwarded (reserved for dismissal). Preview mode disables dismissal. *(macOS NSEvent only; cross-platform input plumbing struck.)*

**Q10. Lifecycle state machine.** Explicit named states (Unloaded→Loaded→Active→Blanked→Running→Closed). `kInit`/`kClose` always paired. `kBlank` once after `kInit`. Dismissal during `kDraw` lets the frame complete. Documented in `docs/module-loading-abi.md`.

**Q11. Module configuration.** Phase A: stub Dialog Manager with "user cancelled" returns (~3 days). Modules run with defaults. Nightfall config UI is global-only (module selection, randomizer, scaling, audio). Phase B: opportunistic per-module PREF sniffing. Full Dialog Manager deferred indefinitely.

**Q12. State persistence.** None. Exact fidelity — `kClose` on stop, `kInit` on start, fresh RNG seed per session (per Q2) so each session is unique but starts fresh. No snapshot/restore.

**Q13. Crash containment.** Catch at emulator boundary, log locally, rotate to next module. Crash-storm detection → Safe Mode (native non-emulated modules: Starfield, Bouncing Logo, Clock, Solid Dim). Architectural containment: emulated memory is one `std::vector<uint8_t>`, all access via `EmuPtr<T>`, per-module contexts, continuous fuzzing, sanitizers. Safe Mode also user-selectable. Crash logs local; Phase B optional Sentry per Q15.

**Q14. Multi-monitor.** One `nf_context` per display. Mirror (default) / Independent. Per-display `CVDisplayLink`. Hotplug via `NSApplicationDidChangeScreenParametersNotification`. *(macOS display handling only.)*

**Q15. Observability pipeline.** Structured trace events from every layer; pluggable sinks (null/ring/JSONL/agent-stream; Sentry in Phase B). Schema published at `core/include/nightfall/trace-schema.json`, versioned with `SCHEMA-BREAK` gate. `nfdebug` is a trace-stream consumer, not a separate tool. Every Toolbox call emits `trap_call`/`trap_return` via `NF_TOOLBOX_TRAP`. Crash attaches last 1000 ring-buffer events. Phase B: opt-in Sentry ingestion, notable clusters graduate to GitHub issues, agent-parseable issue format for autonomous triage.

**Q16. PICT authoring.** `tools/pict-gen/` — YAML-described PICT 2 emission for fixtures. Opcode coverage grows with `DrawPicture` implementation. Optional decoder later for diagnostics.

**Q17. Config format.** JSONC via `nlohmann::json`. Additive-within-version; breaking changes need `SCHEMA-BREAK` + migration. Per-module prefs in separate files. `ConfigStore` isolates the format. Config at `~/Library/Application Support/Nightfall/config.jsonc`.

**Q18. Updates.** Polite weekly check against GitHub Releases API, manual download, no in-place auto-update. CD pipeline produces signed artifacts (`.saver` + `.app` in a DMG) to GitHub Releases on tag. *(macOS signing only — no Authenticode, no distro packaging.)*

**Q19. Signing keys.** AWS KMS via GitHub OIDC federation, Terraform-managed (`nightfall-infra/` repo), maintainer's personal AWS account with documented migration path. *(Apple Developer ID only — Windows Authenticode and Linux GPG struck.)* Phase A: local signing. Phase B prep: KMS + Developer ID with cloud-HSM key.

**Q20. Accessibility/safety.** Honors OS reduce-motion. Photosensitive warning labels in module picker (`docs/module-warnings.md`). Audio off by default, opt-in, quiet mode, startup delay. Dim display, night mode, OLED-safe mode. Safe-fixture preview in settings. `docs/accessibility.md` for Phase B.

**Q21. Telemetry.** None, ever. Deliberate stance. Only conditional outbound: opt-in crash reports (Q15) to Sentry in Phase B. Only unconditional outbound: weekly GitHub Releases check (Q18). Optional one-way donation link. `docs/privacy.md` states it plainly. Verifiable via network monitoring.

**Q22. Compatibility scope.** Aspirational full compatibility with the After Dark 2.x-3.x 68k module format. Any standard-conforming module that misbehaves is a bug, not "out of scope." Seven iconic modules are the validation benchmark, not the scope boundary. `docs/known-divergences.md` replaces a "supported list." *(In-scope format unchanged; PowerPC is Phase C someday-maybe; Windows formats cut entirely.)*

## Agent execution contracts

**STOP gate output contract.** Every STOP requires `docs/blockers/<phase>-<issue>.md` per template before halting. Committed, not hidden — audit trail, human context, future-agent training data, Phase B legal evidence. STOP + blocker doc = success at the gate. STOP without it = contract failure.

**File co-location.** Tests/fixtures/citations next to implementation:
```
core/src/mac68k/ToolboxShim/QuickDraw/
  CopyBits.cpp  CopyBits.test.cpp  CopyBits.fixture.s  CopyBits.md  CopyBits.fuzz.cpp
```
`tbcover` fails CI on missing companions. Coverage doc generated from directory state; manual edits rejected. Removes "remember to update docs" from the agent's task list.

**Concrete trace schema.** Published Phase 0. Event kinds: `trap_call`, `trap_return`, `memory_read`, `memory_write`, `allocation`, `deallocation`, `state_transition`, `selector_call`, `crash`, `error`, `performance_sample`, `breakpoint_hit`, `credits`. `NF_TOOLBOX_TRAP` macro auto-emits trap events; hand-written traps without it fail. Crash path attaches last 1000 ring-buffer events (tested by deliberate-crash fixtures).

**State-level tests.** Every trap test needs ≥1 state assertion beyond framebuffer hash (`AssertMemoryUnchanged`, `AssertHeapIntegrity`, `AssertPortUnchanged`, etc.). `tbcover` fails framebuffer-only tests. Prevents agents Goodharting golden frames with wrong internal state.

**Structure-aware fuzzing.** `fuzz-pict` (PICT grammar mutations), `fuzz-resource-fork`, `fuzz-heap`, `fuzz-macbinary`. Co-located. 1M/PR, 100M nightly. Crashes/sanitizer hits fail build.

**Deterministic mode.** `NF_DETERMINISTIC=1` / `--deterministic`. Fixed seed, virtual clock from tick 0, whimsy suppressed, rate limiters off, fixed date. CI always uses it. Agents default to it for debugging. One global flag consulted at decision points; the virtual clock (Q1) and seeded RNG (Q2) already make this structural.

**Dependency/toolchain pinning.** `third_party/VERSIONS.toml` — exact commits/versions for deps and toolchain (Musashi, nlohmann-json, vasm, abidiff, the Metal shader compiler invocation). `depcheck`/`toolcheck` validate. `DEP-UPDATE:` tag for changes.

**Performance regression gating.** Microbenchmarks vs rolling-30-day baseline on the macOS CI runner: per-tick execution, per-trap latency, framebuffer upload, Metal present, end-to-end virtual-tick-to-present. >10% regression fails PR, 5-10% warns. Append-only `benchmarks/history.jsonl`.

**Core/app boundary enforcement.** `core/` has zero AppKit/Foundation/Metal includes — pure C++17 + the C ABI header. The Swift app links only `libnightfall`'s C ABI, never C++ internals. `boundary-check` CI step fails on violation. This is the testability guarantee, mechanically enforced. (Replaces the old "host thinness" rule; same idea, single host.)

## Repository layout

```
nightfall/
  CMakeLists.txt                     # builds libnightfall (C++)
  Nightfall.xcodeproj                # builds .saver + .app (Swift), links libnightfall

  core/                              # libnightfall — pure C++17, headless, testable
    CMakeLists.txt
    include/nightfall/
      nightfall.h                    # THE C ABI
      trace.h
      trace-schema.json
    src/
      api/                           # C ABI implementation
      mac68k/
        Musashi/                     # vendored MIT
        ToolboxShim/
          QuickDraw/                 # co-located impl/test/fixture/doc/fuzz
          MemoryManager/
          ResourceManager/
          SoundManager/
          DialogManager/             # Phase A stubs
          Misc/
          AfterDarkHostAPI/
        TrapDispatcher.cpp
        Memory68k.cpp
        EmuPtr.h
        LowMemory.{h,cpp}
        ModuleRuntime.cpp
      macppc/                        # Phase C someday-maybe; empty placeholder + README
      shared/
        ResourceFile.cpp  MacBinary.cpp  BinHex.cpp
        PixMap.cpp  AudioBuffer.cpp  ConfigStore.cpp
      trace/                         # sinks: null, ring, jsonl, agent-stream
      crash/                         # ExceptionHandling, StormDetector
      telemetry/                     # profiling counters (compile-time opt-in)
    tests/
      toolbox/  integration/  fixtures/  fuzz/

  app/                               # macOS-native, Swift
    Nightfall.saver/
      NightfallSaverView.swift       # ScreenSaverView subclass
      Info.plist
    Nightfall.app/
      AppDelegate.swift
      FullscreenWindow.swift
    Shared/
      NightfallCore.swift            # thin Swift wrapper over the C ABI
      MetalRenderer.swift            # Metal: upload framebuffer, scaling shaders
      PacingDriver.swift             # CVDisplayLink
      InputHandler.swift             # NSEvent → cursor pos / dismissal
      Config/                        # SwiftUI settings
      SafeMode/                      # native non-emulated fallback modules
      Shaders/
        blit_sharp.metal  blit_smooth.metal  blit_crt.metal
      About/                         # Trinitron-framed about box (whimsy)

  tools/
    adinspect/                       # dump .AD resources
    tbtrace/                         # run module with Toolbox logging
    pict-gen/                        # YAML → PICT 2 for fixtures
    nfdebug/                         # interactive trace-stream debugger

  docs/
    AGENT-GUIDE.md                   # canonical agent handbook
    current-phase.md                 # live phase tracker
    decisions-log.md
    risk-register.md
    architecture.md
    module-loading-abi.md
    toolbox-coverage.md              # generated
    lowmemory-coverage.md
    clean-room-sources.md
    contamination-response.md
    contamination-log.md             # hopefully stays empty
    known-divergences.md
    module-warnings.md
    accessibility.md                 # Phase B
    privacy.md                       # Phase B
    fixture-authoring.md
    performance-notes.md
    pr-checklist.md
    phase-b-checklist.md
    blockers/
      TEMPLATE.md
    attestations/
    analysis/

  third_party/
    musashi/  nlohmann-json/  glsl-crt/
    THIRD_PARTY.md  VERSIONS.toml

  scripts/
    bootstrap.sh
    ci/
      build.sh                       # macOS build (lib + Xcode)
      nflint.py  tbcover.py  abi-guard.sh  trace-schema-guard.py
      depcheck.py  toolcheck.sh  boundary-check.py
      golden-frame-diff.py  pr-checklist-bot.py
    fixtures/build.sh
    release/                         # DMG packaging, notarization

  .github/
    workflows/
      ci.yml                         # macOS only: build, test, sanitizers, gates
      ci-fuzz.yml                    # nightly fuzzing
      release.yml                    # tag → signed DMG → GitHub Release
    pull_request_template.md

  README.md  LICENSE  LICENSING.md  CONTRIBUTING.md  CHANGELOG.md
```

No `hosts/linux/`, no `hosts/windows/`, no Vulkan, no MoltenVK, no distro packaging, no cross-platform CI. The `macppc/` directory exists only as an empty placeholder with a README explaining it's a documented someday-maybe (Phase C), so the architecture has an obvious home for it without carrying any speculative code.

## Build order

Phases sized for focused work. Times are working-day estimates; evenings-and-weekends cadence stretches elapsed time considerably (and that's fine — there's a newborn).

### Phase 0 — Scaffolding + gating + observability (7-8 days)

CMake builds `libnightfall` as a static lib. Xcode project builds skeleton `.saver` and `.app` linking it. `third_party/` vendored (Musashi, nlohmann-json, glsl-crt) with `VERSIONS.toml` pinning.

Gating deliverables: `nflint`, `tbcover`, `abi-guard`, `trace-schema-guard`, `depcheck`/`toolcheck`, `boundary-check`, `pr-checklist-bot`, sanitizer matrix, all on macOS CI. `docs/AGENT-GUIDE.md`, `docs/current-phase.md`, `docs/decisions-log.md`, `docs/risk-register.md`, `docs/blockers/TEMPLATE.md`, `CONTRIBUTING.md` with attestation flow. Trace primitives (`nf_trace_event`, `nf_trace_span`, null/ring/JSONL sinks), `trace-schema.json`. CD pipeline scaffold (`release.yml` stub → draft DMG on tag).

Early visual smoke test (½ day): the `.app` renders a fullscreen quad cycling red→green→blue via Metal. Golden PNG committed. `app-smoke` CI test. Proves the core→C ABI→Swift→Metal→display path before any Toolbox work.

**Sign-off:** trivial-comment PR passes all gates; forbidden-string PR fails nflint; untagged-ABI-break PR fails abi-guard; visual smoke renders the pattern. Demonstrate all four.

### Phase 0.5 — Resolve open decisions (½ day)

Product name (trademark search; "Nightfall" is placeholder), legal-review scheduling plan. Committed to `docs/decisions-log.md`. (Config format, audio lib, contributor policy already decided via Q-decisions. Cross-platform items moot.)

### Phase 1 — Resource fork parser + adinspect + fixture pipeline (5 days)

`ResourceFile.cpp` (resource forks, MacBinary, BinHex). `tools/adinspect` dumps `.AD` resources. `tools/pict-gen` initial (rectangles, lines, basic colors, RF embedding). First committed fixture (hand-written 68k "fill-red"). First golden test. Parser fuzzed 1M iterations clean.

### Phase 2 — Musashi + C ABI baseline (5 days)

Vendor Musashi, C++ wrapper. Freeze `nightfall.h` with abidiff baseline; per-function docs (stability, thread-safety, ownership). `EmuPtr<T>` defined. Trivial 68k program executes. The C ABI is now a tested, frozen contract — take time here, retrofitting it is expensive. C ABI unit tests run headless.

### Phase 3 — Memory model + trap dispatch + fuzz harness (6 days)

Memory map (code/heap/stack/lomem). A-trap callback → C++ dispatcher, all traps stubbed-logging. `docs/toolbox-coverage.md` structure (generated). `fuzz-traps` online against stubs. Load Flying Toasters, capture the trap log → `docs/analysis/flying-toasters-trap-log.md`. This log is Phase 4's priority list.

### Phase 4 — QuickDraw subset (atomic subphases)

Each subphase: 1-3 day timebox, binary-verifiable, own fixture, own `docs/phase-4.N-validation.md`. Standing gates apply (co-located test/fixture/citation, `NF_TOOLBOX_TRAP`, state assertions, sanitizers, fuzzing).

- **4.1 Init + port state (1-2d):** `_InitGraf`, `SetPort`, `GetPort`, `OpenPort`, `ClosePort`, `SetPortBits`. State-assert port + QD lomem globals.
- **4.2 Rect ops (1-2d):** `FillRect`, `EraseRect`, `FrameRect`, `PaintRect`, `InvertRect`, `BackColor`, `ForeColor`, `RGBBackColor`, `RGBForeColor`. Golden + pixel-level state.
- **4.3 Lines (1d):** `MoveTo`, `Move`, `LineTo`, `Line`, pen state. Golden + scanline state.
- **4.4 Regions (2d):** `NewRgn`/`DisposeRgn`/`SetRectRgn`/`RectRgn`/`OffsetRgn`/`CopyRgn`/`UnionRgn`/`SectRgn`/`DiffRgn`/`EmptyRgn`/`EqualRgn`/`PtInRgn`/`SetClip`/`GetClip`. Region-structure state assertions.
- **4.5 CopyBits/CopyMask (2-3d):** plus `PixMap`/`BitMap`. Multiple src/dst/mode combos. Memory state assertions.
- **4.6 PICT interpreter (2-3d):** `DrawPicture` + PICT 2 opcodes (coverage = pict-gen coverage). `fuzz-pict` mandatory, 1M/PR.

**Phase 4 checkpoint:** Q7 color-depth validation against all seven modules. Decision table → proceed / insert Phase 4.5+ (8-bit indexed, ~1 week) / STOP with blocker doc.

### Phase 5 — Memory/Resource/Sound-stub/misc Toolbox (4 days)

`NewHandle`/`DisposeHandle`/`GetHandleSize`/`SetHandleSize`/`HLock`/`HUnlock`/`HGetState`/`HSetState`, `NewPtr`/`DisposePtr`, `GetResource`/`ReleaseResource`/`GetResourceSizeOnDisk`/`DetachResource`, `TickCount`, `GetDateTime`, `Delay`, `LMGetTicks`. Sound Manager stubbed silent.

**Checkpoint:** Flying Toasters runs end-to-end 10 minutes, zero unimplemented-trap warnings. Decision table → proceed / fix surfaced traps ≤2d / STOP.

### Phase 6 — Metal renderer + headless integration (5 days)

`MetalRenderer.swift`: `CAMetalLayer`, framebuffer→`MTLTexture`, fullscreen quad. Three scaling shaders in Metal Shading Language (`blit_sharp/smooth/crt.metal`; CRT adapted from vendored GLSL-CRT, diff documented). Headless render path for CI golden tests (offscreen Metal). `golden-frame-diff` fully online.

*(No cross-platform equivalence — one platform. This phase is meaningfully smaller than the old Vulkan version: no MoltenVK, no SPIR-V toolchain, no surface abstraction. Native Metal is well-trodden.)*

### Phase 7 — Wire the `.app` (4 days)

`Nightfall.app` runs Flying Toasters fullscreen via the C ABI + `MetalRenderer` + `CVDisplayLink` + `InputHandler`. `app-smoke` CI test. **First visible milestone** — the actual 1991 binary running on Apple Silicon. (The `.saver` comes in Phase 12; the `.app` is the faster iteration target and proves the whole stack.)

### Phase 8 — Validate remaining six modules (per-module subphases)

One subphase each, 2-day timebox (extends to 4 with approval; STOP beyond): Fish!, Lawnmower Man, Bad Dog!, Starry Night, Warp!, Rat Race. Each: load, run 60 virtual sec, classify works/partial/broken, diagnose via trace pipeline, implement missing traps with standard discipline, re-validate, log `docs/phase-8.N-validation.md`.

**Phase 8 checkpoint:** 6/6 → proceed; 4-5/6 → proceed, divergences to `known-divergences.md`; 2-3/6 → STOP (systemic); 0-1/6 → STOP (earlier-phase miss).

### Phase 9 — Sound Manager (4 days)

Emulated Sound Manager → CoreAudio directly (no cross-platform audio lib needed; macOS-only means `AVAudioEngine`/`AudioToolbox`). `SndNewChannel`, `SndPlay`, `SndDoCommand`, 'snd ' format 1/2.

**Checkpoint:** Bad Dog! barks; audio works for modules that have it. Decision table → complete / document silent-failures / extend ≤2d / STOP if scope balloons (reduce to minimum viable subset).

*(Renumbered from old Phase 11; old Phases 9/10 were Linux/Windows hosts, now cut. miniaudio dependency dropped — CoreAudio directly is simpler for one platform.)*

### Phase 10 — Polish + ship Phase A (5 days)

Build the `.saver` (the `.app` already works; `.saver` is the same core + renderer in a `ScreenSaverView`). Multi-display, preview-mode optimizations, randomizer, sensible defaults, config UI (global settings only per Q11), About dialog with Trinitron + whimsy. Install `.saver` to `~/Library/Screen Savers/`, install `.app` to `/Applications`. **Phase A complete.**

### Phase A total: ~48-52 working days (~10 weeks focused)

Down from ~82 days in the cross-platform plan. The cut isn't just "remove two host phases" — it's removing Vulkan/MoltenVK complexity, cross-platform equivalence testing, the portability abstraction tax across every phase, distro packaging, and three-runner CI. Evenings-and-weekends cadence: realistically 4-6 months elapsed, vs 6-9 for the cross-platform version.

---

### Phase B — Public release (1-2 weeks)

BYO onboarding UI, module-extraction docs (formats not titles), trademark review, Apple Developer ID + notarization via the Q19 KMS pipeline, DMG, GitHub Release, public repo with clean-room attestation, `docs/privacy.md`/`accessibility.md`, attorney consultation. Smaller than the cross-platform Phase B (no Authenticode, no distro submission, no multi-platform packaging).

### Phase C — PowerPC modules (someday-maybe, deprioritized)

Documented but not scheduled. If undertaken: staged subphases (C.1 PPC interpreter core → C.2 stub-trap ABI validation → C.3 CFM loader → C.4 minimal real module → C.5 AD 4.x validation), each with STOP gates, exactly as the cross-platform plan specified. `core/src/macppc/` exists as an empty placeholder so there's an obvious home. No commitment; the seven 68k iconic modules are the project's heart and PPC is bonus.

### Phase D — Windows modules

**Cut.** Not someday-maybe. Cut. If it's ever wanted it's a new project, not a deferred phase.

## Dependencies inventory

| Library | License | Purpose | Phase |
|---|---|---|---|
| Musashi | MIT | 68k emulator | 1 |
| nlohmann::json | MIT | JSONC config | 1 |
| glsl-crt (wessles) | MIT | CRT shader (adapted to Metal MSL) | 6 |

Dropped vs cross-platform plan: Volk, vk-bootstrap, MoltenVK (no Vulkan), miniaudio (CoreAudio directly), Qt6 (no Linux config UI), Dear ImGui (no Windows config UI), Unicorn (no Windows modules). Toolchain: vasm (fixtures), abidiff (ABI gate), Xcode's Metal compiler. All pinned in `VERSIONS.toml`.

Explicitly rejected (GPL/contaminating): Wine, Basilisk II, SheepShaver, PearPC, qemu, Dolphin.

## Testing strategy

- Per-Toolbox-call unit tests, known in/out, ≥1 state assertion each, run headless via `libnightfall`.
- Hand-authored fixture modules (our 68k, committed). No real modules in CI ever.
- Golden-frame tests: fixture at fixed seed + virtual tick → framebuffer hash → committed golden.
- Structure-aware fuzzing (pict/rf/heap/macbinary), 1M/PR + 100M nightly.
- `app-smoke`: the `.app` with a fixture, 5 sec, clean exit, golden first-frame.
- Manual: real modules on the maintainer's machines, not CI. Recorded videos for regression.
- Performance budget: 60 FPS at 4K on Apple Silicon; 30 FPS floor. Microbenchmark-gated.

All CI on macOS runners only. No equivalence testing (one platform). No cross-compilation.

## First agent checkpoint (Phases 0-3 complete)

1. Gating proven on macOS CI: three test PRs (clean passes, forbidden-string fails nflint, untagged-ABI-break fails abi-guard). Visual smoke renders red→green→blue in the `.app`.
2. `nightfall.h` frozen, abidiff baseline, headless C ABI tests pass.
3. `adinspect` round-trips a real `.AD`; fixture pipeline produces committed "fill-red".
4. `ResourceFile.cpp` survives 1M fuzz iterations clean.
5. Musashi executes arbitrary 68k.
6. Trap coverage doc generated, all stubs.
7. `fuzz-traps` nightly green.
8. Flying Toasters trap log committed → Phase 4 priority list.
9. Clean-room paper trail started; contamination log empty.
10. `docs/decisions-log.md` has Phase 0.5 outcomes; `docs/current-phase.md` accurate.

No Toolbox shim code yet — Phase 4 lands on a fully enforced, fully macOS-native, single-platform foundation.

## Whimsy

After Dark was fun. Deliberate, undocumented joy, preserved from the original plan, macOS-scoped:

- Safe Mode Starfield: 1-in-10000 tick chance of a tiny chrome toaster drifting through.
- Trurl the hamster: tribute to the Berkeley Systems hamster buried with the first Disney Collection box. Scurries across Safe Mode Clock/Bouncing Logo occasionally. README footnote: "In memory of Trurl."
- Date-aware: April 1 (module names backwards in picker), Oct 31 (ghost sprites in Starfield), Dec 25 (gift-wrapped toasters), project anniversary (thank-you splash).
- 3:00-3:03 AM: CRT mode does a subtle VHS tracking glitch every 30 sec. Documented nowhere.
- Cursor-approach: cursor lingering 30+ sec at a screen edge during Flying Toasters spawns a gold toaster that loops the cursor. Pure Nightfall invention, period-appropriate.
- Safe Mode crash-storm flavor text ("Sometimes the toasters just need a nap.").
- About dialog: Trinitron-framed; version-number clicked 7× plays the Mac startup chime.
- README: `# Feed the toaster before first run` shell comment that does nothing.
- Boot credits trace event (invisible unless inspecting traces); ASCII Happy Mac at debug trace level.
- Community: Safe Mode Bouncing Logo reads `<config-dir>/safe-mode-logo.png` if present.

Constraints: never references user data, never unexpected audio (the chime is explicit user action), never breaks observability/determinism, period-authentic over generic memes. `NF_DETERMINISTIC=1` suppresses all of it for CI/debugging. Each piece gets a `whimsy`-tagged tracking issue so future agents don't "fix" it as a bug.

## Risk register

| # | Risk | Likelihood | Impact | Mitigation |
|---|---|---|---|---|
| 1 | QuickDraw 32-bit bet fails | Medium | High | Q7 checkpoint; Phase 4.5 (8-bit) scoped & ready |
| 2 | Clean-room contamination | Low | Very high | nflint denylist + procedure + attestation; cooling-off + subsystem redo |
| 3 | Toolbox scope larger than estimated (long tail of modules) | Medium | Medium | Q22 aspirational posture; per-module STOP gates; seven iconic = benchmark not scope |
| 4 | `NF_TOOLBOX_TRAP` macro is the wrong abstraction, discovered mid-Phase-4 | Medium | Medium | Prototype it in Phase 2 against 3 representative subsystems before Phase 4 |
| 5 | Maintainer bandwidth (newborn, evenings-only) | High | Varies | Self-documenting plan; STOP gates prevent silent drift; hiatus is an acceptable outcome; scope cut to macOS specifically to reduce this |
| 6 | Legal challenge to Phase B BYO distribution | Low | High | Clean-room paper trail + attorney consult + conservative nominative-fair-use posture |

macOS-scoping directly retired three cross-platform risks from the prior register (cross-platform divergence, PPC scope explosion as a committed phase, Win16 surprise). The single biggest execution risk — too-large scope for the agent — is the one this rescope exists to address.

## Project infrastructure (cross-cutting)

- **Observability (Q15):** trace primitives Phase 0; `nfdebug` Phase 3; Sentry Phase B.
- **CI gating (Phase 0):** all gates, macOS runners only.
- **CD (Q18):** GitHub Actions tag→signed DMG→GitHub Release. Exercised continuously in Phase A with alpha tags.
- **Signing (Q19):** AWS KMS + Terraform + OIDC, Apple Developer ID only. Phase B prep.
- **Crash aggregation (Q15):** local Phase A; Sentry + GitHub issues Phase B.
- **Contributor onboarding:** `CONTRIBUTING.md` + attestation, before first external PR.

Standing infrastructure, not phase deliverables. Agents operate within these, extend them when a phase requires, never bypass for expedience.

## Clean-room contributor onboarding

`CONTRIBUTING.md` written Phase 0 (solo-maintainer attestation committed once at `docs/attestations/<id>.md`). Phase B expands to external-contributor flow: DCO sign-off + subsystem-scoped clean-room attestation via CLA bot. Attestation language: contributor affirms not having read forbidden sources (Berkeley Systems After Dark, Basilisk II, SheepShaver, Wine, Executor, PearPC, qemu, Dolphin) for the subsystem they contribute to; implementations derive solely from public specs; contamination triggers immediate notification + procedure; contributions MIT-compatible. Subsystem-scoped — contaminated on QuickDraw ≠ barred from the Metal renderer.

## Contamination response

`docs/contamination-response.md`. On accidental exposure to forbidden source: stop work on the affected subsystem, tag last clean commit, revert uncommitted work, 30-90 day cooling-off, ideally different person reimplements, log in `docs/contamination-log.md`. Paranoid for Phase A, essential for Phase B.
