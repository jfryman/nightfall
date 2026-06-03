# nightfall sanitizer smoke

`nightfall.sanitizer.cpp` is a narrow sanitizer smoke harness for
`nightfall_core`.

It avoids doctest and focuses on context creation, A-line trap registration,
fixture execution, and cleanup so sanitizer builds exercise core behavior without
depending on the regular test framework startup path.
