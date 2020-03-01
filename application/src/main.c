#include <device.h>
#include <drivers/clock_control.h>
#include <esb.h>
#include <hal/nrf_clock.h>
#include <logging/log.h>
#include <zephyr.h>
#include <zephyr/types.h>

#include "led.h"

LOG_MODULE_REGISTER(Main);

extern const k_tid_t esb_thread;
extern const k_tid_t serial_thread;

K_MSGQ_DEFINE(esb_frame_q, sizeof(struct esb_payload), 100, 4);
K_MSGQ_DEFINE(serial_frame_q, sizeof(struct esb_payload), 100, 4);

int clocks_start(void)
{
	int err = 0;

	struct device *hfclk;

	hfclk = device_get_binding(DT_INST_0_NORDIC_NRF_CLOCK_LABEL "_16M");
	if (!hfclk) {
		LOG_ERR("HF Clock device not found!");
		return -EIO;
	}

	err = clock_control_on(hfclk, NULL);
	if (err && (err != -EINPROGRESS)) {
		LOG_ERR("HF clock start fail: %d", err);
		return err;
	}

	LOG_DBG("HF clock started");
	return 0;
}

void main(void)
{
	int err = 0;

	LOG_INF("Starting Application: %p", k_current_get());

	err = clocks_start();
	__ASSERT_NO_MSG(!err);

	led_init();

	k_thread_start(esb_thread);
	k_thread_start(serial_thread);

	while (1) {
		led_flash(LED_0);
		k_sleep(K_SECONDS(1));
	}
}
