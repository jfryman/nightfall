void emit_trace(void *, const char *, const char *, unsigned long long);

void good(void *context) {
  emit_trace(context, "core", "fixture.good", 1u);
}
