#include <zephyr/kernel.h>

#include "saludos.h"

#ifdef CONFIG_FEATURE_CONDITIONAL
#include "feat_conditional.h"
#endif

int main(void)
{
    saludo_base();

#ifdef CONFIG_FEATURE_CONDITIONAL
    feature_conditional();
#endif

    return 0;
}