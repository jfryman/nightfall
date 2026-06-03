# smoke fixture

Minimal 68k assembler smoke fixture for the local gate loop. It includes one
A-line trap word so the fixture set exercises the trap-word encoding path.

This does not model Toolbox behavior. It only proves the configured vasm-family
assembler can assemble a simple 68k source file during bring-up.
