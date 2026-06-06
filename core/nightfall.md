# nightfall core context

This unit owns the bootstrap C ABI, context lifecycle, and the Phase 5 headless
module-loading backfill used by `tbtrace` and later macOS app wiring.

Contexts are allocated from a fixed static pool to keep the Phase 1/2 core
deterministic and avoid heap allocation in sanitizer smoke runs.

No 68k Toolbox trap or host routine behavior is implemented here yet, so there
are no Inside Macintosh citations attached to this file. Add specific public
source citations before implementing any trap behavior.

The trap dispatch scaffold uses only the public A-line trap facts recorded in
`docs/clean-room-sources.md`: A-line traps are MC680x0 opcodes in the range
`0xA000` to `0xAFFF`, and Macintosh environments route those trap words through
dispatch tables. The scaffold can register and dispatch host-provided handlers,
but it does not implement any Toolbox routine semantics.

`nightfall.fixture.s` contains a minimal A-line trap word fixture for the
dispatch scaffold. The unit test asserts the C ABI classification, unimplemented
dispatch result, handler registration, handler dispatch, and emitted trace
events.

`nf_context_execute_fixture` is a bootstrap fixture runner. It reads big-endian
16-bit words, dispatches A-line trap words, and stops on `RTS`. It intentionally
does not implement general 68k execution.

The Phase 5 module C ABI (`nf_module_load`, `nf_module_start`, `nf_advance`,
`nf_module_framebuffer`, and `nf_module_stop`) is a minimal headless backfill for
the plan's missing prior-phase runner/API surface. It reads a macOS resource
fork xattr, parses the fork with `ResourceFork`, loads `ADgm/0`, and executes the
bounded default entry through `M68KRuntime`. It deliberately does not implement
or infer After Dark lifecycle selector/message semantics; that work remains
blocked until clean provenance is supplied.

`nightfall.sanitizer.cpp` is a narrow sanitizer smoke harness. It avoids doctest
because doctest plus ASan can deadlock during ASan runtime initialization on this
macOS host before Nightfall code executes.
