#include <zephyr/kernel.h>
#include <hardware/regs/addressmap.h>
#include <hardware/regs/qmi.h>
#include <hardware/structs/qmi.h>
#include <hardware/regs/powman.h>
#include <hardware/structs/powman.h>

void board_early_init_hook(void)
{
	qmi_hw_t * const qmi = (qmi_hw_t * const)XIP_QMI_BASE;
	powman_hw_t * const powman = (powman_hw_t * const)POWMAN_BASE;

	/* Unlock VREG register */
	powman->vreg_ctrl |= POWMAN_PASSWORD_BITS
		| POWMAN_VREG_CTRL_DISABLE_VOLTAGE_LIMIT_BITS | POWMAN_VREG_CTRL_UNLOCK_BITS;

#if DT_PROP(DT_NODELABEL(clk_sys), clock_frequency) == 400000000
	/* Voltage of 1.35v */
	powman->vreg = (powman->vreg & ~POWMAN_VREG_VSEL_BITS)
		| POWMAN_PASSWORD_BITS | 16 << POWMAN_VREG_VSEL_LSB;
	/* Flash rx-delay = 3, divider = 4 */
	qmi->m[0].timing = 0x60007303;
#elif DT_PROP(DT_NODELABEL(clk_sys), clock_frequency) == 500000000
	/* Voltage of 1.6v */
	powman->vreg = (powman->vreg & ~POWMAN_VREG_VSEL_BITS)
		| POWMAN_PASSWORD_BITS | 19 << POWMAN_VREG_VSEL_LSB;
	/* Flash rx-delay = 3, divider = 5 */
	qmi->m[0].timing = 0x60007304;
#else
#error Please use one of the provided overclock or add your own here
#endif
}
