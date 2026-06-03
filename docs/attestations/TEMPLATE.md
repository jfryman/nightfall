# Clean-Room Attestation (AI contributor) — TEMPLATE

Copy to `docs/attestations/<your-identifier>.md`, fill, and commit **before your
first code commit** (the bootstrap prompt requires this). One attestation per
contributing agent identity.

> **Why this differs from a human attestation.** The standard clean-room
> attestation asks the signer to affirm they "have not read" forbidden sources
> (Berkeley Systems After Dark; Basilisk II, SheepShaver, Wine, Executor, PearPC,
> qemu, Dolphin, or any GPL Mac/PPC emulator). An LLM agent cannot make that claim
> truthfully — those corpora are very likely in its training data, and it can
> surface contaminating code from parametric memory without "reading" anything in
> this session. So this attestation affirms *provenance of output*, not *absence
> of exposure*, and leans on the mechanisms that actually work against recall: the
> `nflint` denylist and mandatory in-session citation. **The maintainer holds the
> legal attestation of record; this is the agent's operational affirmation.** This
> language is to be reviewed with counsel before Phase B relies on it.

---

## Attestation

- **Contributor identifier:** `<agent id / handle>`
- **Model / version:** `<e.g. the executing model and version>`
- **Date:** `<YYYY-MM-DD>`
- **Subsystems this attestation covers:** `<e.g. QuickDraw, MemoryManager, … or "all Phase 0–3">`
  *(Clean-room scope is per-subsystem: contamination on one does not bar
  contribution to an unrelated one.)*

By committing this file I affirm, for the subsystems above:

1. **Provenance.** Every Toolbox routine, host-API call, and emulation behavior I
   implement derives **solely from public specifications I cite in-session** in
   `docs/clean-room-sources.md` — Inside Macintosh, Apple developer
   documentation, IEEE/industry standards, and the public references listed there.
   I do not reconstruct any implementation from recalled proprietary or
   GPL-licensed source.

2. **Citation before implementation.** I do not write a trap or routine until its
   `// Source:` citation to a specific Inside Macintosh page/section (or other
   cited public spec) is recorded. I understand `tbcover` rejects empty or
   placeholder citations.

3. **Recognition and flagging.** If I recognize any output of mine as originating
   from a forbidden source — or encounter forbidden source in tool output — I stop
   work on the affected subsystem immediately, follow
   `docs/contamination-response.md`, log it in `docs/contamination-log.md`, and
   produce a blocker doc. I do not "clean up" or paraphrase suspected
   contamination to make it pass; I surface it.

4. **Licensing.** My contributions are MIT-compatible. I do not copy or link
   GPL/LGPL code into the core. I do not ship licensed-property names in
   user-facing surfaces.

5. **Primary defense acknowledged.** I understand the `nflint` denylist is the
   primary safeguard against accidental parametric recall, that it is a tested
   first-class artifact, and that a denylist hit is a build failure I must
   investigate via the contamination procedure, not suppress.

6. **Scope of this affirmation.** This is an operational affirmation about how I
   produce output. It is not, and does not substitute for, the maintainer's legal
   attestation of record, nor for counsel review prior to public distribution
   (Phase B).

**Signed:** `<agent identifier>`
**Commit:** `<hash of the commit that introduces this file>`

---

*Maintainer note:* before Phase B, replace/extend this with counsel-reviewed
language and the human contributor's DCO sign-off + subsystem-scoped attestation
(see `nightfall-plan-macos.md` → "Clean-room contributor onboarding").
