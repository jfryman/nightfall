# Sanitizer Status

Phase 2 requires sanitizer hardening. Current local status:

- UBSan: required-pass and green via `scripts/ci/sanitizers.sh`.
- ASan: temporarily best-effort on this macOS 26.5 host by the 2026-06-03
  decision in `docs/decisions-log.md`. ASan-linked binaries that exercise
  allocation can deadlock during ASan runtime initialization before Nightfall code
  runs.
- TSan: best-effort by project decision.

## ASan Runtime Evidence

Xcode clang, Homebrew LLVM clang, and Homebrew GCC were tested. ASan can execute
a plain `int main(){return 0;}` binary, but ASan-linked binaries that exercise
allocator interception, global instrumentation, or stack instrumentation can
deadlock during ASan runtime initialization on this macOS 26.5 host before
Nightfall code runs.

A live sample of the stuck Nightfall ASan smoke binary showed the process
spinning inside `libclang_rt.asan_osx_dynamic.dylib` initialization:

`__asan::InitializeShadowMemory` -> `__sanitizer::MemoryRangeIsAvailable` ->
`__sanitizer::get_dyld_hdr` -> malloc interception -> ASan initialization
re-entry -> `__sanitizer::StaticSpinMutex::LockSlow`.

The local sanitizer profile therefore runs a dedicated non-doctest smoke
executable that links `nightfall_core` and exercises context creation, trap
registration, fixture execution, and cleanup. UBSan passes through that harness;
ASan currently times out and is reported as an adjudicated host warning so
unrelated Phase 2 gates continue to run. ASan remains required-pass again on a
host/toolchain containing the upstream compiler-rt fix.

## External Corroboration

Mozilla tracked the same macOS 26 ASan initialization deadlock in Bug 2037587:
"Firefox ASAN binaries using 100% CPU on macOS 26.4.1 with latest clang
toolchain." The public bug records the same stack shape through
`dyld_shared_cache_iterate_text_swift`, ASan initialization re-entry, and
`StaticSpinMutex::LockSlow`, and links the upstream LLVM fix they backported:
`https://bugzilla.mozilla.org/show_bug.cgi?id=2037587`
