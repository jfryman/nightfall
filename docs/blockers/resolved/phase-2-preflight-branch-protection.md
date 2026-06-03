# Phase 2 Blocker: Branch Protection Required Checks Missing

## Summary

Preflight now fails because `main` is protected but has zero required status
checks configured. The Nightfall loop depends on self-merge-on-green being
mechanically enforced by branch protection, so Phase 2 work must halt until the
required gate set is configured.

## Trigger

- Gate, contradiction, or provisioning issue: `scripts/preflight.sh` section 5,
  merge gate.
- Why it cannot be worked around safely: the merge-policy decision requires
  branch protection with required checks so a red merge is mechanically
  impossible. Continuing while required checks are absent would turn the gate
  from enforcement into agent honesty, which contradicts the plan and
  `docs/autonomous-operation.md`.

## Options

1. Recommended option: configure the protected `main` branch required status
   checks to match the full gate set in `docs/preflight.md`, then re-run
   `scripts/preflight.sh`.
2. Alternative: explicitly adjudicate `ci-mode=local` in
   `docs/decisions-log.md` and update the preflight/merge policy documents to
   remove the GitHub Actions branch-protection dependency.
3. Alternative: pause Phase 2 until the self-hosted `smugglebook` runner and
   required checks can be configured together.

## Recommendation

Use option 1. It preserves the already-recorded self-merge-on-green decision and
keeps CI/branch protection as the mechanical safety rail for autonomous work.

## Evidence

- Relevant command or check: `scripts/preflight.sh`
- Relevant file or log:

```text
5. Merge gate  (ci-mode: actions)
  FAIL  main protected but 0 required checks
        -> add the gate set to required status checks (else self-merge-on-green is not enforced)
```

## Adjudication

**Decision:** Configure branch protection to require the `local-gates` status
check and add `.github/workflows/nightfall-ci.yml` to run
`scripts/ci/run-local.sh` on the self-hosted macOS runner.
**Rationale:** This restores the mechanical self-merge-on-green gate required by
the plan. The preflight merge gate now passes with one required status check.
**Decisions-log entry:** Not a new project decision; this applies the existing
self-merge-on-green decision.
**Resume:** yes
