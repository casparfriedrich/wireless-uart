#include <logging/log.h>
#include <nrf_clock.h>
#include <nrf_esb.h>
#include <zephyr.h>
#include <zephyr/types.h>

#include "led.h"

LOG_MODULE_REGISTER(Main);

extern const k_tid_t esb_thread;
extern const k_tid_t serial_thread;

K_MSGQ_DEFINE(esb_frame_q, sizeof(struct nrf_esb_payload), 100, 4);
K_MSGQ_DEFINE(serial_frame_q, sizeof(struct nrf_esb_payload), 100, 4);

static void status_led_expiry_fn(struct k_timer *timer)
{
	led_flash(LED_0);
}

static K_TIMER_DEFINE(status_led_timer, status_led_expiry_fn, NULL);

void hf_clock_init(void)
{
	nrf_clock_task_trigger(NRF_CLOCK_TASK_HFCLKSTART);

	while (!nrf_clock_event_check(NRF_CLOCK_EVENT_HFCLKSTARTED)) {
		k_busy_wait(100);
	}
}

void main(void)
{
	LOG_INF("Starting Application: %p", k_current_get());

	k_timer_start(&status_led_timer, 0, K_SECONDS(1));

	hf_clock_init();
	led_init();

	k_thread_start(esb_thread);
	k_thread_start(serial_thread);
}
