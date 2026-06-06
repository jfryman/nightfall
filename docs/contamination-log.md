# Contamination Log

Append-only log for any suspected or confirmed forbidden-source contamination.

## Entries

## 2026-06-06 — After Dark Lifecycle ABI Search Result

- **Subsystem paused:** After Dark graphics-module lifecycle ABI and entry
  invocation semantics.
- **Incident:** While trying to determine why the local `ADgm/0` resource
  returned immediately under the new bounded runner, a web search for public
  module-format information surfaced pages under
  `https://smfr.org/computing/archaic/afterdark/adprog/` that include Berkeley
  Systems After Dark sample source/header material.
- **Agent exposure:** The agent saw snippets/page text describing the graphics
  module `main` interface and a Berkeley-copyrighted header page containing
  message definitions.
- **Work performed after exposure:** No After Dark lifecycle ABI or message
  dispatch implementation was written from that material. Work stopped on the
  affected subsystem and a blocker was opened.
- **Unaffected work:** The resource fork parser and bounded Musashi runner are
  derived from Inside Macintosh Resource Manager documentation and Musashi's
  pinned upstream documentation/source license, respectively.
