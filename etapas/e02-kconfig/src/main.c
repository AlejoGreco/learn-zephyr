#include <zephyr/kernel.h>

#include "saludos.h"
#include "feat_conditional.h"

int main(void)
{
    saludo_base();

    if (IS_ENABLED(CONFIG_FEATURE_CONDITIONAL)) {
        feature_conditional();
        feature_conditional_config();
    }
    else {
        printk("Feature conditional is disabled!\n");
    }

    return 0;
}