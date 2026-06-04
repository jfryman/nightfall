# pict-gen

`pict-gen.py` emits the Phase 4.6 PICT 2 fixture slice from a small YAML-like
spec without adding a parser dependency. The supported operations are the first
`DrawPicture` validation set: version/header, NOP, RGB foreground/background
colors, line, frame/paint/erase/invert/fill rectangle, and end-of-picture.

Example:

```sh
python3 tools/pict-gen/pict-gen.py \
  tools/pict-gen/examples/basic.yaml \
  build/basic.pict \
  --coverage \
  --rez-output build/basic.r \
  --resource-id 129
```

Opcode coverage must grow here, in `core/QuickDrawPicture.cpp`, and in
`core/QuickDrawPicture.fuzz.cpp` together.
