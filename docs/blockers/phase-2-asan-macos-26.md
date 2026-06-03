# Phase 2 Blocker: ASan Runtime Deadlock on macOS 26.5

## Summary

Phase 2 requires ASan and UBSan to pass. UBSan passes locally, but ASan-linked
binaries that do meaningful instrumented work deadlock during ASan runtime
initialization on this macOS 26.5 host before Nightfall code executes.

## Trigger

- Gate: `scripts/ci/sanitizers.sh`
- Required profile: ASan
- Host: macOS 26.5 on Apple Silicon

## Evidence

- Xcode clang ASan hangs.
- Homebrew LLVM clang ASan hangs.
- Homebrew GCC/libasan hangs.
- Minimal probes reproduce the issue for allocator, global, and stack
  instrumentation.
- A live sample of the stuck Nightfall ASan smoke binary shows re-entry through
  `libclang_rt.asan_osx_dynamic.dylib` initialization and
  `__sanitizer::StaticSpinMutex::LockSlow`.
- Mozilla Bug 2037587 reports the same macOS 26 ASan stack and says the fix is
  an LLVM compiler-rt change:
  `https://bugzilla.mozilla.org/show_bug.cgi?id=2037587`

## Options

1. Recommended option: run ASan on a host/toolchain containing the upstream
   compiler-rt fix, then keep ASan required-pass.
2. Alternative: temporarily mark ASan best-effort for macOS 26.5 in the decisions
   log, with UBSan required-pass locally and ASan required again once the
   toolchain is fixed.
3. Alternative: build or install a patched compiler-rt locally and point the ASan
   profile at that runtime.

## Recommendation

Use option 1 if another macOS/Xcode/LLVM host is available soon. Use option 2 if
we want to continue Phase 2/3 work on this machine today without carrying a
known false-red required gate.

## Adjudication

**Decision:** pending
**Rationale:** pending
**Decisions-log entry:** pending
**Resume:** no
