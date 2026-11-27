#include "feat_conditional.h"

#include <stdarg.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>


#ifdef CONFIG_FEATURE_CONDITIONAL_USE_LOG
LOG_MODULE_REGISTER(feat_conditional, LOG_LEVEL_INF);

static void log_with_log(const char *fmt, ...)
{
    char buf[96];
    va_list args;

    va_start(args, fmt);
    vsnprintk(buf, sizeof(buf), fmt, args);
    va_end(args);

    LOG_INF("%s", buf);
}
static void (*feature_conditional_logger)(const char *fmt, ...) = log_with_log;
#else
static void log_with_printk(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vprintk(fmt, args);
    va_end(args);
}
static void (*feature_conditional_logger)(const char *fmt, ...) = log_with_printk;
#endif


void feature_conditional(void)
{
    if(IS_ENABLED(CONFIG_FEATURE_CONDITIONAL_USE_LOG)) {
        feature_conditional_logger("Feature conditional is enabled with LOG!\n");
    } else {
        feature_conditional_logger("Feature conditional is enabled with PRINTK!\n");
    }
}

void feature_conditional_config(void)
{
    feature_conditional_logger("Feature Baudrate: %d\n", CONFIG_FEATURE_CONDITIONAL_BAUDRATE);
    feature_conditional_logger("Feature Timeout: %d\n", CONFIG_FEATURE_CONDITIONAL_TIMEOUT);
}