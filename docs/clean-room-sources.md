# Clean-Room Sources

Record the public specifications consulted before implementing any trap, host
routine, or emulation behavior.

Rules:

- Cite before implementation.
- Prefer specific page, section, or chapter references.
- Keep entries scoped to the subsystem they justify.
- If any output appears influenced by a forbidden source, stop and follow
  `docs/contamination-response.md`.

## Bootstrap status

## Trap Dispatch

- **Source:** Apple Computer, *A/UX Toolbox: Macintosh ROM Interface*, Appendix C
  "Implementation Notes", "A-line traps", pages C-3 to C-5, preserved at
  Bitsavers:
  `https://bitsavers.computerhistory.org/pdf/apple/mac/a_ux/aux_3.0/AUX_3.0_AUX_Toolbox_Macintosh_ROM_Interface_1992.pdf`
- **Consulted for:** The limited bootstrap facts that A-line traps are MC680x0
  opcodes in the range `0xA000` to `0xAFFF`, and that Macintosh environments use
  dispatch tables to route A-line traps to ROM or patched/user routines.
- **Implemented from this source:** `nf_trap_is_aline`,
  `nf_context_register_trap`, and `nf_context_dispatch_trap` implement only a
  Nightfall host-handler dispatch scaffold. No Toolbox routine behavior is
  implemented from this source.

## 68k Fixture Runner

- **Source:** Motorola, *M68000 Family Programmer's Reference Manual*, "RTS
  Return from Subroutine", page 4-169, and instruction format summary pages
  8-13 to 8-14, hosted by NXP:
  `https://www.nxp.com/docs/en/reference-manual/M68000PRM.pdf`
- **Consulted for:** The `RTS` instruction format word used by bootstrap
  fixtures.
- **Implemented from this source:** `nf_context_execute_fixture` recognizes a
  big-endian stream of 16-bit words, dispatches A-line trap words through the
  Nightfall trap scaffold, and treats `RTS` as the stop marker for the bootstrap
  fixture runner. It does not implement stack effects, registers, or general
  MC68000 instruction semantics.
