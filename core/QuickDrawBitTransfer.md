# QuickDraw bit transfer

This unit implements the Phase 4.5 modeled bit-transfer subset:

- `CopyBits`
- `CopyMask`

Clean-room citations are recorded before implementation in
`docs/clean-room-sources.md`, section "QuickDraw Bit Transfer".

The transfer model uses each modeled graphics port's bounded 32-bit pixel buffer
and 32-bit `PixMap` metadata. `CopyBits` supports nearest-neighbor scaling, the
basic source modes, optional mask-region clipping, and current-port clipping.
`CopyMask` copies source pixels only where the modeled mask pixel is active.
