#include <logging/log.h>
#include <nrf_clock.h>
#include <zephyr.h>
#include <zephyr/types.h>

#include "esb.h"
#include "serial.h"
#include "types.h"

LOG_MODULE_REGISTER(Main, LOG_LEVEL_DBG);

K_MEM_SLAB_DEFINE(package_buffer_slab, sizeof(struct message_t), 128, 4);

K_FIFO_DEFINE(serial2wireless_fifo);
K_FIFO_DEFINE(wireless2serial_fifo);

void hf_clock_init(void)
{
	nrf_clock_task_trigger(NRF_CLOCK_TASK_HFCLKSTART);

	while (nrf_clock_event_check(NRF_CLOCK_EVENT_HFCLKSTARTED) == false) {
		k_busy_wait(1000);
	}
}

void main(void)
{
	hf_clock_init();
	esb_init();
	serial_init();
}
