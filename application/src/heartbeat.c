#include <gpio.h>
#include <logging/log.h>
#include <zephyr.h>
#include <zephyr/types.h>

#include "heartbeat.h"

#define PRIORITY 0
#define STACKSIZE KB(1)

LOG_MODULE_REGISTER(Heartbeat);

static const char* g_led_controller;
static u32_t g_led_pin;

static int init(void)
{
	int err __unused = 0;

	struct device* controller = device_get_binding(g_led_controller);
	__ASSERT_NO_MSG(controller != NULL);
	if (!controller) {
		return -ENODEV;
	}

	gpio_pin_configure(controller, g_led_pin, GPIO_DIR_OUT);
	__ASSERT(err == 0, "gpio_pin_configure: %d", err);
	if (err) {
		return err;
	}

	gpio_pin_write(controller, g_led_pin, 1);
	__ASSERT(err == 0, "gpio_pin_write: %d", err);
	if (err) {
		return err;
	}

	return 0;
}

static void heartbeat_fn()
{
	static int phase = 0;

	struct device* controller = device_get_binding(g_led_controller);
	__ASSERT_NO_MSG(controller != NULL);
	if (!controller) {
		LOG_ERR("error retrieving device");
	}

	int err __unused = 0;

	switch (phase % 32) {
	case 0:
		err = gpio_pin_write(controller, g_led_pin, 0);
		if (err) {
			LOG_ERR("error writing gpio");
		}
		break;
	case 8:
		err = gpio_pin_write(controller, g_led_pin, 0);
		if (err) {
			LOG_ERR("error writing gpio");
		}
		break;
	default:
		err = gpio_pin_write(controller, g_led_pin, 1);
		if (err) {
			LOG_ERR("error writing gpio");
		}
		break;
	}

	phase++;
}

K_TIMER_DEFINE(heartbeat_delay, heartbeat_fn, NULL);

int heartbeat(const char* led_controller, u32_t led_pin)
{
	int err __unused = 0;

	g_led_controller = led_controller;
	g_led_pin = led_pin;

	err = init();
	if (err) {
		return err;
	}

	k_timer_start(&heartbeat_delay, 0, K_MSEC(50));

	return 0;
}
