/* Compatibilidad con glibc 2.35 (Ubuntu 22.04): sustituye los símbolos
   versionados en glibc 2.38 por equivalentes antiguos. Se compila sin
   _GNU_SOURCE para que strtol & co. no se redirijan a __isoc23_*. */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>

#define HIDDEN __attribute__((visibility("hidden")))

HIDDEN long __isoc23_strtol(const char* s, char** e, int b) { return strtol(s, e, b); }
HIDDEN unsigned long __isoc23_strtoul(const char* s, char** e, int b) { return strtoul(s, e, b); }
HIDDEN unsigned long long __isoc23_strtoull(const char* s, char** e, int b) { return strtoull(s, e, b); }
HIDDEN long long __isoc23_strtoll(const char* s, char** e, int b) { return strtoll(s, e, b); }
HIDDEN int __isoc23_sscanf(const char* s, const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int r = vsscanf(s, fmt, ap);
  va_end(ap);
  return r;
}
HIDDEN double fmod(double x, double y) { return (double)fmodl((long double)x, (long double)y); }
HIDDEN float fmodf(float x, float y) { return (float)fmodl((long double)x, (long double)y); }

#include <sys/random.h>
HIDDEN unsigned int arc4random(void) {
  unsigned int v = 0;
  if (getrandom(&v, sizeof v, 0) != (long)sizeof v) v = (unsigned int)rand();
  return v;
}
