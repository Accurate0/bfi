#pragma once

void die(const char *f, ...);
void crash(const char *f, ...);
void debug(const char *f, ...);
void warn(const char *f, ...);

#define UNREACHABLE() \
    do { \
        crash("%s:%s:%d -> unreachable statement", __FILE__, __func__, __LINE__); \
    } while(0)
