# M68KRuntime

`M68KRuntime` is the minimal Musashi-backed execution wrapper needed to
reconcile the missing real-module runner required by the Phase 5 checkpoint.

It owns a bounded emulated memory buffer, seeds the 68000 reset vectors, loads a
single 68k program image, and uses Musashi's instruction hook to record A-line
Toolbox calls before Musashi raises the 1010 exception. The wrapper currently
classifies trap words already implemented by Phases 4 and 5, stops on
unimplemented traps, and stops cleanly on `RTS`.

This is intentionally a headless validation runner. It does not add platform
portability machinery and it does not load real modules in CI.
