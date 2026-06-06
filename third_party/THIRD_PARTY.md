# Third-Party Code

## Musashi

- Upstream: `https://github.com/kstenerud/Musashi`
- Pin: `313ebf1bd9f4d0d93341eb5ce21fd8a119e9dbdd`
- Path: `third_party/musashi/`
- License: MIT-style permission notice embedded in `third_party/musashi/readme.txt`
  and the copied source files.
- Notes: `m68kops.c` and `m68kops.h` were generated with `m68kmake` from the
  same pinned commit before vendoring. The generated tool binary is not
  committed.

## SoftFloat, Bundled With Musashi

- Upstream notice: John R. Hauser SoftFloat Release 2b, as bundled under
  `third_party/musashi/softfloat/`.
- Path: `third_party/musashi/softfloat/`
- License/notice: see `third_party/musashi/softfloat/README.txt` and retained
  source notices.
- Notes: Musashi references the bundled SoftFloat routines even when Nightfall
  configures only 68000 execution, so the bundled files are included as a
  transitive component of the pinned Musashi runtime.
