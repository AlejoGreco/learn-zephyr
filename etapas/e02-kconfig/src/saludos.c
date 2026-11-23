#include "saludos.h"

#include <zephyr/sys/printk.h>

void saludo_base(void)
{
    printk("Hola desde saludo_base()!\n");
}