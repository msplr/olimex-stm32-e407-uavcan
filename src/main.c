#include <hal.h>
#include <stdarg.h>
// #include <chprintf.h>
#include "uavcan_node.h"

void panic_hook(const char *reason);

BaseSequentialStream *stdout;

void __assert_func(const char *_file, int _line, const char *_func, const char *_expr )
{
    (void)_file;
    (void)_line;
    (void)_func;
    (void)_expr;

    panic_hook("assertion failed");
    while(1);
}

void panic_hook(const char *reason)
{
    (void) reason;

    while (1);
    // NVIC_SystemReset();
}

int lowsyslog(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    chvprintf(stdout, fmt, ap);
    va_end(ap);

    return 0;
}

int main(void)
{
    halInit();
    chSysInit();

    sdStart(&SD3, NULL);
    stdout = (BaseSequentialStream *)&SD3;
    uavcan_node_start(NULL);

    while (1) {
        chThdSleepMilliseconds(1000);
    }
}
