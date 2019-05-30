#include <device.h>
#include <logging/log.h>
#include <nrf_esb.h>
#include <stdio.h>
#include <string.h>
#include <uart.h>
#include <zephyr.h>
#include <zephyr/types.h>

#include "esb.h"
#include "types.h"

#define PRIORITY 3
#define STACKSIZE KB(2)
#define FRAME_LENGTH K_MSEC(20)

LOG_MODULE_REGISTER(ESB, LOG_LEVEL_DBG);

extern struct k_sem wireless_activity_alert;
extern struct k_mem_slab package_buffer_slab;
extern struct k_fifo serial2wireless_fifo;
extern struct k_fifo wireless2serial_fifo;

static void esb_thread_function(void* arg0, void* arg1, void* arg2);

K_THREAD_DEFINE(esb_thread,
                STACKSIZE,
                esb_thread_function,
                NULL,
                NULL,
                NULL,
                PRIORITY,
                0,
                K_FOREVER);

void my_timer_expiry_fn(struct k_timer* timer)
{
	LOG_DBG("timer");
}

K_TIMER_DEFINE(my_timer, my_timer_expiry_fn, NULL);

void esb_event_callback(struct nrf_esb_evt const* event)
{
	static int __unused err = 0;
	static struct nrf_esb_payload payload;

	k_sem_give(&wireless_activity_alert);
	k_timer_start(&my_timer, FRAME_LENGTH, 0);

	switch (event->evt_id) {
	case NRF_ESB_EVENT_TX_SUCCESS:
		err = nrf_esb_start_rx();
		__ASSERT(err == 0, "nrf_esb_start_rx: %d", err);

		break;

	case NRF_ESB_EVENT_TX_FAILED:
		err = nrf_esb_pop_tx();
		__ASSERT(err == 0, "nrf_esb_pop_tx: %d", err);
		err = nrf_esb_start_rx();
		__ASSERT(err == 0, "nrf_esb_start_rx: %d", err);

		break;

	case NRF_ESB_EVENT_RX_RECEIVED:
		err = nrf_esb_read_rx_payload(&payload);
		__ASSERT(err == 0, "nrf_esb_read_rx_payload: %d", err);
		if (err) {
			LOG_WRN("nrf_esb_read_rx_payload: %d", err);
			break;
		}

		struct message_t* message;
		int err = k_mem_slab_alloc(&package_buffer_slab, (void**)&message, K_NO_WAIT);
		__ASSERT(err == 0, "k_mem_slab_alloc: %d", err);
		if (err) {
			LOG_WRN("k_mem_slab_alloc: %d", err);
			break;
		}

		memcpy(message->data, payload.data, payload.length);
		message->length = payload.length;
		k_fifo_put(&wireless2serial_fifo, message);

		break;
	}
}

void esb_thread_function(void* arg0, void* arg1, void* arg2)
{
	static int __unused err = 0;
	static struct nrf_esb_payload payload = NRF_ESB_CREATE_PAYLOAD(0);

	err = nrf_esb_start_rx();
	__ASSERT(err == 0, "nrf_esb_start_rx: %d", err);

	while (1) {
		struct message_t* message = k_fifo_get(&serial2wireless_fifo, K_FOREVER);
		memcpy(payload.data, message->data, message->length);
		payload.length = message->length;
		k_mem_slab_free(&package_buffer_slab, (void**)&message);

		err = nrf_esb_stop_rx();
		__ASSERT(err == 0, "nrf_esb_stop_rx: %d", err);

		err = nrf_esb_write_payload(&payload);
		__ASSERT(err == 0, "nrf_esb_write_payload: %d", err);
	}
}

void esb_init(void)
{
	u8_t base_addr_0[4] = {0xE7, 0xE7, 0xE7, 0xE7};
	u8_t base_addr_1[4] = {0xC2, 0xC2, 0xC2, 0xC2};
	u8_t addr_prefix[8] = {0xE7, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8};

	struct nrf_esb_config config = NRF_ESB_DEFAULT_CONFIG;
	config.event_handler = esb_event_callback;

	static int __unused err = 0;

	err = nrf_esb_init(&config);
	__ASSERT(err == 0, "nrf_esb_init: %d", err);

	err = nrf_esb_set_base_address_0(base_addr_0);
	__ASSERT(err == 0, "nrf_esb_set_base_address_0: %d", err);

	err = nrf_esb_set_base_address_1(base_addr_1);
	__ASSERT(err == 0, "nrf_esb_set_base_address_1: %d", err);

	err = nrf_esb_set_prefixes(addr_prefix, ARRAY_SIZE(addr_prefix));
	__ASSERT(err == 0, "nrf_esb_set_prefixes: %d", err);

	k_thread_start(esb_thread);
}
