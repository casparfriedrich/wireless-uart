#include <logging/log.h>
#include <nrf_clock.h>
#include <zephyr.h>
#include <zephyr/types.h>

#include "heartbeat.h"
#include "serial.h"
#include "types.h"

extern const k_tid_t esb_thread;

LOG_MODULE_REGISTER(Main);

K_MEM_SLAB_DEFINE(package_buffer_slab, sizeof(struct message_t), 16, 4);

K_FIFO_DEFINE(serial2wireless_fifo);
K_FIFO_DEFINE(wireless2serial_fifo);

void hf_clock_init(void)
{
	nrf_clock_task_trigger(NRF_CLOCK_TASK_HFCLKSTART);

	while (nrf_clock_event_check(NRF_CLOCK_EVENT_HFCLKSTARTED) == false) {
		k_busy_wait(100);
	}
}

void main(void)
{
	hf_clock_init();

	heartbeat(LED0_GPIO_CONTROLLER, LED0_GPIO_PIN);

	k_thread_start(esb_thread);
	// serial_init();
}
