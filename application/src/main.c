#include <logging/log.h>
#include <nrf_clock.h>
#include <nrf_esb.h>
#include <zephyr.h>
#include <zephyr/types.h>

#include "heartbeat.h"
#include "serial.h"
#include "types.h"

extern const k_tid_t esb_thread;

LOG_MODULE_REGISTER(Main);

K_MSGQ_DEFINE(esb_frame_q, sizeof(struct nrf_esb_payload), 10, 4);
K_MSGQ_DEFINE(serial_frame_q, sizeof(struct nrf_esb_payload), 10, 4);

void hf_clock_init(void)
{
	nrf_clock_task_trigger(NRF_CLOCK_TASK_HFCLKSTART);

	while (nrf_clock_event_check(NRF_CLOCK_EVENT_HFCLKSTARTED) == false) {
		k_busy_wait(100);
	}
}

void main(void)
{
	LOG_INF("Starting Application: %p", k_current_get());

	hf_clock_init();
	heartbeat_init(LED0_GPIO_CONTROLLER, LED0_GPIO_PIN);

	k_thread_start(esb_thread);
}
