#pragma once

void die(const char *f, ...);
void crash(const char *f, ...);
void debug(const char *f, ...);
void warn(const char *f, ...);

// need to tell gcc through the macro expansion that this is unreachable
#define UNREACHABLE()                                                          \
  do {                                                                         \
    crash("%s:%s:%d -> unreachable statement", __FILE__, __func__, __LINE__);  \
    __builtin_unreachable();                                                   \
  } while (0)

#define TODO()                                                                 \
  do {                                                                         \
    crash("%s:%s:%d -> TODO", __FILE__, __func__, __LINE__);                   \
    __builtin_unreachable();                                                   \
  } while (0)

#define ANSI_RED "\e[31m"
#define ANSI_GREEN "\e[32m"
#define ANSI_YELLOW "\e[33m"
#define ANSI_BLUE "\e[34m"
#define ANSI_RESET "\e[0m"
