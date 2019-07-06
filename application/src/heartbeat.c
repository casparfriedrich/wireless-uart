#include <gpio.h>
#include <logging/log.h>
#include <zephyr.h>
#include <zephyr/types.h>

#include "heartbeat.h"

LOG_MODULE_REGISTER(Heartbeat);

static struct k_timer heartbeat_timer;
static const char* led_controller;
static u32_t led_pin;

static void heartbeat_expiry_fn(struct k_timer* timer)
{
	static int phase = 0;

	struct device* controller = device_get_binding(led_controller);
	__ASSERT_NO_MSG(controller != NULL);
	if (!controller) {
		LOG_ERR("error retrieving device");
		return;
	}

	int err __unused = 0;

	switch (phase % 32) {
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

	phase++;
}

int heartbeat_start(const char* controller, u32_t pin)
{
	int err __unused = 0;

	led_controller = controller;
	led_pin = pin;

	struct device* device = device_get_binding(led_controller);
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

	k_timer_init(&heartbeat_timer, heartbeat_expiry_fn, NULL);
	k_timer_start(&heartbeat_timer, 0, K_MSEC(50));

	return 0;
}
