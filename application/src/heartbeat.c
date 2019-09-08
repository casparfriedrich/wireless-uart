#include <gpio.h>
#include <logging/log.h>
#include <zephyr.h>
#include <zephyr/types.h>

#include "heartbeat.h"

LOG_MODULE_REGISTER(heartbeat);

static int err = 0;
static const char *led_controller = NULL;
static u32_t led_pin = 0;

static void heartbeat_expiry_fn(struct k_timer *timer)
{
	static int phase = 0;

	struct device *controller = device_get_binding(led_controller);
	if (!controller) {
		LOG_ERR("error binding device");
		return;
	}

	switch (phase++ % 32) {
	case 0:
		err = gpio_pin_write(controller, led_pin, 0);
		if (err) {
			LOG_ERR("error writing gpio: %d", err);
		}
		break;
	case 8:
		err = gpio_pin_write(controller, led_pin, 0);
		if (err) {
			LOG_ERR("error writing gpio: %d", err);
		}
		break;
	default:
		err = gpio_pin_write(controller, led_pin, 1);
		if (err) {
			LOG_ERR("error writing gpio: %d", err);
		}
		break;
	}

	phase %= 32;
}

K_TIMER_DEFINE(heartbeat_timer, heartbeat_expiry_fn, NULL);

int heartbeat_init(const char *controller, u32_t pin)
{
	led_controller = controller;
	led_pin = pin;

	struct device *device = device_get_binding(led_controller);
	if (!device) {
		LOG_ERR("error retrieving device: %d", err);
		return -ENODEV;
	}

	err = gpio_pin_configure(device, led_pin, GPIO_DIR_OUT);
	if (err) {
		LOG_ERR("error configuring gpio: %d", err);
		return err;
	}

	err = gpio_pin_write(device, led_pin, 1);
	if (err) {
		LOG_ERR("error writing gpio: %d", err);
		return err;
	}

	k_timer_start(&heartbeat_timer, 0, K_MSEC(40));

	return 0;
}
