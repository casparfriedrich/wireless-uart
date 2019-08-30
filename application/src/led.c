#include <gpio.h>
#include <logging/log.h>
#include <zephyr.h>
#include <zephyr/types.h>

#define PRIORITY 0
#define STACKSIZE KB(1)

LOG_MODULE_REGISTER(Led, LOG_LEVEL_DBG);

K_SEM_DEFINE(led_ind_error, 0, 1);
K_SEM_DEFINE(led_ind_wired, 0, 1);
K_SEM_DEFINE(led_ind_wireless, 0, 1);

static const u32_t led_on = 0;
static const u32_t led_off = 1;

static const u32_t led1_gpio_pin = LED1_GPIO_PIN;
static const u32_t led2_gpio_pin = LED2_GPIO_PIN;
static const u32_t led3_gpio_pin = LED3_GPIO_PIN;
static const u32_t led4_gpio_pin = LED4_GPIO_PIN;

static inline void led_init(const char* device_name, u32_t* pin)
{
	int err __unused = 0;

	struct device* controller = device_get_binding(device_name);
	__ASSERT_NO_MSG(controller != NULL);

	err = gpio_pin_configure(controller, *pin, GPIO_DIR_OUT);
	__ASSERT(err == 0, "gpio_pin_configure (%d)", err);

	err = gpio_pin_write(controller, *pin, led_off);
	__ASSERT(err == 0, "gpio_pin_write (%d)", err);
}

static void led_hold(const char* device_name, u32_t* pin, struct k_sem* alert)
{
	led_init(device_name, pin);

	struct device* controller = device_get_binding(device_name);
	__ASSERT_NO_MSG(controller != NULL);

	while (1) {
		k_sem_take(alert, K_FOREVER);
		gpio_pin_write(controller, *pin, led_on);
	}
}

static void led_flash(const char* device_name, u32_t* pin, struct k_sem* alert)
{
	led_init(device_name, pin);

	struct device* controller = device_get_binding(device_name);
	__ASSERT_NO_MSG(controller != NULL);

	while (1) {
		k_sem_take(alert, K_FOREVER);
		gpio_pin_write(controller, *pin, led_on);
		k_sleep(K_MSEC(50));
		gpio_pin_write(controller, *pin, led_off);
	}
}

K_THREAD_DEFINE(led_error_thread,
                STACKSIZE,
                led_hold,
                LED4_GPIO_CONTROLLER,
                &led4_gpio_pin,
                &led_ind_error,
                PRIORITY,
                0,
                K_NO_WAIT);

K_THREAD_DEFINE(wired_activity_led_thread,
                STACKSIZE,
                led_flash,
                LED4_GPIO_CONTROLLER,
                &led4_gpio_pin,
                &led_ind_wired,
                PRIORITY,
                0,
                K_NO_WAIT);

K_THREAD_DEFINE(wireless_activity_led_thread,
                STACKSIZE,
                led_flash,
                LED3_GPIO_CONTROLLER,
                &led3_gpio_pin,
                &led_ind_wireless,
                PRIORITY,
                0,
                K_NO_WAIT);
