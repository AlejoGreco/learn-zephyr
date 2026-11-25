#include "feat_conditional.h"

#include <zephyr/sys/printk.h>

void feature_conditional(void)
{
    printk("Feature conditional is enabled!\n");
}

void feature_conditional_config(void)
{
    printk("Feature Baudrate: %d\n", CONFIG_FEATURE_CONDITIONAL_BAUDRATE);
    printk("Feature Timeout: %d\n", CONFIG_FEATURE_CONDITIONAL_TIMEOUT);
}