/*
 * Minimal blink + USB serial — validates toolchain, board definition,
 * GPIO, and USB CDC ACM console on SuperMini nRF52840.
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(hallmark, LOG_LEVEL_INF);

#define BLINK_INTERVAL_MS 500

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

int main(void)
{
	int ret;
	unsigned int count = 0;

	/* Wait for USB host to enumerate CDC ACM device before logging */
	k_msleep(1000);

	if (!gpio_is_ready_dt(&led)) {
		LOG_ERR("LED device not ready");
		return -ENODEV;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		LOG_ERR("Failed to configure LED: %d", ret);
		return ret;
	}

	LOG_INF("Hallmark firmware v0.0.1");
	LOG_INF("Board: SuperMini nRF52840");
	LOG_INF("LED blink started (%d ms interval)", BLINK_INTERVAL_MS);

	while (1) {
		gpio_pin_toggle_dt(&led);
		if (++count % 10 == 0) {
			LOG_INF("Heartbeat: %u toggles", count);
		}
		k_msleep(BLINK_INTERVAL_MS);
	}

	return 0;
}
