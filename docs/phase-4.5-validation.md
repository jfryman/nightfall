# Phase 4.5 Validation — QuickDraw Bit Transfer

## Scope

Phase 4.5 implements the QuickDraw bit and pixel map transfer subset:

- `CopyBits`
- `CopyMask`

The implementation is co-located with its test, fixture, and citation file:

- `core/QuickDrawBitTransfer.cpp`
- `core/QuickDrawBitTransfer.test.cpp`
- `core/QuickDrawBitTransfer.fixture.s`
- `core/QuickDrawBitTransfer.md`

## Clean-Room Sources

Citations were recorded before implementation in `docs/clean-room-sources.md`,
section "QuickDraw Bit Transfer". The implementation uses Inside Macintosh for
copying images, `CopyBits`, `CopyMask`, source modes, mask-region clipping,
destination clipping, scaling behavior, 32-bit direct-color pixel-map modeling,
and public QuickDraw trap-word references.

## Assertions

- `CopyBits` with `srcCopy` copies modeled 32-bit pixels between ports.
- Modeled `PixMap` metadata is present on each graphics port and records 32-bit
  pixel depth.
- `CopyBits` applies `srcOr`, `srcXor`, and `srcBic` source modes.
- `CopyBits` scales source pixels into a larger destination rectangle with a
  deterministic nearest-neighbor model.
- `CopyBits` respects an optional mask region and the current port clipping
  region.
- `CopyMask` copies source pixels only where the modeled mask pixel is active.
- Missing ports and invalid mask geometry return `NF_ERROR_INVALID_ARGUMENT`.

## Local Validation

- PASS: `scripts/ci/run-local.sh`
