#include "led.h"

#include <device.h>
#include <drivers/clock_control.h>
#include <esb.h>
#include <hal/nrf_clock.h>
#include <logging/log.h>
#include <usb/usb_device.h>
#include <zephyr.h>
#include <zephyr/types.h>

LOG_MODULE_REGISTER(Main);

extern const k_tid_t esb_thread;
extern const k_tid_t serial_thread;

K_MSGQ_DEFINE(esb_frame_q, sizeof(struct esb_payload), 100, 4);
K_MSGQ_DEFINE(serial_frame_q, sizeof(struct esb_payload), 100, 4);

void main(void)
{
	int err = 0;

	LOG_INF("Starting Application: %p", k_current_get());

	k_thread_start(esb_thread);
	k_thread_start(serial_thread);

	err = usb_enable(NULL);
	if (err != 0) {
		LOG_ERR("Failed to enable USB");
		return;
	}

	while (1) {
		led_flash(LED_GREEN);
		k_sleep(K_SECONDS(1));
	}
}
