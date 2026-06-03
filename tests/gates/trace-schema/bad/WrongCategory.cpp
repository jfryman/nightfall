void emit_trace(void *, const char *, const char *, unsigned long long);

void bad(void *context) {
  emit_trace(context, "app", "fixture.good", 1u);
}
