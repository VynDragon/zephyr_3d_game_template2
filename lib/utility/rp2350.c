#include <zephyr/kernel.h>
#include <hardware/vreg.h>
void board_early_init_hook (void)
{
	vreg_set_voltage(VREG_VOLTAGE_1_30);
}

int get_rp2350_v(void)
{
	return vreg_get_voltage();
}
