#ifndef DEBUG_H
#define DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

int lowsyslog(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* DEBUG_H */