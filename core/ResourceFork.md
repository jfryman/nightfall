# ResourceFork

Parses classic Macintosh resource fork bytes into a deterministic in-memory
index keyed by four-character resource type and resource ID.

This unit is intentionally read-only. It validates the resource fork header,
resource data area, resource map, type list, reference lists, optional Pascal
resource names, and length-prefixed resource data before exposing entries to the
local real-module runner.

The parser does not call macOS Resource Manager APIs. It consumes raw resource
fork bytes supplied by a host-side tool so tests and trace capture remain
headless and deterministic.
