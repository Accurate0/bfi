#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

__attribute__((noreturn)) void crash(const char *f, ...) {
  va_list args;
  va_start(args, f);

  fprintf(stderr, ANSI_RED "crash: " ANSI_RESET);
  vfprintf(stderr, f, args);
  fputc('\n', stderr);

  va_end(args);

  abort();
}

__attribute__((noreturn)) void die(const char *f, ...) {
  va_list args;
  va_start(args, f);

  fprintf(stderr, ANSI_RED "critical: " ANSI_RESET);
  vfprintf(stderr, f, args);
  fputc('\n', stderr);

  va_end(args);

  exit(EXIT_FAILURE);
}

void warn(const char *f, ...) {
  va_list args;
  va_start(args, f);

  fprintf(stderr, ANSI_YELLOW "warning: " ANSI_RESET);
  vfprintf(stderr, f, args);
  fputc('\n', stderr);
  va_end(args);
}

// #undef DEBUG
void debug(const char *f, ...) {
#ifdef DEBUG
  va_list args;
  va_start(args, f);

  fprintf(stderr, "debug: ");
  vfprintf(stderr, f, args);
  fputc('\n', stderr);

  va_end(args);
#else
  (void)f;
#endif
}
