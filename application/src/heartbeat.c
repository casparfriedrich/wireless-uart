#include <gpio.h>
#include <logging/log.h>
#include <zephyr.h>
#include <zephyr/types.h>

#define PRIORITY 0
#define STACKSIZE KB(1)

LOG_MODULE_DECLARE(usb_adapter, LOG_LEVEL_DBG);

K_TIMER_DEFINE(heartbeat_delay, NULL, NULL);

void heartbeat_function(void* arg_0, void* arg_1, void* arg_)
{
	LOG_INF("%s (%p)", k_thread_name_get(k_current_get()), k_current_get());

	int err __unused = 0;
	int phase = 0;

	struct device* controller = device_get_binding(LED0_GPIO_CONTROLLER);
	__ASSERT_NO_MSG(controller != NULL);

	err = gpio_pin_configure(controller, LED0_GPIO_PIN, GPIO_DIR_OUT);
	__ASSERT(err == 0, "gpio_pin_configure (%d)", err);

	err = gpio_pin_write(controller, LED0_GPIO_PIN, 1);
	__ASSERT(err == 0, "gpio_pin_write (%d)", err);

	k_timer_start(&heartbeat_delay, 0, K_MSEC(350));

	while (1)
	{
		k_timer_status_sync(&heartbeat_delay);

		switch (phase % 5)
		{
			case 0:
			{
				gpio_pin_write(controller, LED0_GPIO_PIN, 0);
				k_sleep(K_MSEC(50));
				gpio_pin_write(controller, LED0_GPIO_PIN, 1);
				break;
			}
			case 1:
			{
				gpio_pin_write(controller, LED0_GPIO_PIN, 0);
				k_sleep(K_MSEC(50));
				gpio_pin_write(controller, LED0_GPIO_PIN, 1);
				break;
			}
		}

		phase++;
	}
}

K_THREAD_DEFINE(
	heartbeat_thread,
	STACKSIZE,
	heartbeat_function,
	NULL,
	NULL,
	NULL,
	PRIORITY,
	0,
	K_NO_WAIT);
