void emit_trace(void *, const char *, const char *, unsigned long long);

void bad(void *context) {
  emit_trace(context, "core", "fixture.missing", 1u);
}
